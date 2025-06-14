@echo off
setlocal enabledelayedexpansion

set "SOURCE_FILE=WinMain.c"
set "OUTPUT_FILE=DiscordFixer.exe"
set "RESOURCE_FILE=icon.rc"
set "RESOURCE_OBJ=icon.res"

rem Compile resource file
echo Compiling resource...
windres "%RESOURCE_FILE%" -O coff -o "%RESOURCE_OBJ%"
if errorlevel 1 (
    echo Failed to compile resource file
    exit /b 1
)

rem Compile source file with optimization and libraries
echo Compiling source...
gcc.exe -O3 -Wall -s -mwindows "%SOURCE_FILE%" -lpathcch -lntdll -lwtsapi32 -lshlwapi "%RESOURCE_OBJ%" -o "%OUTPUT_FILE%"
if errorlevel 1 (
    echo Compilation failed
    exit /b 1
)

rem Compress executable with UPX
if defined UPX (
    echo Compressing executable with UPX...
    upx --best --ultra-brute "%OUTPUT_FILE%" >nul 2>&1
    if errorlevel 1 (
        echo UPX compression failed
        exit /b 1
    )
)

echo Build finished successfully.
endlocal
