#include "KeyboardWindow.h"
#include "InputSender.h"
#include "Settings.h"
#include <algorithm>

static const wchar_t* WINDOW_CLASS_NAME = L"SyndicateSoftKeyboardWindow";

KeyboardWindow::KeyboardWindow()
    : m_hWnd(nullptr),
      m_hInstance(nullptr),
      m_activeLayoutIndex(0),
      m_startMinimized(false),
      m_umlautHotkeysRegistered(false),
      m_shiftActive(false),
      m_trackingMouse(false),
      m_hoveredKeyIndex(-1),
      m_pressedKeyIndex(-1),
      m_hoveredHeaderBtn(0),
      m_pressedHeaderBtn(0),
      m_isDragging(false),
      m_hFontTitle(nullptr),
      m_hFontKey(nullptr),
      m_hFontHint(nullptr),
      m_hFontSmallBtn(nullptr),
      m_wmTaskbarCreated(0) {
    m_layouts = GetSupportedLayouts();
    m_startMinimized = Settings::LoadStartMinimized();
    ZeroMemory(&m_rcHeader, sizeof(m_rcHeader));
    ZeroMemory(&m_rcTitleBtn, sizeof(m_rcTitleBtn));
    ZeroMemory(&m_rcShiftBtn, sizeof(m_rcShiftBtn));
    ZeroMemory(&m_rcXBtn, sizeof(m_rcXBtn));

    ZeroMemory(&m_dragStartCursor, sizeof(m_dragStartCursor));
    ZeroMemory(&m_dragStartWindowRect, sizeof(m_dragStartWindowRect));
    m_wmTaskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
}


KeyboardWindow::~KeyboardWindow() {
    SetUmlautHotkeys(false);
    m_trayIcon.Remove();

    if (m_hFontTitle) DeleteObject(m_hFontTitle);
    if (m_hFontKey) DeleteObject(m_hFontKey);
    if (m_hFontHint) DeleteObject(m_hFontHint);
    if (m_hFontSmallBtn) DeleteObject(m_hFontSmallBtn);
}

