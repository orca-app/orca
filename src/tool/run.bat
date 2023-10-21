@echo off

call build.bat
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

build\orca.exe -h
