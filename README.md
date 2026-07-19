# ProperNotepad

ProperNotepad is a small, dark-themed tabbed text editor for Linux. Its defining
feature is automatic session recovery: open tabs and their contents are restored
the next time the application starts, even when the notes were never saved to a
normal file.

## Current features

- Dark theme
- Multiple movable tabs
- New, open, save, and save-as operations
- Undo, redo, cut, copy, paste, and select all
- Automatic recovery of every tab, including unnamed and unsaved notes
- Restores the selected tab, cursor positions, and window size
- Atomic writes for both normal files and recovery data

Closing the application preserves all tabs without asking you to save them.
Closing one tab manually still asks whether its modified contents should be
saved or discarded.

## Install the build tools

### Pop!_OS, Ubuntu, Debian, or Linux Mint

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build qt6-qtbase-devel
```

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake ninja qt6-base
```

## Download, build, and run

```bash
git clone https://github.com/SjuskeT/ProperNotepad.git
cd ProperNotepad
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/ProperNotepad
```

After changing the code, only the final two commands are needed to rebuild and
run it again.

## Optional system-wide installation

```bash
sudo cmake --install build
ProperNotepad
```

By default this installs the executable in `/usr/local/bin`.

## Recovery data

ProperNotepad automatically writes its recovery session shortly after each edit
and once more during a normal shutdown. On a typical Linux desktop, Qt stores it
at:

```text
~/.local/share/SjuskeT/ProperNotepad/session.json
```

The recovery file is internal application data. Use **File > Save** when a note
should become a normal file that you can move or share.

## Project layout

```text
CMakeLists.txt          Build configuration
src/main.cpp            Application startup and dark theme
src/mainwindow.*        Tabs, menus, files, and session recovery
src/editortab.*         One text editor tab and its metadata
```
