// (c) 2026 MiraNova Studio
#pragma once
#include <windows.h>

namespace Settings {
    inline constexpr wchar_t kRegKey[] = L"Software\\SyndicateKeyboard";
    inline constexpr wchar_t kStartMinimized[] = L"StartMinimized";

    inline bool LoadStartMinimized() {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, kRegKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            return false;
        }

        DWORD value = 0;
        DWORD size = sizeof(value);
        DWORD type = 0;
        LONG result = RegQueryValueExW(hKey, kStartMinimized, nullptr, &type,
            reinterpret_cast<LPBYTE>(&value), &size);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS && type == REG_DWORD && value != 0;
    }

    inline void SaveStartMinimized(bool enabled) {
        HKEY hKey = nullptr;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, kRegKey, 0, nullptr, 0, KEY_WRITE,
                nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
            return;
        }

        DWORD value = enabled ? 1u : 0u;
        RegSetValueExW(hKey, kStartMinimized, 0, REG_DWORD,
            reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(hKey);
    }
}
