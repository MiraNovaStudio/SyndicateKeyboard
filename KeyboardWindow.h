// (c) 2026 MiraNova Studio
#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "Layout.h"
#include "TrayIcon.h"

class KeyboardWindow {
public:
    KeyboardWindow();
    ~KeyboardWindow();

    bool Create(HINSTANCE hInstance, int nCmdShow);
    HWND GetHwnd() const { return m_hWnd; }

    void SetLayout(size_t layoutIndex);
    size_t GetActiveLayoutIndex() const { return m_activeLayoutIndex; }
    const std::vector<LanguageLayout>& GetLayouts() const { return m_layouts; }

    void ToggleVisibility();
    bool IsVisible() const;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnPaint();
    void OnLButtonDown(int x, int y);
    void OnLButtonUp(int x, int y);
    void OnRButtonUp(int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseLeave();

    void RecalculateLayoutGeometry();
    void KeepWindowOnScreen(int winW = 0, int winH = 0);
    void ShowLayoutDropdown();
    void ShowAbout();
    void SetUmlautHotkeys(bool enable);
    void OnUmlautHotkey(int id);
    int HitTestKey(int x, int y);
    bool HitTestHeader(int x, int y);

    HWND m_hWnd;
    HINSTANCE m_hInstance;
    TrayIcon m_trayIcon;

    std::vector<LanguageLayout> m_layouts;
    size_t m_activeLayoutIndex;

    bool m_startMinimized;
    bool m_umlautHotkeysRegistered;
    bool m_shiftActive;
    bool m_trackingMouse;
    int m_hoveredKeyIndex;
    int m_pressedKeyIndex;

    // Header buttons hit test & states
    RECT m_rcHeader;
    RECT m_rcTitleBtn;
    RECT m_rcShiftBtn;
    RECT m_rcXBtn;

    int m_hoveredHeaderBtn; // 0 = none, 1 = Shift, 2 = X, 4 = Title Dropdown
    int m_pressedHeaderBtn;


    UINT m_wmTaskbarCreated;

    // Key rectangles in window coordinates
    std::vector<RECT> m_keyRects;

    // Dragging state
    bool m_isDragging;
    POINT m_dragStartCursor;
    RECT m_dragStartWindowRect;

    // GDI Fonts
    HFONT m_hFontTitle;
    HFONT m_hFontKey;
    HFONT m_hFontHint;
    HFONT m_hFontSmallBtn;
};
