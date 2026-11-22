# Syndicate Keyboard

**Version 1.0**

A floating virtual keyboard for Windows that types international characters into the app you are already using. It stays on top, never steals focus, and injects Unicode at the caret with Win32 `SendInput`.

Built in native C++ (Win32 API). One executable, no installer, and no extra runtime beyond Windows 10 or 11.

## Features

- **Focus stays in your document** — the window is created with `WS_EX_NOACTIVATE`, so clicks never pull focus away from Notepad, Word, a browser, or an IDE.
- **Always on top** and freely draggable from the header bar, including across monitors.
- **Language dropdown** on the header (`German ▾`), with the same layouts in the tray menu.
- **Uppercase** via the on-screen **SHIFT** toggle, the physical Shift key, or a right-click on any key. Lowercase keys show a small uppercase hint.
- **System tray** — left-click to show or hide; right-click for layouts, **Start Minimized**, **About**, and Exit. The **X** button hides to the tray instead of quitting.
- **Stays on screen** when you switch to a wider layout (for example Russian) or restore from the tray. The window is clamped to the current monitor's work area.
- **Single instance** — launching the app again restores the existing window.
- **Per-monitor DPI awareness (v2)** on Windows 10 and 11.

---

## Languages

| Layout | Keys |
| --- | --- |
| **German** (default) | ä ö ü ß |
| **Spanish** | á é í ó ú ü ñ ¿ ¡ |
| **French** | à â æ ç é è ê ë î ï ô œ ù û ü ÿ |
| **Italian** | à è é ì í ò ó ù ú |
| **Russian** | Full alphabet in three rows of 11: а–й, к–ф, х–я |

Shift produces the matching capitals (including ẞ, Ÿ, and Ё). Spanish **¿** and **¡** stay the same with or without Shift.

### German shortcuts

These type into the focused app and work whenever the keyboard is **visible**, even if another language is selected. Hiding the keyboard turns the shortcuts off.

| Shortcut | Character |
| --- | --- |
| **Alt+A** | ä |
| **Alt+O** | ö |
| **Alt+U** | ü |
| **Alt+S** | ß |

Hold **Shift** (or turn on on-screen SHIFT) for **Ä Ö Ü ẞ**.

---

## Requirements

**To run:** Windows 10 or 11 (64-bit).

**To build from source:** [Visual Studio](https://visualstudio.microsoft.com/) 2022 or later with the **Desktop development with C++** workload, and an MSVC toolset with C++20.

---

## Building

Open `SyndicateKeyboard.slnx` or `SyndicateKeyboard.vcxproj`, choose **Release | x64**, and build.

From a Developer PowerShell or Developer Command Prompt:

```powershell
msbuild SyndicateKeyboard.vcxproj -p:Configuration=Release -p:Platform=x64
```

If `msbuild` is not on your PATH, use the copy under your Visual Studio install, for example:

```powershell
& "${env:ProgramFiles}\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" `
    SyndicateKeyboard.vcxproj -p:Configuration=Release -p:Platform=x64
```

The executable is written to `x64\Release\SyndicateKeyboard.exe`. Build outputs are gitignored.

---

## Usage

1. Run `x64\Release\SyndicateKeyboard.exe`. The keyboard appears in the lower right of the primary screen unless **Start Minimized** is on.
2. Click a key to insert that character at the caret in the foreground app.
3. Drag the header to move the window. Switch languages from the header dropdown or the tray menu.
4. Click **X** to hide to the tray. Left-click the tray icon to show it again. Right-click the tray icon for layouts, **Start Minimized**, **About**, and **Exit**.

---

## Preferences

**Start Minimized** is a checkable item on the tray context menu. It is stored per user at:

```text
HKCU\Software\SyndicateKeyboard
```

When it is enabled, the next launch keeps the keyboard in the tray until you show it.

---

## Source layout

```text
SyndicateKeyboard/
├── Version.h                # Product version (1.0)
├── Layout.h                 # Language and key definitions
├── Settings.h               # Start Minimized persistence
├── InputSender.h / .cpp     # Unicode injection via SendInput
├── TrayIcon.h / .cpp        # Notification-area icon and menu
├── KeyboardWindow.h / .cpp  # Floating window, hotkeys, rendering
├── SyndicateKeyboard.cpp    # wWinMain, DPI, single-instance mutex
├── SyndicateKeyboard.rc     # VERSIONINFO resource
├── SyndicateKeyboard.slnx   # Solution
├── SyndicateKeyboard.vcxproj
└── README.md
```

---

## License

Syndicate Keyboard is licensed under the [MIT License](LICENSE).

Copyright (c) 2026 MiraNova Studio
