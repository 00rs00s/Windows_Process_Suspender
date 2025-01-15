#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <tlhelp32.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <conio.h> // for _getch()
#include <thread>

//Declarations
int FindTarget(const char *procname){
  HANDLE hProcSnap;
  PROCESSENTRY32 pe32;
  int pid = 0;

  hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (INVALID_HANDLE_VALUE == hProcSnap)  return 0;

  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hProcSnap, &pe32)){
    CloseHandle(hProcSnap);
    return 0;
  }

  while (Process32Next(hProcSnap, &pe32)){
    if (lstrcmpiA(procname, pe32.szExeFile) == 0){
      pid = pe32.th32ProcessID;
      break;
    }
  }

  CloseHandle(hProcSnap);
  return pid;  
}

int main(){
  HANDLE hProcessSnap ;
  PROCESSENTRY32 pe32;
  // Set the size of the structure before using it.
  pe32.dwSize = sizeof( PROCESSENTRY32 );
  DWORD dwPriorityClass;
  HANDLE hProcess;
  HANDLE hTargetProcess;
  BOOL general_use_boolean;
  std::string userInput;
  int pid = 0;
  typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
  typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
  int last_err=-1;

  // Prompt the user for input
  std::cout << "Please enter a process to suspend: ";
  std::getline(std::cin, userInput);
  while (TRUE){
    pid = FindTarget(userInput.c_str());
    hTargetProcess = OpenProcess(PROCESS_SUSPEND_RESUME,FALSE,pid);
    if(hTargetProcess == NULL){
        if (last_err != GetLastError()){
            last_err = GetLastError();
            std::cout <<"Last error for "<<userInput<<": "<<last_err<<std::endl;
            printf("Error code (hex): 0x%X\n", last_err);
        }
        //UnComment the line below if you do not have resources problem
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    else{
        std::cout<<"\nHandle of "<<userInput<< " is :"<<hTargetProcess<<std::endl;
        NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtSuspendProcess");
        pfnNtSuspendProcess(hTargetProcess);
        std::cout<<"Process "<<userInput<<" suspended!"<<std::endl;
        break;
    }
  }

  //Wait for user to press a key to continue
  std::cout << "\nPress any key to continue..." << std::endl;
  _getch(); // Waits for a key press

  // Resume the process
  NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandleA("ntdll"), "NtResumeProcess");
  //pfnNtResumeProcess(hTargetProcess);
  //std::cout << "\nProcess " << userInput << " resumed!" << std::endl;
  if (pfnNtResumeProcess) {
        pfnNtResumeProcess(hTargetProcess);
        std::cout << "\nProcess " << userInput << " resumed!" << std::endl;
    } else {
        std::cout << "Failed to get NtResumeProcess function address." << std::endl;
    }

  //Close Handle to target process
  if (CloseHandle( hTargetProcess ) != 0 )
    std::cout<<"\nProcess Handle: 0x"<< hTargetProcess<< " closed sucessfully!"<< std::endl;

  return 0;
}
