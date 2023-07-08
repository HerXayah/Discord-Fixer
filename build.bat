@echo off
set compiler=%1
if "%1"=="" set compiler=gcc.exe
cd "%~dp0"
"%compiler%" -s -mwindows src/main.c -lwtsapi32 -o DiscordFixer.exe
upx --best --ultra-brute DiscordFixer.exe>nul 2>&1