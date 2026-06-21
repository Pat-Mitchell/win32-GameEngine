@echo off
echo Building ECS smoke test...
g++ -std=gnu++17 -g ecs_smoke_test.cpp ^
  ..\..\src\engine\core\ecs\World.cpp ^
  ..\..\src\engine\core\ecs\Entity.cpp ^
  ..\..\src\engine\core\ecs\Component.cpp ^
  ..\..\src\engine\core\ecs\SystemManager.cpp ^
  -o ecs_smoke_test.exe
if %errorlevel% == 0 (
  echo Build successful. Running...
  echo.
  ecs_smoke_test.exe
) else (
  echo Build failed!
)
PAUSE