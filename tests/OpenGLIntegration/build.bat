@echo off
echo Compiling Win32 Hello World...

g++ -mwindows helloGL.cpp -o hello2.exe -DUNICODE -D_UNICODE -lglew32 -lopengl32 -lgdi32
PAUSE

if %errorlevel% == 0(
  echo Compilation sucessful!
) else (
  echo Compilation failed!
)

echo.
PAUSE