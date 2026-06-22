@echo off
REM Builds and runs both graphics smoke tests.
REM Requires GLEW and GLFW.
REM GLFW paths match .vscode/tasks.json; adjust if your GLFW lives elsewhere.

set GLFW_INC=K:\glfw-3.4.bin.WIN64\include
set GLFW_LIB=K:\glfw-3.4.bin.WIN64\lib-mingw-w64
set SRC=..\..\src\engine\core

echo === Building CPU smoke test (no GL context needed) ===
g++ -std=gnu++17 -g graphics_cpu_smoke_test.cpp ^
  %SRC%\graphics\Mesh.cpp ^
  %SRC%\math\vector.cpp ^
  -lglew32 -lopengl32 ^
  -o graphics_cpu_smoke_test.exe
if %errorlevel% == 0 (
  echo Build successful. Running...
  echo.
  graphics_cpu_smoke_test.exe
) else (
  echo CPU test build failed!
)

echo.
echo === Building GL integration smoke test (hidden GLFW window) ===
g++ -std=gnu++17 -g graphics_gl_smoke_test.cpp ^
  %SRC%\graphics\Renderer.cpp ^
  %SRC%\graphics\Shader.cpp ^
  %SRC%\graphics\Mesh.cpp ^
  %SRC%\math\vector.cpp ^
  -I "%GLFW_INC%" -L "%GLFW_LIB%" ^
  -lglew32 -lglfw3 -lopengl32 -lgdi32 ^
  -o graphics_gl_smoke_test.exe
if %errorlevel% == 0 (
  echo Build successful. Running...
  echo.
  graphics_gl_smoke_test.exe
) else (
  echo GL test build failed!
)

echo.
PAUSE
