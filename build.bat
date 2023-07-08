@echo off
set compiler=%1
if "%1"=="" set compiler=gcc.exe
cd "%~dp0"
"%compiler%" -s -mwindows src/main.c -lwtsapi32 -o ./output/DiscordFixer.exe
mkdir output>nul 2>&1
upx --best --ultra-brute /output/DiscordFixer.exe>nul 2>&1