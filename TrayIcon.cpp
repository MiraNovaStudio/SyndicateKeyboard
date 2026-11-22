#include "TrayIcon.h"
#include <cstdint>

TrayIcon::TrayIcon()
    : m_hWnd(nullptr),
      m_hInstance(nullptr),
      m_hIcon(nullptr),
      m_added(false) {
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
    int iconW = GetSystemMetrics(SM_CXSMICON);
    int iconH = GetSystemMetrics(SM_CYSMICON);
    if (iconW <= 0) iconW = 16;
    if (iconH <= 0) iconH = 16;

    HDC hScreenDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreenDC);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = iconW;
    bmi.bmiHeader.biHeight = -iconH; // top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pBits = nullptr;
    HBITMAP hColorBmp = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
    if (!hColorBmp || !pBits) {
        if (hMemDC) DeleteDC(hMemDC);
        if (hScreenDC) ReleaseDC(NULL, hScreenDC);
        return LoadIcon(NULL, IDI_APPLICATION);
    }

    // Initialize all pixels to transparent
    memset(pBits, 0, iconW * iconH * sizeof(uint32_t));

    HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hColorBmp);

    // Keyboard body background - Vibrant Discord/VSCode Indigo (visible on both dark & light taskbars)
    HBRUSH hBodyBrush = CreateSolidBrush(RGB(88, 101, 242));
    HPEN hBodyPen = CreatePen(PS_SOLID, 1, RGB(114, 137, 218));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hMemDC, hBodyBrush);
    HPEN hOldPen = (HPEN)SelectObject(hMemDC, hBodyPen);

    int margin = (iconW >= 24) ? 2 : 1;
    int roundR = (iconW >= 24) ? 5 : 3;
    RoundRect(hMemDC, margin, margin, iconW - margin, iconH - margin, roundR, roundR);

    // Draw keys inside
    HBRUSH hKeyBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hMemDC, hKeyBrush);
    HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
    SelectObject(hMemDC, hNullPen);

    if (iconW >= 24) {
        // High DPI layout: 2 rows of keys + spacebar
        Rectangle(hMemDC, 5, 5, 9, 9);
        Rectangle(hMemDC, 10, 5, 14, 9);
        Rectangle(hMemDC, 15, 5, 19, 9);

        Rectangle(hMemDC, 5, 11, 9, 15);
        Rectangle(hMemDC, 10, 11, 14, 15);
        Rectangle(hMemDC, 15, 11, 19, 15);

        Rectangle(hMemDC, 7, 17, 17, 20);
    } else {
        // Standard 16x16 layout
        RECT k1 = { 3, 3, 6, 6 };
        RECT k2 = { 7, 3, 10, 6 };
        RECT k3 = { 11, 3, 14, 6 };

        RECT k4 = { 3, 7, 6, 10 };
        RECT k5 = { 7, 7, 10, 10 };
        RECT k6 = { 11, 7, 14, 10 };

        RECT k7 = { 5, 11, 12, 13 };

        FillRect(hMemDC, &k1, hKeyBrush);
        FillRect(hMemDC, &k2, hKeyBrush);
        FillRect(hMemDC, &k3, hKeyBrush);
        FillRect(hMemDC, &k4, hKeyBrush);
        FillRect(hMemDC, &k5, hKeyBrush);
        FillRect(hMemDC, &k6, hKeyBrush);
        FillRect(hMemDC, &k7, hKeyBrush);
    }

    SelectObject(hMemDC, hOldBrush);
    SelectObject(hMemDC, hOldPen);
    DeleteObject(hBodyBrush);
    DeleteObject(hBodyPen);
    DeleteObject(hKeyBrush);

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hScreenDC);

    // Create 1-bit mask bitmap
    // Mask bit 0 = use color bitmap pixel, mask bit 1 = transparent
    int maskPitch = ((iconW + 31) / 32) * 4;
    std::vector<BYTE> maskBits(maskPitch * iconH, 0xFF); // Default all transparent (1)

    for (int y = 0; y < iconH; ++y) {
        for (int x = 0; x < iconW; ++x) {
            int idx = y * iconW + x;
            uint32_t color = pBits[idx] & 0x00FFFFFF;
            if (color != 0) {
                // Opaque pixel: set alpha to 0xFF
                pBits[idx] = color | 0xFF000000;
                // Clear mask bit (0 = show color)
                int maskByte = y * maskPitch + (x / 8);
                maskBits[maskByte] &= ~(0x80 >> (x % 8));
            } else {
                pBits[idx] = 0x00000000;
            }
        }
    }

    HBITMAP hMaskBmp = CreateBitmap(iconW, iconH, 1, 1, maskBits.data());

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

    m_hWnd = hWnd;
    m_hInstance = hInstance;

    if (!m_hIcon) {
        m_hIcon = CreateProceduralIcon(hInstance);
    }

    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = hWnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_APP_TRAY;
    m_nid.hIcon = m_hIcon;
    wcscpy_s(m_nid.szTip, L"Syndicate Keyboard");

    // Try standard cbSize first
    BOOL ok = Shell_NotifyIconW(NIM_ADD, &m_nid);
    if (!ok) {
        // Fallback to V3 size if newer version is unsupported
        m_nid.cbSize = NOTIFYICONDATAW_V3_SIZE;
        ok = Shell_NotifyIconW(NIM_ADD, &m_nid);
        if (!ok) {
            // Fallback to V2 size
            m_nid.cbSize = NOTIFYICONDATAW_V2_SIZE;
            ok = Shell_NotifyIconW(NIM_ADD, &m_nid);
        }
    }

    m_added = (ok != FALSE);
    return m_added;
}

