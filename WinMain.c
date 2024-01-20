#include <Windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <wtsapi32.h>
static LPWSTR szFileName = NULL;

void WinEventProc(HWINEVENTHOOK hWinEventHook,
                  DWORD event,
                  HWND hwnd,
                  LONG idObject,
                  LONG idChild,
                  DWORD idEventThread,
                  DWORD dwmsEventTime)
{
    WCHAR lpExeName[MAX_PATH] = {};
    DWORD dwProcessId = 0;
    //DWORD dwThreadId = GetWindowThreadProcessId(hwnd, &dwProcessId);
    WCHAR ptszClassName[256] = {};
    HANDLE hThreadSnapshot = NULL;
    THREADENTRY32 te = {.dwSize = sizeof(THREADENTRY32)};

    RealGetWindowClassW(hwnd, ptszClassName, 256);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    QueryFullProcessImageNameW(hProcess, 0, lpExeName, &((DWORD){MAX_PATH}));
    PathStripPathW(_wcslwr(lpExeName));
    CloseHandle(hProcess);

    if (!wcscmp(lpExeName, szFileName) && !wcscmp(ptszClassName, L"raw_input"))
    {
        if (Thread32First((hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0)), &te))
        {
            do
            {
                if (te.th32OwnerProcessID != dwProcessId)
                    continue;
                DWORD64 ThreadInformation = 0;
                MODULEENTRY32W me = {.dwSize = sizeof(MODULEENTRY32W)};
                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID),
                       hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, dwProcessId);
                NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &ThreadInformation, sizeof(DWORD64), NULL);
                if (Module32FirstW((hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32 | TH32CS_SNAPMODULE, dwProcessId)), &me))
                    do
                    {
                        if (ThreadInformation >= (DWORD64)me.modBaseAddr &&
                            ThreadInformation <= ((DWORD64)me.modBaseAddr + me.modBaseSize) &&
                            !wcscmp(me.szModule, L"discord_utils.node"))
                            SuspendThread(hThread);
                    } while (Module32NextW(hModuleSnapshot, &me));
                CloseHandle(hModuleSnapshot);
                CloseHandle(hThread);
            } while (Thread32Next(hThreadSnapshot, &te));
        }
        CloseHandle(hThreadSnapshot);
        PostQuitMessage(0);
    }
}

BOOL EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
    DWORD dwProcessId = 0;
    WCHAR lpExeName[MAX_PATH] = {};
    GetWindowThreadProcessId(hWnd, &dwProcessId);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    QueryFullProcessImageNameW(hProcess, 0, lpExeName, &((DWORD){MAX_PATH}));
    PathStripPathW(_wcslwr(lpExeName));
    CloseHandle(hProcess);
    if (!wcscmp(lpExeName, szFileName))
        EndTask(hWnd, FALSE, TRUE);
    return TRUE;
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    CONST LPWSTR lpProcesses[] = {L"discord.exe", L"discordptb.exe"};
    WCHAR lpPathName[MAX_PATH] = {};
    HANDLE hProcess = GetCurrentProcess();
    WIN32_FIND_DATAW FindFileData = {};
    //PWTS_PROCESS_INFOW pProcessInfo = NULL;
    //DWORD Count = 0;

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

    HANDLE hFindFile = FindFirstFileExW(lpPathName,
                                        FindExInfoBasic,
                                        &FindFileData,
                                        FindExSearchLimitToDirectories,
                                        NULL,
                                        FIND_FIRST_EX_LARGE_FETCH);
    if (hFindFile)
    {
        do
            if (PathIsDirectoryW(FindFileData.cFileName) &&
                wcscmp(FindFileData.cFileName, L".") &&
                wcscmp(FindFileData.cFileName, L".."))
                if (SetCurrentDirectoryW(FindFileData.cFileName))
                    for (INT iIndex = 0; iIndex < (sizeof(lpProcesses)/sizeof(*lpProcesses)); iIndex++)
                        if (PathFileExistsW(lpProcesses[iIndex]))
                        {
                            szFileName = lpProcesses[iIndex];
                            EnumWindows(EnumWindowsProc, 0);
                            ShellExecuteW(NULL, NULL, szFileName, NULL, NULL, SW_SHOWNORMAL);
                            SetWinEventHook(EVENT_OBJECT_CREATE,
                                            EVENT_OBJECT_CREATE,
                                            NULL,
                                            WinEventProc,
                                            0,
                                            0,
                                            WINEVENT_OUTOFCONTEXT);
                            while (GetMessageW(&((MSG){}), NULL, 0, 0));
                            break;
                        }
        while (FindNextFileW(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }

    return 0;
}