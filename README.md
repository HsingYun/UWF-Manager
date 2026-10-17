# UWF Manager

A Qt GUI for the Windows Unified Write Filter (UWF) — a convenient graphical front end for inspecting and configuring UWF state, alongside the built-in `uwfmgr.exe` command line.

[简体中文](README.zh_CN.md)

![Screenshot](snapshot/snapshot.1.png)

![Screenshot](snapshot/snapshot.2.png)

## About UWF

The Unified Write Filter is a sector-level write-protection driver shipped with Windows Enterprise, Education, IoT Enterprise, and the LTSC variants. When a volume is protected, every write to it is intercepted and redirected to an *overlay* — either a region of RAM or a sparse file on a designated disk volume — instead of touching the underlying sectors. The overlay is discarded on reboot, returning the volume to its previous state. File and registry-key exclusions can be declared so that specific paths bypass the overlay and write through to disk; overlay contents can also be selectively *persisted* (committed back to the underlying media) before reboot.

UWF is configured per-volume and operates with two parallel state sets: the *current* session (read-only, reflecting what the driver is enforcing right now) and the *next* session (writable, taking effect after reboot). All configuration changes — enabling protection, setting the overlay type and size, adding exclusions — apply to the next session.

UWF is the supported successor to the older Enhanced Write Filter (EWF) and File-Based Write Filter (FBWF), and is the standard mechanism for building stateless kiosks, point-of-sale terminals, ATMs, medical devices, classroom workstations, and digital signage on Windows.

> **Note**: *Servicing mode* (`uwfmgr filter enable-servicing` / `disable-servicing`, used to suspend the filter for Windows Update or scheduled maintenance) is not planned.

## Features

- Filter enable / disable, current and next session
- Overlay configuration: type (RAM / Disk), maximum size, warning and critical thresholds
- Per-volume protection toggle and drive-letter / volume-ID binding
- File and directory exclusion lists per volume
- Registry exclusion list (system volume)
- DomainSecretKey / TSCAL persistence switches, shown and toggled inline in the registry exclusion list
- Persist overlay contents back to disk or registry (files, directories, file deletions, registry keys)
- File staging: persist a user-defined file/folder list in the registry and commit its current contents automatically before safe shutdown or restart
- Read-only enumeration of overlay file entries
- Import uwfmgr commands — paste or load a command script and stage each line as a pending change
- System restart and shutdown
- In-app log viewer
- Portable by default — file staging is the only feature that stores application state (its path list is kept in the registry); no configuration files are created, and the in-app log remains in memory and is discarded when the process exits

## Out of scope

- HORM (Hibernate Once / Resume Many)
- Servicing mode
- Free-space passthrough, persistent overlay, read-only media mode, swapfile creation

## Requirements

- Windows 10 / 11 Enterprise, Education, IoT Enterprise, or LTSC variants
- "Unified Write Filter" Windows feature enabled
- Administrator privileges — needed to apply changes

Note: the program still starts when these conditions are not met.

## Custom configuration

UWF Manager does not create or save preference files. To customize the startup theme or language, manually create a UTF-8 text file named **`UWF.ini`** in the same directory as the executable:

```ini
[UI]
Theme=dark
Language=en
```

| Option | Values | Default |
| --- | --- | --- |
| `[UI] Theme` | `system`, `light`, `dark` | `system` |
| `[UI] Language` | `system`, `en`, `zh_CN` | `system` |

`system` uses the Windows application theme or system language at startup. Languages are English (`en`) and Simplified Chinese (`zh_CN`); unsupported system languages fall back to English. Values are case-insensitive. Either option can be omitted. The file name stays `UWF.ini` even when the executable includes a version number; ensure that the file is not accidentally named `UWF.ini.txt`.

Configuration is read once when a new UI instance starts, including tray-only startup. It is resolved relative to the executable, not the working directory. Restart the UI process to load edits; launching another copy while an instance is running does not reload its configuration. Toolbar theme and language changes only affect the current session and never write back to the file.

A missing file or option keeps the default behavior. Invalid options are ignored with a warning in the in-app log, without affecting other valid options; unreadable or malformed files fall back to defaults without a dialog or blocking startup. The application never creates, updates, or repairs `UWF.ini`, and does not save these preferences in the registry or another directory. Remove the file to restore the defaults on the next startup. File staging and enhanced-mode service configuration remain separate features.

## Build

- C++20
- CMake ≥ 3.16
- Qt 6 (Widgets, Svg, LinguistTools)
- A Windows-targeting compiler (MSVC, clang-cl, MSYS2 clang64, mingw-w64)

```sh
cmake -S . -B build
cmake --build build --config Release
```

## AI-generated code

Most of the source in this repository was produced by AI coding assistants under human review and direction. Treat the code accordingly: it is functional but you should verify any non-obvious behavior against the source, the WMI documentation, or the running system before relying on it.

## License

GNU General Public License v3.0 — see [LICENSE](LICENSE).
