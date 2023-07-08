@echo off
windres icon.rc -O coff -o icon.res
gcc.exe -s -mwindows WinMain.c -lpathcch -lntdll -lwtsapi32 -lshlwapi icon.res -o DiscordFixer.exe
upx --best --ultra-brute DiscordFixer.exe>nul 2>&1