# Syndicate Keyboard

A floating virtual keyboard for Windows that types international characters into the app you are already using. It stays on top, never steals focus, and injects Unicode at the caret with Win32 `SendInput`.

Built in native C++ (Win32 API). Single executable, no installer, no runtime beyond Windows 10/11.

## Run

A prebuilt 64-bit Release binary is in the repo. On Windows 10 or 11, clone or download the project and double-click:

```text
x64\Release\SyndicateKeyboard.exe
```

No Visual Studio install is required to run it. Windows SmartScreen may warn on the first launch because the exe is unsigned; choose **More info** → **Run anyway** if you trust the build.

---

## Features

- **Focus stays in your document** — the keyboard is `WS_EX_NOACTIVATE`, so clicks never pull focus away from Notepad, Word, a browser, or an IDE.
- **Always on top** and freely draggable from the header bar, including across monitors.
- **Language dropdown** on the header (`German ▾`) plus the same layouts in the tray menu.
- **Uppercase** via the on-screen **SHIFT** toggle, the physical Shift key, or a right-click on any key. Lowercase keys show a small uppercase hint.
- **System tray** — left-click show/hide; right-click for layouts, **Start Minimized**, and Exit. The **X** button hides to the tray instead of quitting.
- **Stays on screen** when you switch to a wider layout (for example Russian) or restore from the tray; the window is clamped to the current monitor’s work area.
- **Single instance** — launching again restores the existing window.
- **Per-monitor DPI awareness (v2)** on Windows 10/11.

---

## Languages

| Layout | Keys |
| --- | --- |
| **German** (default) | ä ö ü ß |
| **Spanish** | á é í ó ú ü ñ ¿ ¡ |
| **French** | à â æ ç é è ê ë î ï ô œ ù û ü ÿ |
| **Italian** | à è é ì í ò ó ù ú |
| **Russian** | Full alphabet in 3 rows of 11: а–й, к–ф, х–я |

Shift produces the matching capitals (including ẞ, Ÿ, Ё, and so on). Spanish **¿** and **¡** stay the same with or without Shift.

### German shortcuts

These work only while the keyboard window is **visible**. They type into the focused app. Hide the keyboard and the shortcuts are unregistered.

| Shortcut | Character |
| --- | --- |
| **Alt+A** | ä |
| **Alt+O** | ö |
| **Alt+U** | ü |
| **Alt+S** | ß |

Hold **Shift** (or enable on-screen SHIFT) for **Ä Ö Ü ẞ**.

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

That overwrites `x64\Release\SyndicateKeyboard.exe`. Debug builds, PDBs, and the `Syndicat.eaa90256\` compiler folder stay local and are gitignored.

---

## Usage

1. Run `x64\Release\SyndicateKeyboard.exe`. The keyboard appears in the lower-right of the primary screen (unless **Start Minimized** is on).
2. Click a key to insert that character at the caret in the foreground app.
3. Drag the header to move the window. Switch languages from the header dropdown or the tray menu.
4. Click **X** to hide to the tray. Left-click the tray icon to show it again. Right-click the tray icon for **Exit**.

---

## Preferences

**Start Minimized** is a checkable item on the tray context menu. It is stored per user in:

```text
HKCU\Software\SyndicateKeyboard
```

When enabled, the next launch keeps the keyboard in the tray until you show it.

---

## Source layout

```text
SyndicateKeyboard/
├── Layout.h                 # Language and key definitions
├── Settings.h               # Start Minimized persistence
├── InputSender.h / .cpp     # Unicode injection via SendInput
├── TrayIcon.h / .cpp        # Notification-area icon and menu
├── KeyboardWindow.h / .cpp  # Floating window, hotkeys, rendering
├── SyndicateKeyboard.cpp    # wWinMain, DPI, single-instance mutex
├── SyndicateKeyboard.slnx   # Solution
├── SyndicateKeyboard.vcxproj
├── x64/Release/SyndicateKeyboard.exe  # Prebuilt binary
└── README.md
```
