#pragma once

#include <Windows.h>

typedef int (CALLBACK* Callback)(const char* record, void* user);

bool SetHook(Callback callback, void* user);

void ReleaseHook();