bool KeyboardWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    m_hInstance = hInstance;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = KeyboardWindow::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // Handled in WM_PAINT
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExW(&wc);

    // Create fonts
    m_hFontTitle = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontKey = CreateFontW(26, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontHint = CreateFontW(12, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    m_hFontSmallBtn = CreateFontW(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // Position initially in lower right quadrant of the screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int initX = screenW - 320;
    int initY = screenH - 220;

    m_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        WINDOW_CLASS_NAME,
        L"Syndicate Soft Keyboard",
        WS_POPUP,
        initX, initY,
        260, 110,
        NULL,
        NULL,
        hInstance,
        this
    );

    if (!m_hWnd) {
        return false;
    }

    RecalculateLayoutGeometry();

    // Initialize System Tray Icon
    m_trayIcon.Initialize(m_hWnd, hInstance);

    HICON hIcon = m_trayIcon.GetIcon();
    if (hIcon) {
        SendMessageW(m_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
        SendMessageW(m_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
    }

    if (m_startMinimized) {
        ShowWindow(m_hWnd, SW_HIDE);
        SetUmlautHotkeys(false);
    } else {
        ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
        UpdateWindow(m_hWnd);
        SetUmlautHotkeys(true);
    }

    return true;
}

void KeyboardWindow::RecalculateLayoutGeometry() {
    if (m_activeLayoutIndex >= m_layouts.size()) {
        m_activeLayoutIndex = 0;
    }

    const auto& layout = m_layouts[m_activeLayoutIndex];
    size_t keyCount = layout.keys.size();

    const int headerH = 32;
    const int keyW = 54;
    const int keyH = 50;
    const int keyGap = 8;
    const int padX = 12;
    const int padY = 10;

    int cols = 4;
    int rows = 1;

    if (layout.preferredCols > 0) {
        cols = layout.preferredCols;
        rows = static_cast<int>((keyCount + cols - 1) / cols);
    } else if (keyCount <= 5) {
        cols = static_cast<int>(keyCount);
        rows = 1;
    } else {
        cols = 5;
        rows = static_cast<int>((keyCount + 4) / 5);
    }

    int maxRowKeys = cols;
    int maxRowW = maxRowKeys * keyW + (maxRowKeys - 1) * keyGap;
    int winW = padX * 2 + maxRowW;
    int winH = headerH + padY * 2 + rows * keyH + (rows - 1) * keyGap;


    // Minimum width for header controls (Title dropdown + Shift + X)
    if (winW < 240) {
        winW = 240;
    }

    // Measure Title text to size the dropdown pill button properly
    int titleW = 92;
    if (m_hFontTitle && m_hWnd) {
        HDC hdc = GetDC(m_hWnd);
        if (hdc) {
            HFONT hOld = (HFONT)SelectObject(hdc, m_hFontTitle);
            std::wstring titleText = layout.name + L"  \u25BE";
            RECT rcMeasure = { 0, 0, 0, 0 };
            DrawTextW(hdc, titleText.c_str(), -1, &rcMeasure, DT_CALCRECT | DT_SINGLELINE);
            SelectObject(hdc, hOld);
            ReleaseDC(m_hWnd, hdc);
            int measured = (rcMeasure.right - rcMeasure.left) + 20;
            if (measured > titleW) titleW = measured;
        }
    }

    m_rcHeader   = { 0, 0, winW, headerH };
    m_rcTitleBtn = { 8, 4, 8 + titleW, 28 };
    m_rcXBtn     = { winW - 32, 5, winW - 8, 27 };  // width 24, height 22
    m_rcShiftBtn = { winW - 92, 5, winW - 38, 27 }; // width 54, height 22

    // Calculate each key rectangle, centering keys in each row
    m_keyRects.clear();
    int startY = headerH + padY;

    for (int r = 0; r < rows; ++r) {
        int keysInThisRow = (r < rows - 1) ? cols : (static_cast<int>(keyCount) - r * cols);
        int rowW = keysInThisRow * keyW + (keysInThisRow - 1) * keyGap;
        int rowStartX = (winW - rowW) / 2;

        for (int c = 0; c < keysInThisRow; ++c) {
            int kx = rowStartX + c * (keyW + keyGap);
            int ky = startY + r * (keyH + keyGap);

            RECT rk = { kx, ky, kx + keyW, ky + keyH };
            m_keyRects.push_back(rk);
        }
    }

    if (m_hWnd) {
        KeepWindowOnScreen(winW, winH);

        // Apply smooth rounded window shape
        HRGN hRgn = CreateRoundRectRgn(0, 0, winW + 1, winH + 1, 10, 10);
        SetWindowRgn(m_hWnd, hRgn, TRUE);
        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

void KeyboardWindow::KeepWindowOnScreen(int winW, int winH) {
    if (!m_hWnd) {
        return;
    }

    RECT wndRc = {};
    GetWindowRect(m_hWnd, &wndRc);

    if (winW <= 0) {
        winW = wndRc.right - wndRc.left;
    }
    if (winH <= 0) {
        winH = wndRc.bottom - wndRc.top;
    }

    HMONITOR hMon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hMon, &mi)) {
        SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, winW, winH,
            SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        return;
    }

    const RECT& work = mi.rcWork;
    const int workW = work.right - work.left;
    const int workH = work.bottom - work.top;

    int x = wndRc.left;
    int y = wndRc.top;

    if (winW >= workW) {
        x = work.left;
    } else {
        if (x + winW > work.right) {
            x = work.right - winW;
        }
        if (x < work.left) {
            x = work.left;
        }
    }

    if (winH >= workH) {
        y = work.top;
    } else {
        if (y + winH > work.bottom) {
            y = work.bottom - winH;
        }
        if (y < work.top) {
            y = work.top;
        }
    }

    SetWindowPos(m_hWnd, HWND_TOPMOST, x, y, winW, winH, SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

#define IDHOT_UMLAUT_AE        4101
#define IDHOT_UMLAUT_OE        4102
#define IDHOT_UMLAUT_UE        4103
#define IDHOT_UMLAUT_SS        4104
#define IDHOT_UMLAUT_AE_SHIFT  4105
#define IDHOT_UMLAUT_OE_SHIFT  4106
#define IDHOT_UMLAUT_UE_SHIFT  4107
#define IDHOT_UMLAUT_SS_SHIFT  4108

struct UmlautHotkey {
    int id;
    UINT modifiers;
    UINT vk;
    wchar_t lower;
    wchar_t upper;
};

static const UmlautHotkey kUmlautHotkeys[] = {
    { IDHOT_UMLAUT_AE,       MOD_ALT,             'A', 0x00E4, 0x00C4 }, // ä, Ä
    { IDHOT_UMLAUT_OE,       MOD_ALT,             'O', 0x00F6, 0x00D6 }, // ö, Ö
    { IDHOT_UMLAUT_UE,       MOD_ALT,             'U', 0x00FC, 0x00DC }, // ü, Ü
    { IDHOT_UMLAUT_SS,       MOD_ALT,             'S', 0x00DF, 0x1E9E }, // ß, ẞ
    { IDHOT_UMLAUT_AE_SHIFT, MOD_ALT | MOD_SHIFT, 'A', 0x00E4, 0x00C4 },
    { IDHOT_UMLAUT_OE_SHIFT, MOD_ALT | MOD_SHIFT, 'O', 0x00F6, 0x00D6 },
    { IDHOT_UMLAUT_UE_SHIFT, MOD_ALT | MOD_SHIFT, 'U', 0x00FC, 0x00DC },
    { IDHOT_UMLAUT_SS_SHIFT, MOD_ALT | MOD_SHIFT, 'S', 0x00DF, 0x1E9E },
};

void KeyboardWindow::SetUmlautHotkeys(bool enable) {
    if (!m_hWnd || enable == m_umlautHotkeysRegistered) {
        return;
    }

    for (const auto& hk : kUmlautHotkeys) {
        if (enable) {
            RegisterHotKey(m_hWnd, hk.id, hk.modifiers | MOD_NOREPEAT, hk.vk);
        } else {
            UnregisterHotKey(m_hWnd, hk.id);
        }
    }

    m_umlautHotkeysRegistered = enable;
}

void KeyboardWindow::OnUmlautHotkey(int id) {
    if (!IsVisible()) {
        return;
    }

    const UmlautHotkey* match = nullptr;
    for (const auto& hk : kUmlautHotkeys) {
        if (hk.id == id) {
            match = &hk;
            break;
        }
    }
    if (!match) {
        return;
    }

    const bool wantUpper = m_shiftActive || (match->modifiers & MOD_SHIFT) ||
        ((GetKeyState(VK_SHIFT) & 0x8000) != 0);

    // Release Alt so the Unicode character is not treated as a menu mnemonic.
    INPUT altUp[2] = {};
    altUp[0].type = INPUT_KEYBOARD;
    altUp[0].ki.wVk = VK_MENU;
    altUp[0].ki.dwFlags = KEYEVENTF_KEYUP;
    altUp[1].type = INPUT_KEYBOARD;
    altUp[1].ki.wVk = VK_RMENU;
    altUp[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP;
    SendInput(2, altUp, sizeof(INPUT));

    InputSender::SendUnicodeCharacter(wantUpper ? match->upper : match->lower);
}

void KeyboardWindow::SetLayout(size_t layoutIndex) {
    if (layoutIndex < m_layouts.size()) {
        m_activeLayoutIndex = layoutIndex;
        m_hoveredKeyIndex = -1;
        m_pressedKeyIndex = -1;
        RecalculateLayoutGeometry();
    }
}

void KeyboardWindow::ToggleVisibility() {
    if (IsVisible()) {
        ShowWindow(m_hWnd, SW_HIDE);
    } else {
        KeepWindowOnScreen();
        ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
    }
}

bool KeyboardWindow::IsVisible() const {
    return (m_hWnd && IsWindowVisible(m_hWnd));
}

LRESULT CALLBACK KeyboardWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    KeyboardWindow* pThis = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<KeyboardWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<KeyboardWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT KeyboardWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_MOUSEACTIVATE:
        // Crucial: Prevent stealing focus from the active foreground document!
        return MA_NOACTIVATE;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_ERASEBKGND:
        return 1; // Prevent flicker; double-buffered in WM_PAINT

    case WM_LBUTTONDOWN:
        OnLButtonDown(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_LBUTTONUP:
        OnLButtonUp(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_RBUTTONUP:
        OnRButtonUp(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_MOUSELEAVE:
        OnMouseLeave();
        return 0;

    case WM_SETCURSOR: {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(m_hWnd, &pt);
        if (PtInRect(&m_rcTitleBtn, pt) || PtInRect(&m_rcShiftBtn, pt) || PtInRect(&m_rcXBtn, pt) || HitTestKey(pt.x, pt.y) >= 0) {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
        }
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return TRUE;
    }


    case WM_APP_TRAY:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            m_trayIcon.ShowContextMenu(m_hWnd, m_layouts, m_activeLayoutIndex, IsVisible(), m_startMinimized);
        } else if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            ToggleVisibility();
        }
        return 0;

    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        if (id == IDM_TRAY_SHOW) {
            KeepWindowOnScreen();
            ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
        } else if (id == IDM_TRAY_HIDE) {
            ShowWindow(m_hWnd, SW_HIDE);
        } else if (id == IDM_TRAY_START_MINIMIZED) {
            m_startMinimized = !m_startMinimized;
            Settings::SaveStartMinimized(m_startMinimized);
        } else if (id == IDM_TRAY_EXIT) {
            DestroyWindow(m_hWnd);
        } else if (id >= IDM_TRAY_LAYOUT_BASE && id < IDM_TRAY_LAYOUT_BASE + m_layouts.size()) {
            SetLayout(id - IDM_TRAY_LAYOUT_BASE);
        }
        return 0;
    }

    case WM_SHOWWINDOW:
        SetUmlautHotkeys(wParam != FALSE);
        return 0;

    case WM_HOTKEY:
        OnUmlautHotkey(static_cast<int>(wParam));
        return 0;

    case WM_DISPLAYCHANGE:
        KeepWindowOnScreen();
        return 0;

    case WM_DESTROY:
        SetUmlautHotkeys(false);
        m_trayIcon.Remove();
        PostQuitMessage(0);
        return 0;

    default:
        if (msg == m_wmTaskbarCreated) {
            m_trayIcon.ReAdd();
            return 0;
        }
        return DefWindowProcW(m_hWnd, msg, wParam, lParam);
    }
}

int KeyboardWindow::HitTestKey(int x, int y) {
    POINT pt = { x, y };
    for (size_t i = 0; i < m_keyRects.size(); ++i) {
        if (PtInRect(&m_keyRects[i], pt)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool KeyboardWindow::HitTestHeader(int x, int y) {
    POINT pt = { x, y };
    if (!PtInRect(&m_rcHeader, pt)) return false;
    if (PtInRect(&m_rcTitleBtn, pt)) return false;
    if (PtInRect(&m_rcShiftBtn, pt)) return false;
    if (PtInRect(&m_rcXBtn, pt)) return false;
    return true;
}

#define IDM_DROPDOWN_LAYOUT_BASE 3000

void KeyboardWindow::ShowLayoutDropdown() {
    POINT pt = { m_rcTitleBtn.left, m_rcHeader.bottom };
    ClientToScreen(m_hWnd, &pt);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    for (size_t i = 0; i < m_layouts.size(); ++i) {
        UINT flags = MF_STRING;
        if (i == m_activeLayoutIndex) {
            flags |= MF_CHECKED;
        }
        AppendMenuW(hMenu, flags, IDM_DROPDOWN_LAYOUT_BASE + i, m_layouts[i].name.c_str());
    }


    SetForegroundWindow(m_hWnd);
    int cmd = TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
    DestroyMenu(hMenu);

    m_pressedHeaderBtn = 0;
    m_hoveredHeaderBtn = 0;

    if (cmd >= IDM_DROPDOWN_LAYOUT_BASE && cmd < IDM_DROPDOWN_LAYOUT_BASE + static_cast<int>(m_layouts.size())) {
        SetLayout(cmd - IDM_DROPDOWN_LAYOUT_BASE);
    } else {
        InvalidateRect(m_hWnd, &m_rcTitleBtn, FALSE);
    }
}

void KeyboardWindow::OnLButtonDown(int x, int y) {
    POINT pt = { x, y };

    if (PtInRect(&m_rcTitleBtn, pt)) {
        m_pressedHeaderBtn = 4;
        InvalidateRect(m_hWnd, &m_rcTitleBtn, FALSE);
        ShowLayoutDropdown();
        return;
    }

    if (HitTestHeader(x, y)) {
        // Dragging soft keyboard anywhere across screen without losing target app focus
        m_isDragging = true;
        SetCapture(m_hWnd);
        GetCursorPos(&m_dragStartCursor);
        GetWindowRect(m_hWnd, &m_dragStartWindowRect);
        return;
    }

    if (PtInRect(&m_rcShiftBtn, pt)) {
        m_pressedHeaderBtn = 1;
        m_shiftActive = !m_shiftActive;
        InvalidateRect(m_hWnd, NULL, FALSE);
        return;
    }

    if (PtInRect(&m_rcXBtn, pt)) {
        m_pressedHeaderBtn = 2;
        InvalidateRect(m_hWnd, &m_rcXBtn, FALSE);
        return;
    }

    int keyIdx = HitTestKey(x, y);
    if (keyIdx >= 0) {
        m_pressedKeyIndex = keyIdx;
        InvalidateRect(m_hWnd, &m_keyRects[keyIdx], FALSE);

        // Check if physical shift is held or UI Shift is active
        bool isShift = m_shiftActive || ((GetKeyState(VK_SHIFT) & 0x8000) != 0);
        const auto& keyDef = m_layouts[m_activeLayoutIndex].keys[keyIdx];
        wchar_t charToSend = isShift ? keyDef.shiftCharToSend : keyDef.charToSend;

        InputSender::SendUnicodeCharacter(charToSend);
    }
}

void KeyboardWindow::OnLButtonUp(int x, int y) {
    POINT pt = { x, y };

    if (m_isDragging) {
        m_isDragging = false;
        ReleaseCapture();
        KeepWindowOnScreen();
    }

    if (m_pressedHeaderBtn == 2 && PtInRect(&m_rcXBtn, pt)) {
        // X button minimizes to system tray
        ShowWindow(m_hWnd, SW_HIDE);
        m_trayIcon.ShowBalloon(L"Syndicate Keyboard", L"Keyboard minimized to tray. Click the icon to restore, or right-click to exit.");
    }
    m_pressedHeaderBtn = 0;

    if (m_pressedKeyIndex >= 0) {
        int oldPressed = m_pressedKeyIndex;
        m_pressedKeyIndex = -1;
        InvalidateRect(m_hWnd, &m_keyRects[oldPressed], FALSE);
    }
}

void KeyboardWindow::OnRButtonUp(int x, int y) {
    // Right clicking directly inputs the uppercase character
    int keyIdx = HitTestKey(x, y);
    if (keyIdx >= 0) {
        const auto& keyDef = m_layouts[m_activeLayoutIndex].keys[keyIdx];
        InputSender::SendUnicodeCharacter(keyDef.shiftCharToSend);
    }
}

void KeyboardWindow::OnMouseMove(int x, int y) {
    if (!m_trackingMouse) {
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        TrackMouseEvent(&tme);
        m_trackingMouse = true;
    }

    if (m_isDragging) {
        POINT cur;
        GetCursorPos(&cur);
        int dx = cur.x - m_dragStartCursor.x;
        int dy = cur.y - m_dragStartCursor.y;

        SetWindowPos(m_hWnd, HWND_TOPMOST,
            m_dragStartWindowRect.left + dx,
            m_dragStartWindowRect.top + dy,
            0, 0,
            SWP_NOSIZE | SWP_NOACTIVATE);
        return;
    }

    POINT pt = { x, y };
    int newHoveredKey = HitTestKey(x, y);
    int newHoveredHeaderBtn = 0;

    if (PtInRect(&m_rcShiftBtn, pt)) newHoveredHeaderBtn = 1;
    else if (PtInRect(&m_rcXBtn, pt)) newHoveredHeaderBtn = 2;
    else if (PtInRect(&m_rcTitleBtn, pt)) newHoveredHeaderBtn = 4;

    if (newHoveredKey != m_hoveredKeyIndex || newHoveredHeaderBtn != m_hoveredHeaderBtn) {
        m_hoveredKeyIndex = newHoveredKey;
        m_hoveredHeaderBtn = newHoveredHeaderBtn;
        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

void KeyboardWindow::OnMouseLeave() {
    m_trackingMouse = false;
    if (m_hoveredKeyIndex != -1 || m_hoveredHeaderBtn != 0) {
        m_hoveredKeyIndex = -1;
        m_hoveredHeaderBtn = 0;
        InvalidateRect(m_hWnd, NULL, FALSE);
    }
}

static void DrawRoundedBox(HDC hDC, const RECT& rc, COLORREF bg, COLORREF border, int radius = 6) {
    HPEN hPen = CreatePen(PS_SOLID, 1, border);
    HBRUSH hBrush = CreateSolidBrush(bg);
    HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);

    RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, radius, radius);

    SelectObject(hDC, hOldBrush);
    DeleteObject(hBrush);
    SelectObject(hDC, hOldPen);
    DeleteObject(hPen);
}

void KeyboardWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);

    RECT clientRc;
    GetClientRect(m_hWnd, &clientRc);
    int w = clientRc.right - clientRc.left;
    int h = clientRc.bottom - clientRc.top;

    // Double-buffered GDI drawing
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    SetBkMode(memDC, TRANSPARENT);

    // Main window background
    HBRUSH hBgBrush = CreateSolidBrush(RGB(30, 31, 34)); // Discord/VS Code dark background
    FillRect(memDC, &clientRc, hBgBrush);
    DeleteObject(hBgBrush);

    // Header background
    HBRUSH hHeaderBrush = CreateSolidBrush(RGB(43, 45, 49));
    FillRect(memDC, &m_rcHeader, hHeaderBrush);
    DeleteObject(hHeaderBrush);

    // Subtle line below header
    HPEN hDividerPen = CreatePen(PS_SOLID, 1, RGB(58, 61, 67));
    HPEN oldPen = (HPEN)SelectObject(memDC, hDividerPen);
    MoveToEx(memDC, 0, m_rcHeader.bottom - 1, NULL);
    LineTo(memDC, w, m_rcHeader.bottom - 1);
    SelectObject(memDC, oldPen);
    DeleteObject(hDividerPen);

    // 1. Draw Title Dropdown Button
    bool isTitleHovered = (m_hoveredHeaderBtn == 4);
    bool isTitlePressed = (m_pressedHeaderBtn == 4);
    COLORREF titleBg = isTitlePressed ? RGB(88, 101, 242) : (isTitleHovered ? RGB(55, 58, 64) : RGB(38, 40, 44));
    COLORREF titleBorder = isTitleHovered ? RGB(88, 101, 242) : RGB(60, 63, 70);
    COLORREF titleTextCol = (isTitlePressed || isTitleHovered) ? RGB(255, 255, 255) : RGB(225, 228, 234);
    DrawRoundedBox(memDC, m_rcTitleBtn, titleBg, titleBorder, 4);

    SelectObject(memDC, m_hFontTitle);
    SetTextColor(memDC, titleTextCol);
    const auto& activeLayout = m_layouts[m_activeLayoutIndex];
    std::wstring titleText = activeLayout.name + L"  \u25BE";
    DrawTextW(memDC, titleText.c_str(), -1, &m_rcTitleBtn, DT_CENTER | DT_VCENTER | DT_SINGLELINE);


    // 2. Draw Shift Button
    COLORREF shiftBg = m_shiftActive ? RGB(88, 101, 242) : (m_hoveredHeaderBtn == 1 ? RGB(60, 63, 69) : RGB(49, 51, 56));
    COLORREF shiftBorder = m_shiftActive ? RGB(114, 137, 218) : RGB(70, 73, 80);
    COLORREF shiftText = m_shiftActive ? RGB(255, 255, 255) : RGB(180, 185, 195);
    DrawRoundedBox(memDC, m_rcShiftBtn, shiftBg, shiftBorder, 4);

    SelectObject(memDC, m_hFontSmallBtn);
    SetTextColor(memDC, shiftText);
    RECT shiftTextRc = m_rcShiftBtn;
    DrawTextW(memDC, L"SHIFT", -1, &shiftTextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 3. Draw Minimize-to-Tray 'X' Button
    COLORREF xBg = (m_hoveredHeaderBtn == 2) ? RGB(218, 55, 60) : RGB(49, 51, 56);
    COLORREF xBorder = (m_hoveredHeaderBtn == 2) ? RGB(237, 66, 69) : RGB(70, 73, 80);
    COLORREF xText = (m_hoveredHeaderBtn == 2) ? RGB(255, 255, 255) : RGB(200, 204, 210);
    DrawRoundedBox(memDC, m_rcXBtn, xBg, xBorder, 4);

    SelectObject(memDC, m_hFontSmallBtn);
    SetTextColor(memDC, xText);
    RECT xTextRc = m_rcXBtn;
    DrawTextW(memDC, L"X", -1, &xTextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);


    // 5. Draw Keys
    bool isShift = m_shiftActive || ((GetKeyState(VK_SHIFT) & 0x8000) != 0);

    for (size_t i = 0; i < m_keyRects.size() && i < activeLayout.keys.size(); ++i) {
        const auto& keyDef = activeLayout.keys[i];
        const RECT& rk = m_keyRects[i];

        bool isPressed = (static_cast<int>(i) == m_pressedKeyIndex);
        bool isHovered = (static_cast<int>(i) == m_hoveredKeyIndex);

        COLORREF keyBg;
        COLORREF keyBorder;

        if (isPressed) {
            keyBg = RGB(88, 101, 242);
            keyBorder = RGB(114, 137, 218);
        } else if (isHovered) {
            keyBg = RGB(64, 66, 73);
            keyBorder = RGB(88, 101, 242);
        } else {
            keyBg = RGB(49, 51, 56);
            keyBorder = RGB(63, 65, 71);
        }

        DrawRoundedBox(memDC, rk, keyBg, keyBorder, 6);

        // Draw Corner Hint for Alternate Case (when Shift is OFF, show small upper-case hint)
        if (!isShift) {
            SelectObject(memDC, m_hFontHint);
            SetTextColor(memDC, isPressed ? RGB(220, 225, 255) : RGB(140, 145, 155));
            RECT hintRc = { rk.left + 6, rk.top + 3, rk.right - 6, rk.top + 16 };
            DrawTextW(memDC, keyDef.shiftLabel.c_str(), -1, &hintRc, DT_RIGHT | DT_TOP | DT_SINGLELINE);
        }

        // Draw Main Character
        SelectObject(memDC, m_hFontKey);
        SetTextColor(memDC, RGB(255, 255, 255));
        std::wstring displayChar = isShift ? keyDef.shiftLabel : keyDef.label;

        RECT textRc = rk;
        DrawTextW(memDC, displayChar.c_str(), -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Outer border for the keyboard window
    HPEN hWinBorderPen = CreatePen(PS_SOLID, 1, RGB(70, 73, 80));
    HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    SelectObject(memDC, hWinBorderPen);
    SelectObject(memDC, hNullBrush);
    RoundRect(memDC, 0, 0, w, h, 10, 10);
    DeleteObject(hWinBorderPen);

    // Blit from memory DC to screen
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(m_hWnd, &ps);
}
