#include "stdafx.h"
#include "Log.h"
#include <stdio.h>
#include <stdarg.h>

using std::string;

extern HMODULE thisModule;
extern const char* logFile;
extern HANDLE hFile;

void Log(std::string format, ...) 
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	format += "\r\n";
	vsnprintf(buffer, sizeof(buffer), format.c_str(), args);
	va_end(args);
	if (!IsDebuggerPresent())
	{
		WriteFile(hFile, buffer, strlen(buffer), NULL, NULL);
	}
	else
	{
		OutputDebugString(buffer);
	}
}
