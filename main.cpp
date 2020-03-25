#include <iostream>
#include <windows.h>
#include <io.h>
#include <tlhelp32.h>

DWORD GetProcessByName(char * process_name) {
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    DWORD proc_id = 0;
    if(Process32First(Snapshot, &process)) {
        while(Process32Next(Snapshot, &process)) {
            if(_stricmp(process.szExeFile, process_name) == 0) {
                proc_id = process.th32ProcessID;
                break;
            }
        }
    }
    CloseHandle(Snapshot);
    return proc_id;
}

bool FileExists(char * name) {
    return _access(name, 0) != -1;
}
bool Inject(DWORD pID, char * path) {
    HANDLE proc_handle;
    LPVOID RemoteString;
    LPVOID LoadLibAddy;

    if(pID == 0) {
        std::cout << "ERROR PID = 0" << std::endl;
        return false;
    }
    proc_handle = OpenProcess(PROCESS_ALL_ACCESS, false, pID);
    if(proc_handle == 0) {
        std::cout << "ERROR PROC HANDLE = 0" << std::endl;
        return false;
    }

    LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    RemoteString = VirtualAllocEx(proc_handle, NULL, strlen(path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(proc_handle, RemoteString, path, strlen(path), NULL);
    CreateRemoteThread(proc_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, RemoteString, NULL, NULL);

    CloseHandle(proc_handle);
    return true;
}
int main() {
    char process_name[32];
    char dll_name[32];
    char path[256];
    std::cout << "Enter process name: " << std::endl;
    std::cin >> process_name;

    DWORD pID = GetProcessByName(process_name);
    std::cout << "Wait for starting process " << std::endl;

    for(; ; Sleep(5000)) {
        if(pID == 0) {
            pID = GetProcessByName(process_name);
        }
        if(pID != 0) {
            std::cout << "Found process with id: " << pID << std::endl;
            break;
        }
    }

    while(FileExists(path) == false) {
        std::cout << "Enter dll name: " << std::endl;
        std::cin >> dll_name;
        GetFullPathName(dll_name, sizeof(path), path, 0);
        if(FileExists(path)) std::cout << "dll found" << std::endl;
        else if(FileExists(path)) std::cout << "dll  not found" << std::endl;
    }

    std::cout << "Preparing dll for injection " << std::endl;
    if(Inject(pID, path)) std::cout << "dll injected " << std::endl;

    system("PAUSE");
}
