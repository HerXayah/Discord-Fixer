@echo off
set SOURCE_FILE=WinMain.c
set OUTPUT_FILE=DiscordFixer.exe
set RESOURCE_FILE=icon.rc
rem Compile resource file
set resfile=
where windres.exe >nul 2>&1 && (
  windres.exe %RESOURCE_FILE% -O coff -o icon.res
  if exist "icon.res" (
    echo compiled resource file.
    set resfile=icon.res
  )
)
rem Compile source file with optimized GCC flags
where gcc.exe >nul 2>&1 || (
  echo gcc not detected. download it and then add it to your %%PATH%%.
  pause
  exit
)
gcc.exe -O3 -Wall -s -mwindows %SOURCE_FILE% -lpathcch -lntdll -lwtsapi32 -lshlwapi %resfile% -o %OUTPUT_FILE%
if exist "%OUTPUT_FILE%" (
  echo compiled source file.
  rem Compress executable with UPX
  where upx.exe >nul 2>&1 && (
    upx.exe --best --ultra-brute %OUTPUT_FILE% > nul 2>&1
    echo compressed executable with upx.
  )
) else (
  echo error occured during building.
)

if exist "icon.res" (
  del /f /s /q icon.res >nul 2>&1
)

echo done.
timeout /t 3 >nul 2>&1
exit