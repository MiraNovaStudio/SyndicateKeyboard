#pragma once
#include <string>
#include <vector>

struct KeyDefinition {
    std::wstring label;          // Display label when shift is off (e.g. L"ä")
    std::wstring shiftLabel;     // Display label when shift is on (e.g. L"Ä")
    wchar_t charToSend;         // Unicode character to inject
    wchar_t shiftCharToSend;    // Unicode character to inject with Shift
};

struct LanguageLayout {
    std::wstring id;            // Unique identifier e.g. L"de_std", L"de_e", L"es", L"ru"
    std::wstring name;          // Display name e.g. L"German"
    std::wstring description;   // Detailed description for menu
    std::vector<KeyDefinition> keys;
};

inline std::vector<LanguageLayout> GetSupportedLayouts() {
    std::vector<LanguageLayout> layouts;

    // 1. German Standard Layout (4 keys: ä, ö, ü, ß)
    LanguageLayout germanStd;
    germanStd.id = L"de_std";
    germanStd.name = L"German";
    germanStd.description = L"German (ä, ö, ü, ß)";
    germanStd.keys = {
        { L"\u00E4", L"\u00C4", 0x00E4, 0x00C4 }, // ä, Ä
        { L"\u00F6", L"\u00D6", 0x00F6, 0x00D6 }, // ö, Ö
        { L"\u00FC", L"\u00DC", 0x00FC, 0x00DC }, // ü, Ü
        { L"\u00DF", L"\u1E9E", 0x00DF, 0x1E9E }  // ß, ẞ
    };
    layouts.push_back(germanStd);

    // 2. German with ë variant (4 keys: ä, ë, ö, ß) in case user specifically needs ë
    LanguageLayout germanE;
    germanE.id = L"de_e";
    germanE.name = L"German (with ë)";
    germanE.description = L"German variant (ä, ë, ö, ß)";
    germanE.keys = {
        { L"\u00E4", L"\u00C4", 0x00E4, 0x00C4 }, // ä, Ä
        { L"\u00EB", L"\u00CB", 0x00EB, 0x00CB }, // ë, Ë
        { L"\u00F6", L"\u00D6", 0x00F6, 0x00D6 }, // ö, Ö
        { L"\u00DF", L"\u1E9E", 0x00DF, 0x1E9E }  // ß, ẞ
    };
    layouts.push_back(germanE);

    // 3. Spanish Layout (Ready for next phase)
    LanguageLayout spanish;
    spanish.id = L"es";
    spanish.name = L"Spanish";
    spanish.description = L"Spanish (ñ, á, é, í, ó, ú, ¿, ¡)";
    spanish.keys = {
        { L"\u00F1", L"\u00D1", 0x00F1, 0x00D1 }, // ñ, Ñ
        { L"\u00E1", L"\u00C1", 0x00E1, 0x00C1 }, // á, Á
        { L"\u00E9", L"\u00C9", 0x00E9, 0x00C9 }, // é, É
        { L"\u00ED", L"\u00CD", 0x00ED, 0x00CD }, // í, Í
        { L"\u00F3", L"\u00D3", 0x00F3, 0x00D3 }, // ó, Ó
        { L"\u00FA", L"\u00DA", 0x00FA, 0x00DA }, // ú, Ú
        { L"\u00BF", L"?",      0x00BF, 0x003F }, // ¿, ?
        { L"\u00A1", L"!",      0x00A1, 0x0021 }  // ¡, !
    };
    layouts.push_back(spanish);

    // 4. Russian Layout (Ready for next phase)
    LanguageLayout russian;
    russian.id = L"ru";
    russian.name = L"Russian";
    russian.description = L"Russian (ж, ч, ш, щ, ъ, ы, ь, э, ю, я)";
    russian.keys = {
        { L"\u0436", L"\u0416", 0x0436, 0x0416 }, // ж, Ж
        { L"\u0447", L"\u0427", 0x0447, 0x0427 }, // ч, Ч
        { L"\u0448", L"\u0428", 0x0448, 0x0428 }, // ш, Ш
        { L"\u0449", L"\u0429", 0x0449, 0x0429 }, // щ, Щ
        { L"\u044A", L"\u042A", 0x044A, 0x042A }, // ъ, Ъ
        { L"\u044B", L"\u042B", 0x044B, 0x042B }, // ы, Ы
        { L"\u044C", L"\u042C", 0x044C, 0x042C }, // ь, Ь
        { L"\u044D", L"\u042D", 0x044D, 0x042D }, // э, Э
        { L"\u044E", L"\u042E", 0x044E, 0x042E }, // ю, Ю
        { L"\u044F", L"\u042F", 0x044F, 0x042F }  // я, Я
    };
    layouts.push_back(russian);

    return layouts;
}
