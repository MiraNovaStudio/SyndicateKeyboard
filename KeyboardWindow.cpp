#include "KeyboardWindow.h"
#include "InputSender.h"
#include <algorithm>

static const wchar_t* WINDOW_CLASS_NAME = L"SyndicateSoftKeyboardWindow";

KeyboardWindow::KeyboardWindow()
    : m_hWnd(nullptr),
      m_hInstance(nullptr),
      m_activeLayoutIndex(0),
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
      m_hFontSmallBtn(nullptr) {
    m_layouts = GetSupportedLayouts();
    ZeroMemory(&m_rcHeader, sizeof(m_rcHeader));
    ZeroMemory(&m_rcShiftBtn, sizeof(m_rcShiftBtn));
    ZeroMemory(&m_rcMinBtn, sizeof(m_rcMinBtn));
    ZeroMemory(&m_rcCloseBtn, sizeof(m_rcCloseBtn));
    ZeroMemory(&m_dragStartCursor, sizeof(m_dragStartCursor));
    ZeroMemory(&m_dragStartWindowRect, sizeof(m_dragStartWindowRect));
}

KeyboardWindow::~KeyboardWindow() {
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

    // Show window without taking focus
    ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
    UpdateWindow(m_hWnd);

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

    if (keyCount <= 5) {
        cols = static_cast<int>(keyCount);
        rows = 1;
    } else {
        cols = (static_cast<int>(keyCount) + 1) / 2;
        rows = 2;
    }

    int winW = padX * 2 + cols * keyW + (cols - 1) * keyGap;
    int winH = headerH + padY * 2 + rows * keyH + (rows - 1) * keyGap;

    // Minimum width for header controls (Title + Shift + Min + Close)
    if (winW < 260) {
        winW = 260;
    }

    m_rcHeader = { 0, 0, winW, headerH };
    m_rcCloseBtn = { winW - 28, 5, winW - 6, 27 };
    m_rcMinBtn   = { winW - 54, 5, winW - 32, 27 };
    m_rcShiftBtn = { winW - 108, 5, winW - 58, 27 };

    // Calculate each key rectangle
    m_keyRects.clear();
    int startY = headerH + padY;

    // Center keys horizontally if winW is larger
    int keysBlockW = cols * keyW + (cols - 1) * keyGap;
    int startX = (winW - keysBlockW) / 2;

    for (size_t i = 0; i < keyCount; ++i) {
        int r = static_cast<int>(i / cols);
        int c = static_cast<int>(i % cols);

        int kx = startX + c * (keyW + keyGap);
        int ky = startY + r * (keyH + keyGap);

        RECT rk = { kx, ky, kx + keyW, ky + keyH };
        m_keyRects.push_back(rk);
    }

    if (m_hWnd) {
        SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, winW, winH, SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        // Apply smooth rounded window shape
        HRGN hRgn = CreateRoundRectRgn(0, 0, winW + 1, winH + 1, 10, 10);
        SetWindowRgn(m_hWnd, hRgn, TRUE);
        InvalidateRect(m_hWnd, NULL, FALSE);
    }
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

    case WM_APP_TRAY:
        if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
            m_trayIcon.ShowContextMenu(m_hWnd, m_layouts, m_activeLayoutIndex, IsVisible());
        } else if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK) {
            ToggleVisibility();
        }
        return 0;

    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        if (id == IDM_TRAY_SHOW) {
            ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
        } else if (id == IDM_TRAY_HIDE) {
            ShowWindow(m_hWnd, SW_HIDE);
        } else if (id == IDM_TRAY_EXIT) {
            DestroyWindow(m_hWnd);
        } else if (id >= IDM_TRAY_LAYOUT_BASE && id < IDM_TRAY_LAYOUT_BASE + m_layouts.size()) {
            SetLayout(id - IDM_TRAY_LAYOUT_BASE);
        }
        return 0;
    }

    case WM_DESTROY:
        m_trayIcon.Remove();
        PostQuitMessage(0);
        return 0;

    default:
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
    if (PtInRect(&m_rcShiftBtn, pt)) return false;
    if (PtInRect(&m_rcMinBtn, pt)) return false;
    if (PtInRect(&m_rcCloseBtn, pt)) return false;
    return true;
}

