#include <Windows.h>
#include <shlwapi.h>
#include <tlhelp32.h>
#include <winternl.h>

static LPCWSTR szFileName = NULL;

void WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild,
                  DWORD idEventThread, DWORD dwmsEventTime)
{
    WCHAR lpExeName[MAX_PATH] = {0};
    DWORD dwProcessId = 0;
    WCHAR ptszClassName[256] = {0};

    if (!RealGetWindowClassW(hwnd, ptszClassName, _countof(ptszClassName)))
        return;

    GetWindowThreadProcessId(hwnd, &dwProcessId);
    if (!dwProcessId)
        return;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess)
        return;

    DWORD size = _countof(lpExeName);
    if (!QueryFullProcessImageNameW(hProcess, 0, lpExeName, &size))
    {
        CloseHandle(hProcess);
        return;
    }
    CloseHandle(hProcess);

    _wcslwr_s(lpExeName, _countof(lpExeName));
    PathStripPathW(lpExeName);

    if (wcscmp(lpExeName, szFileName) == 0 && wcscmp(ptszClassName, L"raw_input") == 0)
    {
        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnapshot == INVALID_HANDLE_VALUE)
            return;

        THREADENTRY32 te = { sizeof(te) };
        BOOL firstThreadFound = FALSE;

        if (Thread32First(hThreadSnapshot, &te))
        {
            do
            {
                if (te.th32OwnerProcessID != dwProcessId)
                    continue;

                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (!hThread)
                    continue;

                DWORD64 ThreadInformation = 0;
                if (NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &ThreadInformation, sizeof(ThreadInformation), NULL) != 0)
                {
                    CloseHandle(hThread);
                    continue;
                }

                HANDLE hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessId);
                if (hModuleSnapshot == INVALID_HANDLE_VALUE)
                {
                    CloseHandle(hThread);
                    continue;
                }

                MODULEENTRY32W me = { sizeof(me) };
                BOOL inDiscordModule = FALSE;
                if (Module32FirstW(hModuleSnapshot, &me))
                {
                    do
                    {
                        if (ThreadInformation >= (DWORD64)me.modBaseAddr &&
                            ThreadInformation <= (DWORD64)me.modBaseAddr + me.modBaseSize &&
                            wcscmp(me.szModule, L"discord_utils.node") == 0)
                        {
                            inDiscordModule = TRUE;
                            break;
                        }
                    } while (Module32NextW(hModuleSnapshot, &me));
                }

                CloseHandle(hModuleSnapshot);

                if (inDiscordModule)
                {
                    if (firstThreadFound)
                        SuspendThread(hThread);
                    else
                        firstThreadFound = TRUE;
                }

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
    WCHAR lpExeName[MAX_PATH] = {0};

    GetWindowThreadProcessId(hWnd, &dwProcessId);
    if (!dwProcessId)
        return TRUE;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess)
        return TRUE;

    DWORD size = _countof(lpExeName);
    if (!QueryFullProcessImageNameW(hProcess, 0, lpExeName, &size))
    {
        CloseHandle(hProcess);
        return TRUE;
    }
    CloseHandle(hProcess);

    _wcslwr_s(lpExeName, _countof(lpExeName));
    PathStripPathW(lpExeName);

    if (wcscmp(lpExeName, szFileName) == 0)
        EndTask(hWnd, FALSE, TRUE);

    return TRUE;
}

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    CONST LPCWSTR aProcesses[] = {L"discord.exe", L"discordptb.exe", L"discordcanary.exe"};
    WCHAR lpPathName[MAX_PATH] = {0};
    HANDLE hProcess = GetCurrentProcess();

    DWORD size = _countof(lpPathName);
    if (!QueryFullProcessImageNameW(hProcess, 0, lpPathName, &size))
        return 1;

    for (DWORD dwIndex = size; dwIndex > 0; dwIndex--)
    {
        if (lpPathName[dwIndex] == L'\\')
        {
            lpPathName[dwIndex] = L'\0';
            SetCurrentDirectoryW(lpPathName);
            lpPathName[dwIndex] = L'\\';
            lpPathName[dwIndex + 1] = L'*';
            lpPathName[dwIndex + 2] = L'\0';
            break;
        }
    }

    WIN32_FIND_DATAW FindFileData = {0};
    HANDLE hFindFile = FindFirstFileExW(lpPathName, FindExInfoBasic, &FindFileData, FindExSearchLimitToDirectories,
                                        NULL, FIND_FIRST_EX_LARGE_FETCH);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (PathIsDirectoryW(FindFileData.cFileName) &&
                wcscmp(FindFileData.cFileName, L".") != 0 &&
                wcscmp(FindFileData.cFileName, L"..") != 0)
            {
                if (SetCurrentDirectoryW(FindFileData.cFileName))
                {
                    for (int iIndex = 0; iIndex < 3; iIndex++)
                    {
                        if (PathFileExistsW(aProcesses[iIndex]))
                        {
                            szFileName = aProcesses[iIndex];
                            EnumWindows(EnumWindowsProc, 0);
                            ShellExecuteW(NULL, NULL, szFileName, NULL, NULL, SW_SHOWNORMAL);
                            SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

                            MSG msg;
                            while (GetMessageW(&msg, NULL, 0, 0))
                                DispatchMessageW(&msg);

                            break;
                        }
                    }
                    SetCurrentDirectoryW(L"..");
                }
            }
        } while (FindNextFileW(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }

    return 0;
}
