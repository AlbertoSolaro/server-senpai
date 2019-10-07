#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "Timer.h"
#include <iostream>

Timer t = Timer();


PROCESS_INFORMATION startup()
{
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcess(NULL,   // the path
		const_cast<LPSTR>("python mqtt.py"), // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return pi;
}

BOOL TerminateProcessEx(PROCESS_INFORMATION pi)
{
	DWORD dwDesiredAccess = PROCESS_TERMINATE;
	BOOL  bInheritHandle = FALSE;
	HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, pi.dwProcessId);
	if (hProcess == NULL)
		return FALSE;

	BOOL result = TerminateProcess(hProcess, 0);

	CloseHandle(hProcess);

	return result;
}


void f1() {
	std::cout << "Hey.. After each 1s..." << std::endl;
}

void f2() {
	std::cout << "Hey.. After 5.2s. But I will stop the timer!" << std::endl;
	t.stop();
}


int main() {

	t.setInterval(f1, 1000);

	t.setTimeout(f2, 5200);

	auto pi = startup();
	while (true);
	TerminateProcessEx(pi);
}