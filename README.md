# Fast Search for Log Files

# Installation:

## Windows
Install SDL2 for Windows, easiest way is using vcpkg.

If you haven't installed vcpkg,
In a directory like `C:\dev\`
```powershell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
From `C:\dev\vcpkg` run
```powershell
.\vcpkg install sdl2:x64-windows
```

Next, from the project directory, 
```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake"
```
Note, DCMAKE_TOOLCHAIN_FILE should point to the location of cmake.

Build your executable
```powershell
cmake --build build --config Release
```

Run it
```powershell
.\build\Release\main.exe
```