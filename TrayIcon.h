// (c) 2026 MiraNova Studio
#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include "Layout.h"

#define WM_APP_TRAY (WM_APP + 101)

#define IDM_TRAY_SHOW             1001
#define IDM_TRAY_HIDE             1002
#define IDM_TRAY_EXIT             1003
#define IDM_TRAY_START_MINIMIZED  1004
#define IDM_TRAY_LAYOUT_BASE      2000

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();

    bool Initialize(HWND hWnd, HINSTANCE hInstance);
    bool ReAdd();
    void Remove();
    void ShowContextMenu(HWND hWnd, const std::vector<LanguageLayout>& layouts, size_t activeLayoutIndex, bool isWindowVisible, bool startMinimized);
    void ShowBalloon(const std::wstring& title, const std::wstring& text);
    HICON GetIcon() const { return m_hIcon; }
    bool IsAdded() const { return m_added; }

private:
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    NOTIFYICONDATAW m_nid;
    HICON m_hIcon;
    bool m_added;

    HICON CreateProceduralIcon(HINSTANCE hInstance);
};

