# Fullscreen Timer

Offline stopwatch and countdown timer for Windows, Linux, and macOS. Controls disappear while the timer is running; only the selected digits blink when editing.

## Downloads

| Platform | File | Size |
| --- | --- | --- |
| Windows | [EXE](FullscreenTimer-Windows.exe) | 16 KB |
| Linux x86_64 | [AppImage](FullscreenTimer-Linux-x86_64.AppImage) or [TAR.GZ](FullscreenTimer-Linux-x86_64.tar.gz) | 935 KB / 8 KB |
| macOS — Intel and Apple Silicon | [ZIP](FullscreenTimer-macOS.zip) | 6 KB |

**Windows:** Open the EXE.  
**Linux:** Mark the AppImage as executable, or extract the TAR.GZ and run `FullscreenTimer`.  
**macOS:** Extract the ZIP and move the app to Applications.

## Requirements

The app does not download anything during use and requires no internet connection.

- Windows 10/11 with the system .NET Framework/WPF.
- Linux: GTK 3, Cairo, and glibc 2.31+. The AppImage requires FUSE; the TAR.GZ does not.
- macOS 11+: system Cocoa, WebKit, and JavaScript for Automation. The app has not yet been tested on a Mac and is not notarized.

## Controls

| Key | Action |
| --- | --- |
| Space | Start / pause |
| S | Switch between stopwatch and countdown |
| R | Reset |
| ← / → | Select minutes / seconds |
| ↑ / ↓ | Adjust the time |
| Enter | Toggle digit selection |
| F or double-click the timer | Toggle fullscreen |

## Building

Source files are in `src/`, and build scripts are in `tools/`. Building requires Python 3, plus a C compiler on Linux or the .NET Framework compiler on Windows.

```sh
python tools/build.py windows
python3 tools/build.py linux
python3 tools/build.py macos
```

Build output is saved to `build/`. The optional AppImage wrapper requires a local copy of the official [type2 runtime](https://github.com/AppImage/type2-runtime) and `mksquashfs`:

```sh
python3 tools/build_appimage.py --runtime /path/to/runtime-x86_64 --binary build/FullscreenTimer --output build/FullscreenTimer-Linux-x86_64.AppImage
```

## License

MIT; see `LICENSE`. The interface is based on [alphakevin/fullscreen-timer](https://github.com/alphakevin/fullscreen-timer). AppImage runtime licensing information is included in `THIRD_PARTY_LICENSES.txt`. Package checksums are available in `SHA256SUMS.txt`.
