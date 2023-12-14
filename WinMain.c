#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <wtsapi32.h>
#include <Psapi.h>
#include <wchar.h>

BOOL FindDiscordExe(const wchar_t* folderPattern, wchar_t* resultPath) {
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(folderPattern, &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            wcscmp(findData.cFileName, L".") != 0 &&
            wcscmp(findData.cFileName, L"..") != 0) {
            _snwprintf(resultPath, MAX_PATH, L"%s\\discord.exe", findData.cFileName);
            FindClose(hFind);
            return TRUE;
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);
    return FALSE;
}

void SuspendDiscordThread(const wchar_t* discordPath) {
    WTS_PROCESS_INFOW* pProcessInfo = NULL;
    DWORD dwCount = 0;
    BOOL bSuspended = FALSE;

    ShellExecuteW(NULL, NULL, discordPath, NULL, NULL, SW_SHOWNORMAL);

    while (!bSuspended && !SleepEx(1, TRUE)) {
        WTSEnumerateProcessesW(WTS_CURRENT_SERVER, 0, 1, &pProcessInfo, &dwCount);

        for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++) {
            if (_wcsicmp(discordPath, _wcslwr(pProcessInfo[dwIndex].pProcessName)) == 0)
                continue;

            HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            THREADENTRY32 te = { sizeof(THREADENTRY32) };

            if (Thread32First(hThreadSnapshot, &te)) {
                do {
                    if (te.th32OwnerProcessID != pProcessInfo[dwIndex].ProcessId)
                        continue;

                    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                    DWORD64 dw64StartAddress = 0;
                    NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dw64StartAddress, sizeof(DWORD64), NULL);

                    HANDLE hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pProcessInfo[dwIndex].ProcessId);
                    MODULEENTRY32W me = { sizeof(MODULEENTRY32W) };

                    if (Module32FirstW(hModuleSnapshot, &me)) {
                        do {
                            if (dw64StartAddress >= (DWORD64)me.modBaseAddr &&
                                dw64StartAddress <= ((DWORD64)me.modBaseAddr + me.modBaseSize) &&
                                !wcscmp(me.szModule, L"discord_utils.node"))
                                bSuspended = SuspendThread(hThread);
                        } while (Module32NextW(hModuleSnapshot, &me));
                    }

                    CloseHandle(hModuleSnapshot);
                    CloseHandle(hThread);
                } while (Thread32Next(hThreadSnapshot, &te));
            }

            CloseHandle(hThreadSnapshot);
        }

        WTSFreeMemory(pProcessInfo);
    }
}

int main() {
    wchar_t discordPath[MAX_PATH];

    if (FindDiscordExe(L"app-*", discordPath)) {
        SuspendDiscordThread(discordPath);
    }

    return 0;
}
