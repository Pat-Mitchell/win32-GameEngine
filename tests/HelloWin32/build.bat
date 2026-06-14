@echo off
echo Compiling Win32 Hello World...
g++ -mwindows helloWin32.cpp -o hello -DUNICODE -D_UNICODE
if %errorlevel% == 0(
  echo Compilation sucessful!
) else (
  echo Compilation failed!
)
pause