@echo off
echo ============================================
echo   EMAIL SYSTEM WITH MALICIOUS SNIFFER
echo   Man-in-the-Middle Attack Demonstration
echo ============================================
echo.

echo [Step 1] Cleaning previous build...
cd src
make clean
cd ..
echo.

echo [Step 2] Rebuilding project with sniffer...
cd src
make
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    echo Check the error messages above.
    cd ..
    pause
    exit /b 1
)
cd ..
echo.

echo [Step 3] Running simulation...
echo Watch for the sniffer attack messages!
echo.
cd simulations
..\src\project.exe -u Cmdenv -c General
cd ..
echo.

echo [Step 4] Analyzing intercepted traffic...
python analyze_traffic.py
echo.

echo ============================================
echo   Attack demonstration complete!
echo   Check intercepted_traffic.log for details
echo ============================================
pause