void KeyboardWindow::OnLButtonDown(int x, int y) {
    POINT pt = { x, y };

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

    if (PtInRect(&m_rcMinBtn, pt)) {
        m_pressedHeaderBtn = 2;
        InvalidateRect(m_hWnd, &m_rcMinBtn, FALSE);
        return;
    }

    if (PtInRect(&m_rcCloseBtn, pt)) {
        m_pressedHeaderBtn = 3;
        InvalidateRect(m_hWnd, &m_rcCloseBtn, FALSE);
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
    }

    if (m_pressedHeaderBtn == 2 && PtInRect(&m_rcMinBtn, pt)) {
        ShowWindow(m_hWnd, SW_HIDE);
    } else if (m_pressedHeaderBtn == 3 && PtInRect(&m_rcCloseBtn, pt)) {
        DestroyWindow(m_hWnd);
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
    else if (PtInRect(&m_rcMinBtn, pt)) newHoveredHeaderBtn = 2;
    else if (PtInRect(&m_rcCloseBtn, pt)) newHoveredHeaderBtn = 3;

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

    // 1. Draw Title: Language Name (e.g. "German")
    SelectObject(memDC, m_hFontTitle);
    SetTextColor(memDC, RGB(242, 243, 245));
    RECT titleRc = { 12, 0, m_rcShiftBtn.left - 4, m_rcHeader.bottom };
    const auto& activeLayout = m_layouts[m_activeLayoutIndex];
    DrawTextW(memDC, activeLayout.name.c_str(), -1, &titleRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    // 2. Draw Shift Button
    COLORREF shiftBg = m_shiftActive ? RGB(88, 101, 242) : (m_hoveredHeaderBtn == 1 ? RGB(60, 63, 69) : RGB(49, 51, 56));
    COLORREF shiftBorder = m_shiftActive ? RGB(114, 137, 218) : RGB(70, 73, 80);
    COLORREF shiftText = m_shiftActive ? RGB(255, 255, 255) : RGB(180, 185, 195);
    DrawRoundedBox(memDC, m_rcShiftBtn, shiftBg, shiftBorder, 4);

    SelectObject(memDC, m_hFontSmallBtn);
    SetTextColor(memDC, shiftText);
    RECT shiftTextRc = m_rcShiftBtn;
    DrawTextW(memDC, L"SHIFT", -1, &shiftTextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 3. Draw Minimize Button [–]
    COLORREF minBg = (m_hoveredHeaderBtn == 2) ? RGB(60, 63, 69) : RGB(49, 51, 56);
    COLORREF minBorder = RGB(70, 73, 80);
    DrawRoundedBox(memDC, m_rcMinBtn, minBg, minBorder, 4);

    SelectObject(memDC, m_hFontSmallBtn);
    SetTextColor(memDC, RGB(200, 204, 210));
    RECT minTextRc = m_rcMinBtn;
    DrawTextW(memDC, L"–", -1, &minTextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 4. Draw Close Button [✕]
    COLORREF closeBg = (m_hoveredHeaderBtn == 3) ? RGB(237, 66, 69) : RGB(49, 51, 56);
    COLORREF closeBorder = (m_hoveredHeaderBtn == 3) ? RGB(237, 66, 69) : RGB(70, 73, 80);
    COLORREF closeText = (m_hoveredHeaderBtn == 3) ? RGB(255, 255, 255) : RGB(200, 204, 210);
    DrawRoundedBox(memDC, m_rcCloseBtn, closeBg, closeBorder, 4);

    RECT closeTextRc = m_rcCloseBtn;
    DrawTextW(memDC, L"✕", -1, &closeTextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

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
