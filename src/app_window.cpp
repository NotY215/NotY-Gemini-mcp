#include "app_window.h"
#include <windows.h>
#include <commctrl.h>
#include <exdisp.h>
#include <exdispid.h>
#include <mshtml.h>
#include <string>
#include <sstream>
#include <shellapi.h>

// Define GUID for IWebBrowser2 if not defined
#ifndef IID_IWebBrowser2
const IID IID_IWebBrowser2 = { 0xD30C1661, 0xCDAF, 0x11D0, {0x8A, 0x3E, 0x00, 0xC0, 0x4F, 0xC9, 0xE2, 0x6E} };
#endif

AppWindow::AppWindow(int w, int h)
    : hwnd(nullptr), width(w), height(h), title("NotY-Gemini-MCP"),
    isVisible(false), browserHwnd(nullptr) {
    CoInitialize(NULL);
}

AppWindow::~AppWindow() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    CoUninitialize();
}

bool AppWindow::create() {
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"NotYGeminiMCPWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        L"NotYGeminiMCPWindow",
        std::wstring(title.begin(), title.end()).c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
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

    createBrowserControl();
    isVisible = true;

    return true;
}

void AppWindow::createBrowserControl() {
    RECT rect;
    GetClientRect(hwnd, &rect);

    // Create WebBrowser control using the old-style ActiveX control
    browserHwnd = CreateWindow(
        L"AtlAxWin140",
        L"Shell.Explorer.2",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        rect.left, rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (!browserHwnd) {
        // Fallback: Try using the older WebBrowser control
        browserHwnd = CreateWindow(
            L"WebBrowser",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            rect.left, rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL
        );
    }

    if (browserHwnd) {
        navigateTo("http://localhost:31415/");
    }
}

void AppWindow::navigateTo(const std::string& url) {
    if (!browserHwnd) return;

    // Send navigation message to the WebBrowser control
    std::wstring wurl(url.begin(), url.end());

    // Try to get IWebBrowser2 interface
    IWebBrowser2* pBrowser = nullptr;
    if (SUCCEEDED(::SendMessage(browserHwnd, OCM__BASE + 0x2000, 0, (LPARAM)&pBrowser))) {
        if (pBrowser) {
            VARIANT vURL;
            VariantInit(&vURL);
            vURL.vt = VT_BSTR;
            vURL.bstrVal = SysAllocString(wurl.c_str());

            VARIANT vFlags, vTargetFrame, vPostData, vHeaders;
            VariantInit(&vFlags);
            VariantInit(&vTargetFrame);
            VariantInit(&vPostData);
            VariantInit(&vHeaders);

            pBrowser->Navigate2(&vURL, &vFlags, &vTargetFrame, &vPostData, &vHeaders);

            VariantClear(&vURL);
            pBrowser->Release();
        }
    }
}

void AppWindow::show() {
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
        isVisible = true;
    }
}

void AppWindow::hide() {
    if (hwnd) {
        ShowWindow(hwnd, SW_HIDE);
        isVisible = false;
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

void AppWindow::setOnClose(std::function<void()> callback) {
    onCloseCallback = callback;
}

void AppWindow::setOnWebMessage(std::function<void(const std::string&)> callback) {
    onWebMessageCallback = callback;
}

void AppWindow::sendToWeb(const std::string& message) {
    // Inject JavaScript into the page
    if (browserHwnd) {
        std::string script =
            "if (window.receiveMessage) { window.receiveMessage('" +
            message + "'); }";
        // Try to execute script via IHTMLDocument2
        // This is simplified - full implementation would need COM
    }
}

LRESULT CALLBACK AppWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppWindow* app = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = reinterpret_cast<AppWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    else {
        app = reinterpret_cast<AppWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (app) {
        switch (msg) {
        case WM_SIZE:
            if (app->browserHwnd) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                SetWindowPos(app->browserHwnd, NULL,
                    rect.left, rect.top,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOZORDER);
            }
            break;

        case WM_CLOSE:
            if (app->onCloseCallback) {
                app->onCloseCallback();
            }
            else {
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