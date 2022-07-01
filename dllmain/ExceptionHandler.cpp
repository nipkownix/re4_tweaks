#include <iostream>
#include "stdafx.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include <exception.hpp>
#include "miniz/miniz.h"

// miniz unfortunately doesn't include wchar versions of its functions, but fortunately does allow passing FILE* to it
mz_bool mz_zip_writer_add_file(mz_zip_archive* pZip, const char* pArchive_name, const wchar_t* pSrc_filename, const void* pComment, mz_uint16 comment_size, mz_uint level_and_flags)
{
  MZ_FILE* pSrc_file = NULL;
  mz_uint64 uncomp_size = 0;
  MZ_TIME_T file_modified_time;
  MZ_TIME_T* pFile_time = NULL;
  mz_bool status;

  memset(&file_modified_time, 0, sizeof(file_modified_time));

  _wfopen_s(&pSrc_file, pSrc_filename, L"rb");
  if (!pSrc_file)
    return false;

  _fseeki64(pSrc_file, 0, SEEK_END);
  uncomp_size = _ftelli64(pSrc_file);
  _fseeki64(pSrc_file, 0, SEEK_SET);

  status = mz_zip_writer_add_cfile(pZip, pArchive_name, pSrc_file, uncomp_size, pFile_time, pComment, comment_size, level_and_flags, NULL, 0, NULL, 0);

  fclose(pSrc_file);

  return status;
}

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
    wchar_t     modulename[MAX_PATH];
    wchar_t     dump_filename[MAX_PATH];
    wchar_t     log_filename[MAX_PATH];
    wchar_t     save_filename[MAX_PATH];
    wchar_t     zip_filename[MAX_PATH];
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
    swprintf_s(dump_filename, L"%s\\%s\\%s.%s.dmp", modulename, L"CrashDumps", modulenameptr, timestamp);

    hFile = CreateFileW(dump_filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
    swprintf_s(log_filename, L"%s\\%s\\%s.%s.log", modulename, L"CrashDumps", modulenameptr, timestamp);
    hFile = CreateFileW(log_filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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

    // Dump the games save file, if it's loaded in
    // (minidump unfortunately doesn't contain this as gamesave memory is dynamic allocated for some reason)
    auto* p_GameSave_BufPtr = GameSavePtr();
    if (p_GameSave_BufPtr)
    {
      swprintf_s(save_filename, L"%s\\%s\\%s.%s.save", modulename, L"CrashDumps", modulenameptr, timestamp);
      FILE* save_file;
      if (_wfopen_s(&save_file, save_filename, L"wb") == 0 && save_file)
      {
        fwrite(p_GameSave_BufPtr, 1, GAMESAVE_LENGTH, save_file);
        fclose(save_file);
      }
    }

    // ZIP up the dump/log/save
    {
      swprintf_s(zip_filename, L"%s\\%s\\%s.%s.zip", modulename, L"CrashDumps", modulenameptr, timestamp);

      bool zip_created = false;

      FILE* zip_file;
      if (_wfopen_s(&zip_file, zip_filename, L"wb") == 0 && zip_file)
      {
        mz_zip_archive zip_archive;
        mz_zip_zero_struct(&zip_archive);
        if (mz_zip_writer_init_cfile(&zip_archive, zip_file, 3))
        {
          // Even if one of these fails we want all to attempt being added, so don't think we can do any assignment trick here...
          zip_created = true;

          if (!mz_zip_writer_add_file(&zip_archive, "dump.dmp", dump_filename, nullptr, 0, 3))
            zip_created = false;
          if (!mz_zip_writer_add_file(&zip_archive, "log.log", log_filename, nullptr, 0, 3))
            zip_created = false;

          if (p_GameSave_BufPtr)
            if (!mz_zip_writer_add_file(&zip_archive, "savegame00.sav", save_filename, nullptr, 0, 3))
              zip_created = false;

          mz_zip_writer_finalize_archive(&zip_archive);
          mz_zip_writer_end(&zip_archive);
        }
        fclose(zip_file);
      }

      if (zip_created)
      {
        DeleteFileW(dump_filename);
        DeleteFileW(log_filename);
        if (p_GameSave_BufPtr)
          DeleteFileW(save_filename);
      }
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