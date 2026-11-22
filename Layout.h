// (c) 2026 MiraNova Studio
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
    std::wstring id;            // Unique identifier e.g. L"de", L"es", L"fr", L"ru"
    std::wstring name;          // Display name e.g. L"French"
    std::wstring description;   // Detailed description for menu
    std::vector<KeyDefinition> keys;
    int preferredCols = 0;      // Columns per row (0 = auto)
};

inline std::vector<LanguageLayout> GetSupportedLayouts() {
    std::vector<LanguageLayout> layouts;

    // 1. German Layout (4 keys: ä, ö, ü, ß)
    LanguageLayout german;
    german.id = L"de";
    german.name = L"German";
    german.description = L"German (ä, ö, ü, ß)";
    german.preferredCols = 4;
    german.keys = {
        { L"\u00E4", L"\u00C4", 0x00E4, 0x00C4 }, // ä, Ä
        { L"\u00F6", L"\u00D6", 0x00F6, 0x00D6 }, // ö, Ö
        { L"\u00FC", L"\u00DC", 0x00FC, 0x00DC }, // ü, Ü
        { L"\u00DF", L"\u1E9E", 0x00DF, 0x1E9E }  // ß, ẞ
    };
    layouts.push_back(german);

    // 2. Spanish Layout (9 keys)
    LanguageLayout spanish;
    spanish.id = L"es";
    spanish.name = L"Spanish";
    spanish.description = L"Spanish (á, é, í, ó, ú, ü, ñ, ¿, ¡)";
    spanish.preferredCols = 5;
    spanish.keys = {
        // Row 1: Accented vowels
        { L"\u00E1", L"\u00C1", 0x00E1, 0x00C1 }, // á, Á
        { L"\u00E9", L"\u00C9", 0x00E9, 0x00C9 }, // é, É
        { L"\u00ED", L"\u00CD", 0x00ED, 0x00CD }, // í, Í
        { L"\u00F3", L"\u00D3", 0x00F3, 0x00D3 }, // ó, Ó
        { L"\u00FA", L"\u00DA", 0x00FA, 0x00DA }, // ú, Ú
        // Row 2: Diaeresis, tilde, persistent punctuation
        { L"\u00FC", L"\u00DC", 0x00FC, 0x00DC }, // ü, Ü
        { L"\u00F1", L"\u00D1", 0x00F1, 0x00D1 }, // ñ, Ñ
        { L"\u00BF", L"\u00BF", 0x00BF, 0x00BF }, // ¿, ¿
        { L"\u00A1", L"\u00A1", 0x00A1, 0x00A1 }  // ¡, ¡
    };
    layouts.push_back(spanish);

    // 3. French Layout (16 keys: 2 rows of 8)
    LanguageLayout french;
    french.id = L"fr";
    french.name = L"French";
    french.description = L"French (à, â, æ, ç, é, è, ê, ë, î, ï, ô, œ, ù, û, ü, ÿ)";
    french.preferredCols = 8;
    french.keys = {
        // Row 1 (8 keys)
        { L"\u00E0", L"\u00C0", 0x00E0, 0x00C0 }, // à, À
        { L"\u00E2", L"\u00C2", 0x00E2, 0x00C2 }, // â, Â
        { L"\u00E6", L"\u00C6", 0x00E6, 0x00C6 }, // æ, Æ
        { L"\u00E7", L"\u00C7", 0x00E7, 0x00C7 }, // ç, Ç
        { L"\u00E9", L"\u00C9", 0x00E9, 0x00C9 }, // é, É
        { L"\u00E8", L"\u00C8", 0x00E8, 0x00C8 }, // è, È
        { L"\u00EA", L"\u00CA", 0x00EA, 0x00CA }, // ê, Ê
        { L"\u00EB", L"\u00CB", 0x00EB, 0x00CB }, // ë, Ë

        // Row 2 (8 keys)
        { L"\u00EE", L"\u00CE", 0x00EE, 0x00CE }, // î, Î
        { L"\u00EF", L"\u00CF", 0x00EF, 0x00CF }, // ï, Ï
        { L"\u00F4", L"\u00D4", 0x00F4, 0x00D4 }, // ô, Ô
        { L"\u0153", L"\u0152", 0x0153, 0x0152 }, // œ, Œ
        { L"\u00F9", L"\u00D9", 0x00F9, 0x00D9 }, // ù, Ù
        { L"\u00FB", L"\u00DB", 0x00FB, 0x00DB }, // û, Û
        { L"\u00FC", L"\u00DC", 0x00FC, 0x00DC }, // ü, Ü
        { L"\u00FF", L"\u0178", 0x00FF, 0x0178 }  // ÿ, Ÿ
    };
    layouts.push_back(french);

    // 4. Italian Layout (9 keys: à, è, é, ì, í, ò, ó, ù, ú)
    LanguageLayout italian;
    italian.id = L"it";
    italian.name = L"Italian";
    italian.description = L"Italian (à, è, é, ì, í, ò, ó, ù, ú)";
    italian.preferredCols = 5;
    italian.keys = {
        // Row 1: A, E, I vowels (5 keys)
        { L"\u00E0", L"\u00C0", 0x00E0, 0x00C0 }, // à, À
        { L"\u00E8", L"\u00C8", 0x00E8, 0x00C8 }, // è, È
        { L"\u00E9", L"\u00C9", 0x00E9, 0x00C9 }, // é, É
        { L"\u00EC", L"\u00CC", 0x00EC, 0x00CC }, // ì, Ì
        { L"\u00ED", L"\u00CD", 0x00ED, 0x00CD }, // í, Í
        // Row 2: O, U vowels (4 keys)
        { L"\u00F2", L"\u00D2", 0x00F2, 0x00D2 }, // ò, Ò
        { L"\u00F3", L"\u00D3", 0x00F3, 0x00D3 }, // ó, Ó
        { L"\u00F9", L"\u00D9", 0x00F9, 0x00D9 }, // ù, Ù
        { L"\u00FA", L"\u00DA", 0x00FA, 0x00DA }  // ú, Ú
    };
    layouts.push_back(italian);

    // 5. Russian Layout (33 keys: 3 rows of 11)
    LanguageLayout russian;
    russian.id = L"ru";
    russian.name = L"Russian";
    russian.description = L"Russian (а, б, в, г, д, е, ё, ж, з, и, й, к, л, м, н, о, п, р, с, т, у, ф, х, ц, ч, ш, щ, ъ, ы, ь, э, ю, я)";
    russian.preferredCols = 11;
    russian.keys = {
        // Row 1
        { L"\u0430", L"\u0410", 0x0430, 0x0410 }, // а, А
        { L"\u0431", L"\u0411", 0x0431, 0x0411 }, // б, Б
        { L"\u0432", L"\u0412", 0x0432, 0x0412 }, // в, В
        { L"\u0433", L"\u0413", 0x0433, 0x0413 }, // г, Г
        { L"\u0434", L"\u0414", 0x0434, 0x0414 }, // д, Д
        { L"\u0435", L"\u0415", 0x0435, 0x0415 }, // е, Е
        { L"\u0451", L"\u0401", 0x0451, 0x0401 }, // ё, Ё
        { L"\u0436", L"\u0416", 0x0436, 0x0416 }, // ж, Ж
        { L"\u0437", L"\u0417", 0x0437, 0x0417 }, // з, З
        { L"\u0438", L"\u0418", 0x0438, 0x0418 }, // и, И
        { L"\u0439", L"\u0419", 0x0439, 0x0419 }, // й, Й
        // Row 2
        { L"\u043A", L"\u041A", 0x043A, 0x041A }, // к, К
        { L"\u043B", L"\u041B", 0x043B, 0x041B }, // л, Л
        { L"\u043C", L"\u041C", 0x043C, 0x041C }, // м, М
        { L"\u043D", L"\u041D", 0x043D, 0x041D }, // н, Н
        { L"\u043E", L"\u041E", 0x043E, 0x041E }, // о, О
        { L"\u043F", L"\u041F", 0x043F, 0x041F }, // п, П
        { L"\u0440", L"\u0420", 0x0440, 0x0420 }, // р, Р
        { L"\u0441", L"\u0421", 0x0441, 0x0421 }, // с, С
        { L"\u0442", L"\u0422", 0x0442, 0x0422 }, // т, Т
        { L"\u0443", L"\u0423", 0x0443, 0x0423 }, // у, У
        { L"\u0444", L"\u0424", 0x0444, 0x0424 }, // ф, Ф
        // Row 3
        { L"\u0445", L"\u0425", 0x0445, 0x0425 }, // х, Х
        { L"\u0446", L"\u0426", 0x0446, 0x0426 }, // ц, Ц
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
