@echo off

python build.py
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

build\orca.exe bundle -h
