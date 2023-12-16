@echo off
set SOURCE_FILE=WinMain.c
set OUTPUT_FILE=DiscordFixer.exe
set RESOURCE_FILE=icon.rc

rem Compile resource file
windres %RESOURCE_FILE% -O coff -o icon.res

rem Compile source file with optimized GCC flags
gcc.exe -O3 -Wall -s -mwindows %SOURCE_FILE% -lpathcch -lntdll -lwtsapi32 -lshlwapi icon.res -o %OUTPUT_FILE%

rem Compress executable with UPX
upx --best --ultra-brute %OUTPUT_FILE% > nul 2>&1
