#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include <memory>

class AppWindow {
private:
    HWND hwnd;
    int width;
    int height;
    std::string title;
    std::string iconPath;
    bool isVisible;
    HWND browserHwnd;

    std::function<void()> onCloseCallback;
    std::function<void(const std::string&)> onWebMessageCallback;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void createBrowserControl();
    void injectScript(const std::string& script);

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

    void setOnClose(std::function<void()> callback);
    void setOnWebMessage(std::function<void(const std::string&)> callback);
    void sendToWeb(const std::string& message);
    void navigateTo(const std::string& url);
};