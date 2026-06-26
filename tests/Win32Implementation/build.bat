@echo off
REM Builds the Win32 + OpenGL engine implementation (ECS + graphics).
REM Requires GLEW installed for the active MinGW/MSYS2 toolchain.
REM Uses native Win32/WGL for the window and context, so no GLFW is needed.

set SRC=..\..\src\engine\core

echo === Building Win32 engine implementation ===
g++ -std=gnu++17 -mwindows -DUNICODE -D_UNICODE -g main.cpp ^
  %SRC%\graphics\Renderer.cpp ^
  %SRC%\graphics\Shader.cpp ^
  %SRC%\graphics\Mesh.cpp ^
  %SRC%\graphics\Camera.cpp ^
  %SRC%\math\vector.cpp ^
  %SRC%\math\Mat4.cpp ^
  %SRC%\math\quaternion.cpp ^
  %SRC%\math\transform.cpp ^
  %SRC%\ecs\World.cpp ^
  %SRC%\ecs\Component.cpp ^
  %SRC%\ecs\Entity.cpp ^
  %SRC%\ecs\SystemManager.cpp ^
  -lglew32 -lopengl32 -lgdi32 ^
  -o win32_engine.exe
if %errorlevel% == 0 (
  echo Build successful. Running...
  echo.
  win32_engine.exe
) else (
  echo Build failed!
)

echo.
PAUSE
