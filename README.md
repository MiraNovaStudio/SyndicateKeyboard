# Syndicate Virtual Soft Keyboard

A lightweight, high-performance floating virtual soft keyboard utility built in native Visual C++ (Win32 API). It provides fast, seamless access to international characters without stealing keyboard or cursor focus from your active foreground applications (e.g., Notepad, Word, web browsers, or IDEs).

---

## Features

- **Focus-Preserving Input (`WS_EX_NOACTIVATE`)**:
  - The window never takes focus away from the document you are typing in.
  - Clicks are intercepted cleanly and injected directly at the active cursor using Win32 `SendInput` with `KEYEVENTF_UNICODE`.
- **Always on Top**:
  - Floats above all other desktop applications with `WS_EX_TOPMOST`.
- **Freely Draggable Anywhere**:
  - Click and drag the header bar to move the keyboard anywhere across single or multi-monitor setups without interrupting background typing.
- **German Layout (4 Keys)**:
  - Dedicated keys for **ä**, **ö**, **ü**, and **ß** (eszett).
  - An optional German variant with **ë** is selectable via the system tray menu.
- **Multiple Ways to Type Uppercase**:
  - Click the **`SHIFT`** toggle button in the header bar (`Ä`, `Ö`, `Ü`, `ẞ`).
  - Hold the physical **Shift** key while clicking any button.
  - **Right-click** any key to immediately type its uppercase character.
  - Visual lowercase keys feature subtle corner previews of their uppercase counterpart.
- **System Tray Integration**:
  - Runs with a notification area tray icon.
  - **Left-click / Double-click**: Toggle show/hide.
  - **Right-click Menu**: Show/Hide, Language Layout selection, and Exit.
- **Extensible Multi-Language Architecture**:
  - Modular data-driven design in `Layout.h`.
  - Already pre-configured for upcoming **Spanish** (`ñ`, `á`, `é`, `í`, `ó`, `ú`, `¿`, `¡`) and **Russian** (`ж`, `ч`, `ш`, `щ`, `ъ`, `ы`, `ь`, `э`, `ю`, `я`) layouts.
- **Per-Monitor V2 DPI Awareness**:
  - Crisp typography and sharp vector-like rendering across 100%, 125%, 150%, and 200%+ scaling on Windows 10/11.

---

## Project Structure

```text
SyndicateKeyboard/
├── Layout.h                 # Language and key definitions (German, Spanish, Russian)
├── InputSender.h / .cpp     # Win32 SendInput Unicode character injection engine
├── TrayIcon.h / .cpp        # System tray icon (Shell_NotifyIcon) and context menu
├── KeyboardWindow.h / .cpp  # Double-buffered floating window, dragging, and UI rendering
├── SyndicateKeyboard.cpp    # Application entry point (wWinMain), DPI awareness, mutex
├── SyndicateKeyboard.vcxproj# Visual C++ project file (configured for Windows SubSystem)
└── README.md                # Project documentation
```

---

## Building the Project

### Requirements
- Windows 10 or 11
- Visual Studio (2022 / 2026) with the **Desktop development with C++** workload
- MSVC Toolset supporting C++20

### Build via Visual Studio
1. Open `SyndicateKeyboard.slnx` or `SyndicateKeyboard.vcxproj` in Visual Studio.
2. Select **Release** configuration and **x64** platform.
3. Build the solution (**Ctrl + Shift + B** or `Build > Build Solution`).

### Build via Command Line (MSBuild)
Run from a Developer Command Prompt or PowerShell:
```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" SyndicateKeyboard.vcxproj -p:Configuration=Release -p:Platform=x64
```

The output executable will be generated at:
```text
x64\Release\SyndicateKeyboard.exe
```

---

## How to Use

1. **Launch**:
   Run `x64\Release\SyndicateKeyboard.exe`. The keyboard will appear floating in the lower-right area of the screen.
2. **Type**:
   Open any text editor (Notepad, Word, browser search bar, etc.). Click any key on the soft keyboard to insert characters directly at your cursor.
3. **Move**:
   Click and drag the top header bar (the title area labeled "German") to reposition the keyboard anywhere on your desktop.
4. **Shift Mode**:
   Click the **SHIFT** button in the top-right of the keyboard header, or right-click any key for a one-off capital letter.
5. **System Tray**:
   Find the keyboard icon in the taskbar notification tray to hide/show the window, switch language layouts, or exit.
