#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include <exception.hpp>

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
    wchar_t     modulename[MAX_PATH];
    wchar_t     filename[MAX_PATH];
    wchar_t     timestamp[128];
    wchar_t*    modulenameptr{};
    bool        bDumpSuccess;
    __time64_t  time;
    struct tm   ltime;
    HWND        hWnd;
    HANDLE      hFile;

    // Write minidump
    if (GetModuleFileNameW(GetModuleHandle(NULL), modulename, _countof(modulename)) != 0)
    {
        modulenameptr = wcsrchr(modulename, '\\');
        *modulenameptr = L'\0';
        modulenameptr += 1;
    }
    else
    {
        wcscpy(modulenameptr, L"err.err");
    }

    _time64(&time);
    _localtime64_s(&ltime, &time);
    wcsftime(timestamp, _countof(timestamp), L"%Y%m%d%H%M%S", &ltime);
    swprintf_s(filename, L"%s\\%s\\%s.%s.dmp", modulename, L"CrashDumps", modulenameptr, timestamp);

    hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION ex;
        memset(&ex, 0, sizeof(ex));
        ex.ThreadId = GetCurrentThreadId();
        ex.ExceptionPointers = ExceptionInfo;
        ex.ClientPointers = TRUE;

        if (!(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, &ex, NULL, NULL)))
            bDumpSuccess = false;

        CloseHandle(hFile);
    }

    // Logs exception into buffer and writes to file
    swprintf_s(filename, L"%s\\%s\\%s.%s.log", modulename, L"CrashDumps", modulenameptr, timestamp);
    hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        auto Log = [ExceptionInfo, hFile](char* buffer, size_t size, bool reg, bool stack, bool trace)
        {
            if (LogException(buffer, size, (LPEXCEPTION_POINTERS)ExceptionInfo, reg, stack, trace))
            {
                // Add some info of our own. Only game version for now.
                std::stringstream re4ti;
                re4ti << "bio4.exe version: " << GameVersion() << std::endl;
                strcat(buffer, re4ti.str().data());

                // Write log file
                DWORD NumberOfBytesWritten = 0;
                WriteFile(hFile, buffer, strlen(buffer), &NumberOfBytesWritten, NULL);
            }
        };

        // Try to make a very descriptive exception, for that we need to malloc a huge buffer...
        if (auto buffer = (char*)malloc(max_logsize_ever))
        {
            Log(buffer, max_logsize_ever, true, true, true);
            free(buffer);
        }
        else
        {
            // Use a static buffer, no need for any allocation
            static const auto size = max_logsize_basic + max_logsize_regs + max_logsize_stackdump;
            static char static_buf[size];
            static_assert(size <= max_static_buffer, "Static buffer is too big");

            Log(buffer = static_buf, sizeof(static_buf), true, true, false);
        }
        CloseHandle(hFile);
    }

    // Exit the application
    ShowCursor(TRUE);
    hWnd = FindWindowW(0, L"");
    SetForegroundWindow(hWnd);

    return EXCEPTION_CONTINUE_SEARCH;
}

void Init_ExceptionHandler()
{
    std::string dumpPath = rootPath + "CrashDumps";

    auto FolderExists = [](LPCSTR szPath) -> BOOL
    {
        DWORD dwAttrib = GetFileAttributesA(szPath);
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    };

    if (FolderExists(dumpPath.c_str()))
    {
        SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

        // Now stub out SetUnhandledExceptionFilter so NO ONE ELSE can set it!
        uint32_t ret = 0x900004C2; //ret4
        DWORD protect[2];
        VirtualProtect(&SetUnhandledExceptionFilter, sizeof(ret), PAGE_EXECUTE_READWRITE, &protect[0]);
        memcpy(&SetUnhandledExceptionFilter, &ret, sizeof(ret));
        VirtualProtect(&SetUnhandledExceptionFilter, sizeof(ret), protect[0], &protect[1]);
    }
}