#include "TrayIcon.h"

TrayIcon::TrayIcon() : m_hIcon(nullptr), m_added(false) {
    ZeroMemory(&m_nid, sizeof(m_nid));
}

TrayIcon::~TrayIcon() {
    Remove();
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
        m_hIcon = nullptr;
    }
}

HICON TrayIcon::CreateProceduralIcon(HINSTANCE hInstance) {
    const int iconWidth = 16;
    const int iconHeight = 16;

    HDC hScreenDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = iconWidth;
    bmi.bmiHeader.biHeight = -iconHeight; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = nullptr;
    HBITMAP hColorBmp = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hColorBmp);

    // Draw keyboard icon
    RECT rc = { 0, 0, iconWidth, iconHeight };
    HBRUSH hBgBrush = CreateSolidBrush(RGB(35, 39, 42));
    FillRect(hMemDC, &rc, hBgBrush);
    DeleteObject(hBgBrush);

    // Outer keyboard body
    HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(88, 101, 242));
    HPEN hOldPen = (HPEN)SelectObject(hMemDC, hBorderPen);
    HBRUSH hBodyBrush = CreateSolidBrush(RGB(47, 49, 54));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBodyBrush);
    RoundRect(hMemDC, 1, 2, 15, 14, 3, 3);

    // Draw 4 mini keys inside
    HBRUSH hKeyBrush = CreateSolidBrush(RGB(240, 240, 240));
    RECT k1 = { 3, 4, 6, 7 };
    RECT k2 = { 7, 4, 10, 7 };
    RECT k3 = { 11, 4, 14, 7 };
    RECT k4 = { 5, 9, 11, 12 };
    FillRect(hMemDC, &k1, hKeyBrush);
    FillRect(hMemDC, &k2, hKeyBrush);
    FillRect(hMemDC, &k3, hKeyBrush);
    FillRect(hMemDC, &k4, hKeyBrush);
    DeleteObject(hKeyBrush);

    SelectObject(hMemDC, hOldBrush);
    DeleteObject(hBodyBrush);
    SelectObject(hMemDC, hOldPen);
    DeleteObject(hBorderPen);

    // Create 1-bit mask bitmap
    HBITMAP hMaskBmp = CreateBitmap(iconWidth, iconHeight, 1, 1, NULL);
    HDC hMaskDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hOldMask = (HBITMAP)SelectObject(hMaskDC, hMaskBmp);

    // 0 in mask means visible in color bitmap
    RECT maskRc = { 0, 0, iconWidth, iconHeight };
    HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
    FillRect(hMaskDC, &maskRc, hBlack);

    SelectObject(hMaskDC, hOldMask);
    DeleteDC(hMaskDC);

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);

    ICONINFO ii = {};
    ii.fIcon = TRUE;
    ii.hbmMask = hMaskBmp;
    ii.hbmColor = hColorBmp;

    HICON hIcon = CreateIconIndirect(&ii);

    DeleteObject(hColorBmp);
    DeleteObject(hMaskBmp);

    if (!hIcon) {
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    return hIcon;
}

bool TrayIcon::Initialize(HWND hWnd, HINSTANCE hInstance) {
    if (m_added) return true;

    m_hIcon = CreateProceduralIcon(hInstance);

    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = hWnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_APP_TRAY;
    m_nid.hIcon = m_hIcon;
    wcscpy_s(m_nid.szTip, L"Syndicate Virtual Soft Keyboard");

    m_added = Shell_NotifyIconW(NIM_ADD, &m_nid) ? true : false;
    return m_added;
}

void TrayIcon::Remove() {
    if (m_added) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_added = false;
    }
}

void TrayIcon::ShowContextMenu(HWND hWnd, const std::vector<LanguageLayout>& layouts, size_t activeLayoutIndex, bool isWindowVisible) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    if (isWindowVisible) {
        AppendMenuW(hMenu, MF_STRING, IDM_TRAY_HIDE, L"Hide Keyboard");
    } else {
        AppendMenuW(hMenu, MF_STRING, IDM_TRAY_SHOW, L"Show Keyboard");
    }

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    HMENU hSubMenu = CreatePopupMenu();
    for (size_t i = 0; i < layouts.size(); ++i) {
        UINT flags = MF_STRING;
        if (i == activeLayoutIndex) {
            flags |= MF_CHECKED;
        }
        AppendMenuW(hSubMenu, flags, IDM_TRAY_LAYOUT_BASE + i, layouts[i].description.c_str());
    }
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Language Layout");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
    PostMessageW(hWnd, WM_NULL, 0, 0);

    DestroyMenu(hMenu);
}
