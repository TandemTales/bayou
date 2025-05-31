@echo off
echo Running all tests...
echo.

echo ========================================
echo Running BayouBonanzaTests (Main Tests)
echo ========================================
cd build\tests\Debug
if not exist BayouBonanzaTests.exe (
    echo ERROR: BayouBonanzaTests.exe not found! Run build_debug.bat first.
    cd ..\..\..
    pause
    exit /b 1
)
BayouBonanzaTests.exe
set MAIN_TESTS_RESULT=%errorlevel%
cd ..\..\..

echo.
echo ========================================
echo Running BayouBonanzaClientTests
echo ========================================
cd build\tests\Debug
if not exist BayouBonanzaClientTests.exe (
    echo ERROR: BayouBonanzaClientTests.exe not found! Run build_debug.bat first.
    cd ..\..\..
    pause
    exit /b 1
)
BayouBonanzaClientTests.exe
set CLIENT_TESTS_RESULT=%errorlevel%
cd ..\..\..

echo.
echo ========================================
echo Running GameplayTest
echo ========================================
cd build\Debug
if not exist GameplayTest.exe (
    echo ERROR: GameplayTest.exe not found! Run build_debug.bat first.
    cd ..\..
    pause
    exit /b 1
)
GameplayTest.exe
set GAMEPLAY_TEST_RESULT=%errorlevel%
cd ..\..

echo.
echo ========================================
echo Test Results Summary
echo ========================================
echo BayouBonanzaTests: %MAIN_TESTS_RESULT%
echo BayouBonanzaClientTests: %CLIENT_TESTS_RESULT%
echo GameplayTest: %GAMEPLAY_TEST_RESULT%

if %MAIN_TESTS_RESULT% neq 0 (
    echo FAILED: BayouBonanzaTests
)
if %CLIENT_TESTS_RESULT% neq 0 (
    echo FAILED: BayouBonanzaClientTests
)
if %GAMEPLAY_TEST_RESULT% neq 0 (
    echo FAILED: GameplayTest
)

if %MAIN_TESTS_RESULT% equ 0 if %CLIENT_TESTS_RESULT% equ 0 if %GAMEPLAY_TEST_RESULT% equ 0 (
    echo All tests PASSED!
) else (
    echo Some tests FAILED!
)

pause 