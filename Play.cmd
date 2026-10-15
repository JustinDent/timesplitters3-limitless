@echo off
start "" "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -STA -WindowStyle Hidden -ExecutionPolicy Bypass -File "%~dp0tools\start-game.ps1" -Script "%~dp0tsfp.ps1" -Run
if errorlevel 1 pause
