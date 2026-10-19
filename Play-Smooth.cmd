@echo off
setlocal
set "smooth_fps=144"
if "%~1"=="120" set "smooth_fps=120"
if not "%~1"=="" if not "%~1"=="120" if not "%~1"=="144" (
    echo Usage: Play-Smooth.cmd [120^|144]
    exit /b 1
)
set "RECOMP_FRAME_TARGET=%smooth_fps%"
set "RECOMP_FPS_CAP=60"
set "RECOMP_FULLSCREEN=1"
set "RECOMP_VRR=1"
set "RECOMP_WINDOW_BACKGROUND=0"
set "RECOMP_REFRESH_RATE=%smooth_fps%"
call "%~dp0Play.cmd"
endlocal
