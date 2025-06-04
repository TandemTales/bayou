@echo off
echo Starting BayouBonanza Debug Server...
echo.

if not exist build\Debug\BayouBonanzaServer.exe (
    echo ERROR: BayouBonanzaServer.exe not found!
    echo Please run build_debug.bat first to build the server.
    pause
    exit /b 1
)

echo Server executable found. Starting server...
echo Press Ctrl+C to stop the server.
echo.

build\Debug\BayouBonanzaServer.exe

echo.
echo Server has stopped.
pause 