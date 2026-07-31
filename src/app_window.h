#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <memory>
#include <webview2.h>
#include <wrl.h>

using namespace Microsoft::WRL;

class AppWindow {
private:
    HWND hwnd;
    int width;
    int height;
    std::string title;
    std::string iconPath;
    
    ComPtr<ICoreWebView2> webView;
    ComPtr<ICoreWebView2Controller> webViewController;
    
    std::function<void()> onCloseCallback;
    std::function<void(const std::string&)> onWebMessageCallback;
    std::function<std::string(const std::string&)> webHandler;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void initializeWebView();

public:
    AppWindow(int w, int h);
    ~AppWindow();

    bool create();
    void show();
    void hide();
    void close();
    void setTitle(const std::string& title);
    void setIcon(const std::string& path);
    HWND getHandle() const { return hwnd; }
    
    void setWebHandler(std::function<std::string(const std::string&)> handler);
    void setOnClose(std::function<void()> callback);
    void setOnWebMessage(std::function<void(const std::string&)> callback);
    void sendToWeb(const std::string& message);
};