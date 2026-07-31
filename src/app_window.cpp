#include "app_window.h"
#include <thread>
#include <chrono>
#include <iostream>

AppWindow::AppWindow(int w, int h) 
    : hwnd(nullptr), width(w), height(h), title("NotY-Gemini-MCP") {
}

AppWindow::~AppWindow() {
    if (webView) {
        webView->Close();
    }
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

bool AppWindow::create() {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"NotYGeminiMCPWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassEx(&wc)) {
        return false;
    }

    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        0,
        L"NotYGeminiMCPWindow",
        std::wstring(title.begin(), title.end()).c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL, NULL,
        GetModuleHandle(NULL),
        this
    );

    if (!hwnd) {
        return false;
    }

    // Initialize WebView2
    initializeWebView();

    // Load HTML
    webView->Navigate(L"http://localhost:31415/");

    return true;
}

void AppWindow::initializeWebView() {
    auto webViewEnvironmentOptions = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, webViewEnvironmentOptions.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                env->CreateCoreWebView2Controller(
                    hwnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            webViewController = controller;
                            webViewController->get_CoreWebView2(&webView);
                            
                            // Set up web message handler
                            webView->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                        LPWSTR message = nullptr;
                                        args->TryGetWebMessageAsString(&message);
                                        if (message && onWebMessageCallback) {
                                            std::wstring wstr(message);
                                            std::string str(wstr.begin(), wstr.end());
                                            onWebMessageCallback(str);
                                        }
                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );

                            // Set up navigation handler
                            webView->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                        // Enable script
                                        webView->AddScriptToExecuteOnDocumentCreated(
                                            L"window.electronAPI = { postMessage: (msg) => window.chrome.webview.postMessage(msg) };",
                                            nullptr
                                        );
                                        return S_OK;
                                    }
                                ).Get(),
                                nullptr
                            );

                            // Resize web view to fill window
                            RECT bounds;
                            GetClientRect(hwnd, &bounds);
                            webViewController->put_Bounds(bounds);
                            
                            return S_OK;
                        }
                    ).Get()
                );
                return S_OK;
            }
        ).Get()
    );
}

void AppWindow::show() {
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
    }
}

void AppWindow::hide() {
    if (hwnd) {
        ShowWindow(hwnd, SW_HIDE);
    }
}

void AppWindow::close() {
    if (hwnd) {
        SendMessage(hwnd, WM_CLOSE, 0, 0);
    }
}

void AppWindow::setTitle(const std::string& title) {
    this->title = title;
    if (hwnd) {
        std::wstring wtitle(title.begin(), title.end());
        SetWindowTextW(hwnd, wtitle.c_str());
    }
}

void AppWindow::setIcon(const std::string& path) {
    this->iconPath = path;
    if (hwnd && !path.empty()) {
        HICON hIcon = (HICON)LoadImageA(
            GetModuleHandle(NULL),
            path.c_str(),
            IMAGE_ICON,
            32, 32,
            LR_LOADFROMFILE
        );
        if (hIcon) {
            SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
    }
}

void AppWindow::setWebHandler(std::function<std::string(const std::string&)> handler) {
    webHandler = handler;
}

void AppWindow::setOnClose(std::function<void()> callback) {
    onCloseCallback = callback;
}

void AppWindow::setOnWebMessage(std::function<void(const std::string&)> callback) {
    onWebMessageCallback = callback;
}

void AppWindow::sendToWeb(const std::string& message) {
    if (webView) {
        std::wstring wmessage(message.begin(), message.end());
        webView->PostWebMessageAsString(wmessage.c_str());
    }
}

LRESULT CALLBACK AppWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppWindow* app = nullptr;
    
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<AppWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<AppWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (app) {
        switch (msg) {
            case WM_SIZE:
                if (app->webViewController) {
                    RECT bounds;
                    GetClientRect(hwnd, &bounds);
                    app->webViewController->put_Bounds(bounds);
                }
                break;
                
            case WM_CLOSE:
                if (app->onCloseCallback) {
                    app->onCloseCallback();
                } else {
                    DestroyWindow(hwnd);
                }
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}