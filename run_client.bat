@echo off
echo Starting BayouBonanza Debug Client...
echo.

if not exist build\Debug\BayouBonanzaClient.exe (
    echo ERROR: BayouBonanzaClient.exe not found!
    echo Please run build_debug.bat first to build the client.
    pause
    exit /b 1
)

echo Client executable found. Starting client...
echo.

build\Debug\BayouBonanzaClient.exe

echo.
echo Client has stopped.
pause 