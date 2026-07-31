#pragma once
#include <windows.h>
#include <string>
#include <functional>

class TrayIcon {
private:
    HWND hwnd;
    NOTIFYICONDATAW nid;
    bool visible;
    std::string iconPath;
    
    std::function<void()> onOpenCallback;
    std::function<void()> onExitCallback;

    static LRESULT CALLBACK TrayWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void initNotifyIcon();

public:
    TrayIcon();
    ~TrayIcon();

    bool create();
    void destroy();
    void show();
    void hide();
    void updateIcon(const std::string& path);
    void setTooltip(const std::string& tooltip);
    void setIcon(const std::string& path);
    
    void onOpen(std::function<void()> callback);
    void onExit(std::function<void()> callback);

    bool isVisible() const { return visible; }
    HWND getHandle() const { return hwnd; }
};