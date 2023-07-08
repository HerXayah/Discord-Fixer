#include <windows.h>
#include <tlhelp32.h>
#include <wtsapi32.h>
#include <winternl.h>
#include <stdio.h>

#define printf __builtin_printf

#define  NtQueryInformationThread ((NTSTATUS (NTAPI*)(HANDLE, THREADINFOCLASS, PVOID, ULONG, PULONG))GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread"))

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nShowCmd)
{
    BOOL debugMode = wcsstr(GetCommandLineW(), L"-debug") != NULL;

    if(debugMode) {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        freopen("CON", "w", stdout);
        freopen ("CON", "r", stdin);
    }

    HANDLE hThreadSnapshot, hModuleSnapshot, hThread, hProcess = 0;
    THREADENTRY32 te = {.dwSize = sizeof(THREADENTRY32)};
    MODULEENTRY32W me = {.dwSize = sizeof(MODULEENTRY32W)};
    PWTS_PROCESS_INFOW pProcessInfo = 0;
    DWORD dwCount = 0;
    DWORD64 dw64StartAddress = 0;

    // Array of the executables names
    LPCWSTR exes[] = { L"discord.exe", L"discordcanary.exe", L"discordptb.exe" };
    int exesCount = sizeof(exes) / sizeof(LPCWSTR);

    if (!WTSEnumerateProcessesW(WTS_CURRENT_SERVER, 0, 1, &pProcessInfo, &dwCount))
        return 1;

    for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        for(int i = 0; i < exesCount; i++)
        {
            if (wcscmp(exes[i], _wcslwr(pProcessInfo[dwIndex].pProcessName)) != 0)
                continue;
            if(debugMode) {
                printf("Process Name: %ls | Process ID: %ld\n", pProcessInfo[dwIndex].pProcessName, pProcessInfo[dwIndex].ProcessId);
            }
            if (Thread32First((hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)), &te))
            {
                OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                do
                {
                    if (te.th32OwnerProcessID != pProcessInfo[dwIndex].ProcessId)
                        continue;
                    hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                    NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dw64StartAddress, sizeof(DWORD64), NULL);
                    if (Module32FirstW((hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, pProcessInfo[dwIndex].ProcessId)), &me))
                    {
                        do
                        {
                            if (dw64StartAddress >= (DWORD64)me.modBaseAddr &&
                                dw64StartAddress <= ((DWORD64)me.modBaseAddr + me.modBaseSize))
                            {
                                if(debugMode) {
                                    printf("Thread ID: %ld | Module: %ls | Module Match: %ls\n", te.th32ThreadID, me.szModule, !wcscmp(me.szModule, L"discord_utils.node") ? L"True" : L"False");
                                }
                                if (!wcscmp(me.szModule, L"discord_utils.node"))
                                    SuspendThread(hThread);
                                break;
                            }
                        } while (Module32NextW(hModuleSnapshot, &me));
                    }
                    CloseHandle(hModuleSnapshot);
                    CloseHandle(hThread);
                } while (Thread32Next(hThreadSnapshot, &te));
            }
            if(debugMode) {
                printf("Done!\n");
            }
            CloseHandle(hThreadSnapshot);
        }
        CloseHandle(hProcess);
    }
    if(debugMode) {
        AllocConsole();
        AttachConsole(GetCurrentProcessId());
        freopen("CON", "w", stdout);
        freopen ("CON", "r", stdin);
    }
    if(debugMode) {
        printf("Press any key to close the console...");
        getchar();
    }

    return 0;
}