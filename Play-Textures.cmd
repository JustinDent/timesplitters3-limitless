@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\play-textures.ps1"
if errorlevel 1 pause
