@echo off
echo Compiling Win32 Hello World...
g++ -mwindows helloGL.cpp -o hello2.exe -DUNICODE -D_UNICODE -lglfw3 -lgdi32 -lopengl32
if %errorlevel% == 0(
  echo Compilation sucessful!
) else (
  echo Compilation failed!
)
pause