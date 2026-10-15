@echo off
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0Assemble.ps1"
if errorlevel 1 goto failed
pause
exit /b 0
:failed
echo Assembly failed. Read the message above.
pause
exit /b 1
