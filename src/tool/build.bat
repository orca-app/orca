@echo off
setlocal enabledelayedexpansion

for /f "delims=" %%v in ('git rev-parse --short HEAD') do set githash=%%v

clang ^
    -std=c11 ^
    -I.. ^
    -DFLAG_IMPLEMENTATION ^
    -DORCA_TOOL_VERSION=!githash! ^
    -MJbuild\main.json ^
    -o build\orca.exe ^
    main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

echo [ > build\compile_commands.json
type build\main.json >> build\compile_commands.json
echo ] >> build\compile_commands.json
