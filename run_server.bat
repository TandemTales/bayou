@echo off
echo Starting BayouBonanza Debug Server...
echo.

cd build\Debug
if not exist BayouBonanzaServer.exe (
    echo ERROR: BayouBonanzaServer.exe not found!
    echo Please run build_debug.bat first to build the server.
    cd ..\..
    pause
    exit /b 1
)

echo Server executable found. Starting server...
echo Press Ctrl+C to stop the server.
echo.

BayouBonanzaServer.exe

echo.
echo Server has stopped.
cd ..\..
pause 