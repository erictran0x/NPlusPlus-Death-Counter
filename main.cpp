#include <iostream>
#include <fstream>
#include <string.h>
#include <windows.h>
#include <Tlhelp32.h>

using namespace std;

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | 0x10, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                wchar_t wc[100];
                mbstowcs(wc, modEntry.szModule, 100);
                if (!_wcsicmp(wc, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

int main()
{
    HWND hwnd = FindWindowA(NULL, "NPP");
    if (hwnd == NULL) {
        cout << "Cannot find window." << endl;
        Sleep(1500);
        return 1;
    }

    DWORD procId;
    GetWindowThreadProcessId(hwnd, &procId);
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, procId);
    if (procId == NULL) {
        cout << "Cannot obtain process." << endl;
        Sleep(1500);
        return 1;
    };

    const uintptr_t DLL_BASE_ADDR = GetModuleBaseAddress(procId, L"npp.dll");

    ofstream fout;

    int first_death_count, new_death_count;
    const LPCVOID* DEATH_COUNT_ADDR = (LPCVOID*)(DLL_BASE_ADDR + 0xE91414);

    ReadProcessMemory(handle, DEATH_COUNT_ADDR, &first_death_count, sizeof(int), 0);

    while (IsWindowVisible(hwnd)) {
        int rd;
        ReadProcessMemory(handle, DEATH_COUNT_ADDR, &rd, sizeof(int), 0);
        if (new_death_count != rd) {
            new_death_count = rd;
            cout << new_death_count << endl;

            fout.open("npp_death_count_session.txt");
            fout << (new_death_count - first_death_count);
            fout.close();

            fout.open("npp_death_count_alltime.txt");
            fout << new_death_count;
            fout.close();
        }
        Sleep(1);
    }
    return 0;
}
