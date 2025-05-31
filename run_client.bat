@echo off
echo Starting BayouBonanza Debug Client...
echo.

cd build\Debug
if not exist BayouBonanzaClient.exe (
    echo ERROR: BayouBonanzaClient.exe not found!
    echo Please run build_debug.bat first to build the client.
    cd ..\..
    pause
    exit /b 1
)

echo Client executable found. Starting client...
echo.

BayouBonanzaClient.exe

echo.
echo Client has stopped.
cd ..\..
pause 