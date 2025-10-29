# Quick Build and Run Script for Attack Demo

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  EMAIL SYSTEM WITH MALICIOUS SNIFFER" -ForegroundColor Red
Write-Host "  Man-in-the-Middle Attack Demonstration" -ForegroundColor Red
Write-Host "============================================`n" -ForegroundColor Cyan

# Step 1: Clean
Write-Host "[Step 1] Cleaning previous build..." -ForegroundColor Yellow
Set-Location src
& make clean
Set-Location ..

# Step 2: Build
Write-Host "`n[Step 2] Rebuilding project with sniffer..." -ForegroundColor Yellow
Set-Location src
& make
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nERROR: Build failed!" -ForegroundColor Red
    Write-Host "Check the error messages above." -ForegroundColor Red
    Set-Location ..
    Read-Host "Press Enter to exit"
    exit 1
}
Set-Location ..

# Step 3: Run simulation
Write-Host "`n[Step 3] Running simulation..." -ForegroundColor Yellow
Write-Host "Watch for the sniffer attack messages!`n" -ForegroundColor Green
Set-Location simulations
& ..\src\project.exe -u Cmdenv -c General
Set-Location ..

# Step 4: Analyze
Write-Host "`n[Step 4] Analyzing intercepted traffic..." -ForegroundColor Yellow
if (Test-Path "intercepted_traffic.log") {
    python analyze_traffic.py
} else {
    Write-Host "Warning: intercepted_traffic.log not found!" -ForegroundColor Red
    Write-Host "The sniffer may not have intercepted any traffic." -ForegroundColor Red
}

Write-Host "`n============================================" -ForegroundColor Cyan
Write-Host "  Attack demonstration complete!" -ForegroundColor Green
Write-Host "  Check intercepted_traffic.log for details" -ForegroundColor Green
Write-Host "============================================`n" -ForegroundColor Cyan

Read-Host "Press Enter to exit"
