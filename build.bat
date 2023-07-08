@echo off
gcc.exe -s -mwindows WinMain.c -lpathcch -lntdll -lwtsapi32 -lshlwapi -o DiscordFixer.exe
upx --best --ultra-brute DiscordFixer.exe>nul 2>&1