bool TrayIcon::ReAdd() {
    Remove();
    if (m_hWnd && m_hInstance) {
        return Initialize(m_hWnd, m_hInstance);
    }
    return false;
}

void TrayIcon::Remove() {
    if (m_added) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_added = false;
    }
}

void TrayIcon::ShowBalloon(const std::wstring& title, const std::wstring& text) {
    if (!m_added) return;
    NOTIFYICONDATAW nid = m_nid;
    nid.uFlags |= NIF_INFO;
    wcsncpy_s(nid.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(nid.szInfo, text.c_str(), _TRUNCATE);
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 3000;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void TrayIcon::ShowContextMenu(HWND hWnd, const std::vector<LanguageLayout>& layouts, size_t activeLayoutIndex, bool isWindowVisible, bool startMinimized) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    if (isWindowVisible) {
        AppendMenuW(hMenu, MF_STRING, IDM_TRAY_HIDE, L"Hide Keyboard");
        SetMenuDefaultItem(hMenu, IDM_TRAY_HIDE, FALSE);
    } else {
        AppendMenuW(hMenu, MF_STRING, IDM_TRAY_SHOW, L"Show Keyboard");
        SetMenuDefaultItem(hMenu, IDM_TRAY_SHOW, FALSE);
    }

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);

    HMENU hSubMenu = CreatePopupMenu();
    for (size_t i = 0; i < layouts.size(); ++i) {
        UINT flags = MF_STRING;
        if (i == activeLayoutIndex) {
            flags |= MF_CHECKED;
        }
        AppendMenuW(hSubMenu, flags, IDM_TRAY_LAYOUT_BASE + i, layouts[i].name.c_str());
    }

    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Language Layout");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    UINT startMinFlags = MF_STRING;
    if (startMinimized) {
        startMinFlags |= MF_CHECKED;
    }
    AppendMenuW(hMenu, startMinFlags, IDM_TRAY_START_MINIMIZED, L"Start Minimized");

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, IDM_TRAY_EXIT, L"Exit");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);
    PostMessageW(hWnd, WM_NULL, 0, 0);

    DestroyMenu(hMenu);
}
