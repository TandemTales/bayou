@echo off
echo Building Debug configuration...
echo.

echo Building GameLogic library...
cmake --build build --config Debug --target GameLogic
if %errorlevel% neq 0 (
    echo ERROR: GameLogic build failed!
    pause
    exit /b 1
)

echo Building BayouBonanzaClient...
cmake --build build --config Debug --target BayouBonanzaClient
if %errorlevel% neq 0 (
    echo ERROR: Client build failed!
    pause
    exit /b 1
)

echo Building BayouBonanzaServer...
cmake --build build --config Debug --target BayouBonanzaServer
if %errorlevel% neq 0 (
    echo ERROR: Server build failed!
    pause
    exit /b 1
)

echo Building BayouBonanzaTests...
cmake --build build --config Debug --target BayouBonanzaTests
if %errorlevel% neq 0 (
    echo ERROR: Tests build failed!
    pause
    exit /b 1
)

echo Building BayouBonanzaClientTests...
cmake --build build --config Debug --target BayouBonanzaClientTests
if %errorlevel% neq 0 (
    echo ERROR: Client Tests build failed!
    pause
    exit /b 1
)

echo Building GameplayTest...
cmake --build build --config Debug --target GameplayTest
if %errorlevel% neq 0 (
    echo ERROR: GameplayTest build failed!
    pause
    exit /b 1
)

echo.
echo All Debug builds completed successfully!
pause 