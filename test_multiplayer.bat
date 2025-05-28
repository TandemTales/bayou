@echo off
echo Starting Bayou Bonanza Multiplayer Test
echo =======================================

echo Starting server...
start "Bayou Server" ".\build\Debug\BayouBonanzaServer.exe"

echo Waiting 2 seconds for server to start...
timeout /t 2 /nobreak >nul

echo Starting Player 1 client...
start "Player 1" ".\build\Debug\BayouBonanzaClient.exe"

echo Waiting 1 second...
timeout /t 1 /nobreak >nul

echo Starting Player 2 client...
start "Player 2" ".\build\Debug\BayouBonanzaClient.exe"

echo.
echo All components started!
echo - Server: Manages game logic and state
echo - Player 1: First client (Player One)
echo - Player 2: Second client (Player Two)
echo.
echo The game should start automatically when both clients connect.
echo Player 1 (blue pieces) moves first.
echo.
pause 