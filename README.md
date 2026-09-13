# Parts Inventory Native

This directory contains the native C++/Qt rebuild. The legacy React/Tauri application remains in the parent directory during the migration.

## Requirements

- C++20 compiler
- CMake 3.24 or newer
- Qt 6 Widgets and Network
- Ninja (recommended)

## Linux build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/parts-inventory
```

Create a Debian package:

```bash
cmake --build build --target package
```

The same CMake package target produces an NSIS Windows installer when configured with a Qt 6 Windows x64 toolchain. The standalone executable is available directly in the build directory on each platform; Windows builds produce `parts-inventory.exe`.

For a local Windows build, install Qt 6.8.x with the `MSVC 2022 64-bit` kit, then run:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
cpack --config build/CPackConfig.cmake -C Release -G NSIS
```

The repository also includes a GitHub Actions workflow at `.github/workflows/windows-native.yml` that builds the x64 executable and NSIS installer automatically.

## Supported Windows target

The native Windows build targets Windows 10 and newer, x64 only. Windows XP, Vista, 7, 8, 8.1, and 32-bit Windows are intentionally unsupported. The CMake configuration rejects a 32-bit Windows toolchain.

The initial native release line starts at `1.0.0`.
