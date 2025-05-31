@echo off
echo Generating CMake project files...
cmake -B build -S .
if %errorlevel% neq 0 (
    echo ERROR: CMake generation failed!
    pause
    exit /b 1
)
echo CMake project files generated successfully in build/ directory
pause 