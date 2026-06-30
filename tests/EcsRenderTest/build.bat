@echo off
REM Builds the ECS render-bridge test: entities (Transform + MeshRenderer) drawn
REM by a RenderSystem, a camera entity driven by an InputSystem from Keyboard/Mouse.
REM Native Win32/WGL window + OpenGL/GLEW, no GLFW. Requires GLEW for the toolchain.

set SRC=..\..\src\engine\core

echo === Building ECS render-bridge test ===
g++ -std=gnu++17 -mwindows -DUNICODE -D_UNICODE -g main.cpp ^
  %SRC%\graphics\Renderer.cpp ^
  %SRC%\graphics\Shader.cpp ^
  %SRC%\graphics\Mesh.cpp ^
  %SRC%\graphics\Camera.cpp ^
  %SRC%\loaders\ObjLoader.cpp ^
  %SRC%\input\Keyboard.cpp ^
  %SRC%\input\Mouse.cpp ^
  %SRC%\math\vector.cpp ^
  %SRC%\math\Mat4.cpp ^
  %SRC%\math\quaternion.cpp ^
  %SRC%\math\transform.cpp ^
  %SRC%\ecs\World.cpp ^
  %SRC%\ecs\Component.cpp ^
  %SRC%\ecs\Entity.cpp ^
  %SRC%\ecs\SystemManager.cpp ^
  %SRC%\ecs\RenderSystem.cpp ^
  %SRC%\ecs\InputSystem.cpp ^
  -lglew32 -lopengl32 -lgdi32 ^
  -o ecs_render_test.exe
if %errorlevel% == 0 (
  echo Build successful. Running...
  echo.
  ecs_render_test.exe
) else (
  echo Build failed!
)

echo.
PAUSE
