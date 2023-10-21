@echo off

clang ^
    -std=c11 ^
    -I.. ^
    -DFLAG_IMPLEMENTATION ^
    -MJbuild\main.json ^
    -o build\orca.exe ^
    main.c
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%

echo [ > build\compile_commands.json
type build\main.json >> build\compile_commands.json
echo ] >> build\compile_commands.json
