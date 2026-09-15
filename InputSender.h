#pragma once
#include <windows.h>

class InputSender {
public:
    // Sends a Unicode character to the active foreground application without stealing focus
    static void SendUnicodeCharacter(wchar_t ch);
};
