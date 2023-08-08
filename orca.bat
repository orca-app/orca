@echo off

rem Get the directory of this batch script
set "script_dir=%~dp0"

python3 "%script_dir%orca" %*
exit /b %errorlevel%
