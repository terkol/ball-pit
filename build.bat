@echo off
setlocal
cd /d C:\dev\ball-pit

if not exist build mkdir build

call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64

cl /nologo /std:c++20 /W4 /EHsc /MD^
  /DGLFW_STATIC ^
  /I external\glfw\include ^
  /Fe:build\ball-pit.exe /Fo:build\ /Fd:build\ball_pit.pdb ^
  src\main.cpp ^
  /link /LIBPATH:external\glfw\lib-vc2022 ^
  glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib ^
  advapi32.lib ole32.lib


if errorlevel 1 (
  echo Build failed.
  pause
  exit /b 1
)

copy /Y external\glfw\lib-vc2022\glfw3.dll build\ >nul

build\ball-pit.exe