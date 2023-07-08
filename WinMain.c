#include <windows.h>
#include <pathcch.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <wtsapi32.h>
#include <shlwapi.h>
#define printf __builtin_printf

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    INT iNumArgs = 0;
    WCHAR **pszArgvW = CommandLineToArgvW(GetCommandLineW(), &iNumArgs),
          *szFileName = 0;
    THREADENTRY32 te = {.dwSize = sizeof(THREADENTRY32)};
    MODULEENTRY32W me = {.dwSize = sizeof(MODULEENTRY32W)};
    HANDLE hThreadSnapshot = 0, hModuleSnapshot = 0, hThread = 0;
    DWORD64 dw64StartAddress = 0;
    PWTS_PROCESS_INFOW pProcessInfo = 0;
    DWORD dwCount = 0;
    BOOL bSuspended = FALSE;

    PathCchRemoveFileSpec(pszArgvW[0], wcslen(pszArgvW[0]) + 1);
    SetCurrentDirectoryW(pszArgvW[0]);
    LocalFree(pszArgvW);

    if ((szFileName = PathFileExistsW(L"discord.exe")
                          ? L"discord.exe"
                      : PathFileExistsW(L"discordcanary.exe")
                          ? L"discordcanary.exe"
                      : PathFileExistsW(L"discordptb.exe")
                          ? L"discordptb.exe"
                          : 0))
    {
        ShellExecuteW(NULL, NULL, szFileName, NULL, NULL, SW_SHOWNORMAL);
        while (!bSuspended && !SleepEx(1, TRUE))
        {
            WTSEnumerateProcessesW(WTS_CURRENT_SERVER, 0, 1, &pProcessInfo, &dwCount);
            for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                if (wcscmp(szFileName, _wcslwr(pProcessInfo[dwIndex].pProcessName)))
                    continue;
                if (Thread32First((hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)), &te))
                {
                    do
                    {
                        if (te.th32OwnerProcessID != pProcessInfo[dwIndex].ProcessId)
                            continue;
                        hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                        NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dw64StartAddress, sizeof(DWORD64), NULL);
                        if (Module32FirstW((hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pProcessInfo[dwIndex].ProcessId)), &me))
                            do
                            {
                                if (dw64StartAddress >= (DWORD64)me.modBaseAddr &&
                                    dw64StartAddress <= ((DWORD64)me.modBaseAddr + me.modBaseSize) &&
                                    !wcscmp(me.szModule, L"discord_utils.node"))
                                    bSuspended = SuspendThread(hThread);
                            } while (Module32NextW(hModuleSnapshot, &me));
                        CloseHandle(hModuleSnapshot);
                        CloseHandle(hThread);
                    } while (Thread32Next(hThreadSnapshot, &te));
                }
                CloseHandle(hThreadSnapshot);
            }
            WTSFreeMemory(pProcessInfo);
        }
    }
    return 0;
}