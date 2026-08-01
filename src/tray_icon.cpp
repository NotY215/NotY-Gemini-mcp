#include "tray_icon.h"
#include <shellapi.h>
#include <strsafe.h>
#include <iostream>

TrayIcon::TrayIcon() : hwnd(nullptr), visible(false), iconPath("") {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
    nid.cbSize = sizeof(NOTIFYICONDATAW);
}

TrayIcon::~TrayIcon() {
    destroy();
}

bool TrayIcon::create() {
    // Create a hidden window to handle tray messages
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = TrayWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"TrayIconWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    hwnd = CreateWindowEx(
        0,
        L"TrayIconWindow",
        L"TrayIcon",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 300,
        NULL, NULL,
        GetModuleHandle(NULL),
        this
    );

    if (!hwnd) {
        return false;
    }

    // Initialize the tray icon
    initNotifyIcon();
    visible = true;

    return true;
}

void TrayIcon::destroy() {
    if (visible) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        visible = false;
    }
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
}

void TrayIcon::initNotifyIcon() {
    nid.hWnd = hwnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;

    // Load default icon if no custom icon specified
    if (iconPath.empty()) {
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    else {
        std::wstring wPath(iconPath.begin(), iconPath.end());
        nid.hIcon = (HICON)LoadImageW(
            GetModuleHandle(NULL),
            wPath.c_str(),
            IMAGE_ICON,
            16, 16,
            LR_LOADFROMFILE
        );
        if (!nid.hIcon) {
            nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        }
    }

    StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), L"NotY-Gemini-MCP");

    Shell_NotifyIconW(NIM_ADD, &nid);
}

void TrayIcon::show() {
    if (!visible && hwnd) {
        initNotifyIcon();
        visible = true;
    }
}

void TrayIcon::hide() {
    if (visible) {
        Shell_NotifyIconW(NIM_DELETE, &nid);
        visible = false;
    }
}

void TrayIcon::updateIcon(const std::string& path) {
    iconPath = path;
    if (visible) {
        std::wstring wPath(path.begin(), path.end());
        nid.hIcon = (HICON)LoadImageW(
            GetModuleHandle(NULL),
            wPath.c_str(),
            IMAGE_ICON,
            16, 16,
            LR_LOADFROMFILE
        );
        if (nid.hIcon) {
            Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
    }
}

void TrayIcon::setIcon(const std::string& path) {
    iconPath = path;
}

void TrayIcon::setTooltip(const std::string& tooltip) {
    std::wstring wTooltip(tooltip.begin(), tooltip.end());
    StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), wTooltip.c_str());
    if (visible) {
        Shell_NotifyIconW(NIM_MODIFY, &nid);
    }
}

void TrayIcon::onOpen(std::function<void()> callback) {
    onOpenCallback = callback;
}

void TrayIcon::onExit(std::function<void()> callback) {
    onExitCallback = callback;
}

LRESULT CALLBACK TrayIcon::TrayWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TrayIcon* tray = reinterpret_cast<TrayIcon*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        tray = reinterpret_cast<TrayIcon*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(tray));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    if (tray) {
        switch (msg) {
        case WM_USER + 1: // Tray icon message
            switch (lParam) {
            case WM_LBUTTONDBLCLK:
            case WM_LBUTTONUP:
                if (tray->onOpenCallback) {
                    tray->onOpenCallback();
                }
                break;

            case WM_RBUTTONUP: {
                // Show context menu
                POINT pt;
                GetCursorPos(&pt);

                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, 1, L"Open NotY-Gemini-MCP");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, MF_STRING, 2, L"Exit");

                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);

                if (cmd == 1) {
                    if (tray->onOpenCallback) {
                        tray->onOpenCallback();
                    }
                }
                else if (cmd == 2) {
                    if (tray->onExitCallback) {
                        tray->onExitCallback();
                    }
                }

                DestroyMenu(hMenu);
                break;
            }
            }
            break;

        case WM_DESTROY:
            if (tray->isVisible()) {
                tray->destroy();
            }
            break;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}