#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#define _CRT_SECURE_NO_WARNINGS

// Windows 头文件
#include <windows.h>
#include <stdio.h>

// 钩子消息回调函数
typedef int (CALLBACK *Callback)(const char* wnd, const char* key, void* user);

//安装钩子
extern "C" __declspec(dllexport) BOOL InstallHook(Callback cb, void* user);

//卸载钩子
extern "C" __declspec(dllexport) BOOL UninstallHook();

//钩子处理函数
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
