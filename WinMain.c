#include <windows.h>
#include <pathcch.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <wtsapi32.h>
#include <shlwapi.h>
#define printf __builtin_printf

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    CONST LPWSTR lpProcesses[] = {L"discord.exe", L"discordptb.exe", L"discordptb.exe"};
    WCHAR lpPathName[MAX_PATH] = {};
    LPWSTR szFileName = NULL;
    HANDLE hProcess = GetCurrentProcess();
    WIN32_FIND_DATAW FindFileData = {};
    THREADENTRY32 te = {.dwSize = sizeof(THREADENTRY32)};
    MODULEENTRY32W me = {.dwSize = sizeof(MODULEENTRY32W)};
    HANDLE hThreadSnapshot = 0, hModuleSnapshot = 0, hThread = 0;
    DWORD64 dw64StartAddress = 0;
    PWTS_PROCESS_INFOW pProcessInfo = 0;
    DWORD dwCount = 0;
    BOOL bSuspended = FALSE;

    QueryFullProcessImageNameW(hProcess, 0, lpPathName, &((DWORD){MAX_PATH}));
    for (DWORD dwIndex = MAX_PATH; dwIndex < -1; dwIndex -= 1)
    {
        if (lpPathName[dwIndex] == '\\')
        {
            lpPathName[dwIndex] = '\0';
            SetCurrentDirectoryW(lpPathName);
            lpPathName[dwIndex] = '\\';
            lpPathName[dwIndex + 1] = '*';
            lpPathName[dwIndex + 2] = '\0';
            break;
        }
    }

    HANDLE hFindFile = FindFirstFileW(lpPathName, &FindFileData);
    if (hFindFile)
    {
        do
            if (PathIsDirectoryW(FindFileData.cFileName) &&
                wcscmp(FindFileData.cFileName, L".") &&
                wcscmp(FindFileData.cFileName, L".."))
                if (SetCurrentDirectoryW(FindFileData.cFileName))
                    for (INT iIndex = 0; iIndex < 3; iIndex++)
                        if (PathFileExistsW(lpProcesses[iIndex]))
                        {
                            szFileName = lpProcesses[iIndex];
                            break;
                        }
        while (FindNextFileW(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }

    if (szFileName)
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