#include "wrappers.h"
#include "shared.h"
#include "../dllmain/dllmain.h"
#include <initguid.h>

// Wrappers
#include "bink2w64.h"
#include "binkw32.h"
#include "d3d8.h"
#include "d3d9.h"
#include "d3d10.h"
#include "d3d11.h"
#include "d3d12.h"
#include "ddraw.h"
#include "dinput.h"
#include "dinput8.h"
#include "dsound.h"
#include "msacm32.h"
#include "msvfw32.h"
#include "version.h"
#include "wininet.h"
#include "winmm.h"
#include "xinput1_3.h"
#include "X3DAudio1_7.h"

//#define EnableKernel32Hook

std::wstring wrapperName;
std::wstring rootPath;
std::wstring logPath;
std::wstring customDllPath = L"";

// Prefer Nvidia and AMD high performance graphics
extern "C" { _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }

bool iequals(std::wstring_view s1, std::wstring_view s2)
{
    std::wstring str1(std::move(s1));
    std::wstring str2(std::move(s2));
    std::transform(str1.begin(), str1.end(), str1.begin(), [](wchar_t c) { return ::towlower(c); });
    std::transform(str2.begin(), str2.end(), str2.begin(), [](wchar_t c) { return ::towlower(c); });
    return (str1 == str2);
}

std::wstring SHGetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken)
{
    std::wstring r;
    WCHAR* szSystemPath = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(rfid, dwFlags, hToken, &szSystemPath)))
    {
        r = szSystemPath;
    }
    CoTaskMemFree(szSystemPath);
    return r;
};

HMODULE LoadLibraryW(const std::wstring& lpLibFileName)
{
    return LoadLibraryW(lpLibFileName.c_str());
}

static LONG OriginalLibraryLoaded = 0;
void LoadOriginalLibrary()
{
    if (_InterlockedCompareExchange(&OriginalLibraryLoaded, 1, 0) != 0) return;

    std::wstring szSelfName = wrapperName + L".dll";
    std::wstring szLocalPath = rootPath;
    std::wstring szDllPath;

    if (!customDllPath.empty())
    {
        // User-specified dll
        szDllPath = customDllPath;

        spd::log()->info("Loading '{}' as real dll...", WstrToStr(szDllPath)); 
    }
    else
    {
        // System dll
        szDllPath = SHGetKnownFolderPath(FOLDERID_System, 0, nullptr) + L'\\' + szSelfName;

        spd::log()->info("Loading '{}' from System32...", WstrToStr(szSelfName));
    }

    if (iequals(szSelfName, L"dsound.dll"))
    {
        dsound.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"dinput8.dll"))
    {
        dinput8.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"wininet.dll"))
    {
        wininet.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"version.dll"))
    {
        version.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"d3d9.dll"))
    {
        d3d9.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"d3d10.dll"))
    {
        d3d10.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"d3d11.dll"))
    {
        d3d11.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"d3d12.dll"))
    {
        d3d12.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    } 
    else if (iequals(szSelfName, L"xinput1_3.dll"))
    {
        xinput1_3.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"X3DAudio1_7.dll"))
    {
        X3DAudio1_7.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"winmm.dll"))
    {
        winmm.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    } else
#if !X64
    if (iequals(szSelfName, L"ddraw.dll"))
    {
        ddraw.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"d3d8.dll"))
    {
        d3d8.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"msacm32.dll"))
    {
        msacm32.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"dinput.dll"))
    {
        dinput.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"msvfw32.dll"))
    {
        msvfw32.LoadOriginalLibrary(LoadLibraryW(szDllPath));
    }
    else if (iequals(szSelfName, L"binkw32.dll"))
    {
        szLocalPath += L"binkw32Hooked.dll";
        if (std::filesystem::exists(szLocalPath))
        {
            binkw32.LoadOriginalLibrary(LoadLibraryW(szLocalPath));
        }
        else
        {
            spd::log()->info("Error: \"binkw32Hooked.dll\" doesn't exist!");
        }
    }
    else
#else
    if (iequals(szSelfName, L"bink2w64.dll"))
    {
        szLocalPath += L"bink2w64Hooked.dll";
        if (std::filesystem::exists(szLocalPath))
        {
            bink2w64.LoadOriginalLibrary(LoadLibraryW(szLocalPath));
        }
        else
        {
            spd::log()->info("Error: \"bink2w64Hooked.dll\" doesn't exist!");
        }
    } 
    else
#endif
    {
        spd::log()->info("Error: Unsupported wrapper '{}'!", WstrToStr(szSelfName));

        MessageBox(0, TEXT("This library isn't supported."), TEXT("re4_tweaks"), MB_ICONERROR);
        ExitProcess(0);
    }

}

#ifdef EnableKernel32Hook
template<typename T, typename... Args>
void GetSections(T&& h, Args... args)
{
    const std::set< std::string_view, std::less<> > s = { args... };
    size_t dwLoadOffset = (size_t)GetModuleHandle(NULL);
    BYTE* pImageBase = reinterpret_cast<BYTE*>(dwLoadOffset);
    PIMAGE_DOS_HEADER   pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(dwLoadOffset);
    PIMAGE_NT_HEADERS   pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(pImageBase + pDosHeader->e_lfanew);
    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNtHeader);
    for (int iSection = 0; iSection < pNtHeader->FileHeader.NumberOfSections; ++iSection, ++pSection)
    {
        auto pszSectionName = reinterpret_cast<const char*>(pSection->Name);
        if (s.find(pszSectionName) != s.end())
        {
            DWORD dwPhysSize = (pSection->Misc.VirtualSize + 4095) & ~4095;
            std::forward<T>(h)(pSection, dwLoadOffset, dwPhysSize);
        }
    }
}

enum Kernel32ExportsNames
{
    eGetStartupInfoA,
    eGetStartupInfoW,
    eGetModuleHandleA,
    eGetModuleHandleW,
    eGetProcAddress,
    eGetShortPathNameA,
    eFindNextFileA,
    eFindNextFileW,
    eLoadLibraryA,
    eLoadLibraryW,
    eFreeLibrary,
    eCreateEventA,
    eCreateEventW,
    eGetSystemInfo,
    eInterlockedCompareExchange,
    eSleep,

    Kernel32ExportsNamesCount
};

enum Kernel32ExportsData
{
    IATPtr,
    ProcAddress,

    Kernel32ExportsDataCount
};

size_t Kernel32Data[Kernel32ExportsNamesCount][Kernel32ExportsDataCount];


static LONG RestoredOnce = 0;
void LoadAndRestoreIAT(uintptr_t retaddr)
{
    bool calledFromBind = false;

    //steam drm check
    GetSections([&](PIMAGE_SECTION_HEADER pSection, size_t dwLoadOffset, DWORD dwPhysSize)
    {
        auto dwStart = static_cast<uintptr_t>(dwLoadOffset + pSection->VirtualAddress);
        auto dwEnd = dwStart + dwPhysSize;
        if (retaddr >= dwStart && retaddr <= dwEnd)
            calledFromBind = true;
    }, ".bind");

    if (calledFromBind) return;

    if (_InterlockedCompareExchange(&RestoredOnce, 1, 0) != 0) return;

    LoadOriginalLibrary();

    for (size_t i = 0; i < Kernel32ExportsNamesCount; i++)
    {
        if (Kernel32Data[i][IATPtr] && Kernel32Data[i][ProcAddress])
        {
            auto ptr = (size_t*)Kernel32Data[i][IATPtr];
            DWORD dwProtect[2];
            VirtualProtect(ptr, sizeof(size_t), PAGE_EXECUTE_READWRITE, &dwProtect[0]);
            *ptr = Kernel32Data[i][ProcAddress];
            VirtualProtect(ptr, sizeof(size_t), dwProtect[0], &dwProtect[1]);
        }
    }
}

void WINAPI CustomGetStartupInfoA(LPSTARTUPINFOA lpStartupInfo)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetStartupInfoA(lpStartupInfo);
}

void WINAPI CustomGetStartupInfoW(LPSTARTUPINFOW lpStartupInfo)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetStartupInfoW(lpStartupInfo);
}

HMODULE WINAPI CustomGetModuleHandleA(LPCSTR lpModuleName)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetModuleHandleA(lpModuleName);
}

HMODULE WINAPI CustomGetModuleHandleW(LPCWSTR lpModuleName)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetModuleHandleW(lpModuleName);
}

FARPROC WINAPI CustomGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetProcAddress(hModule, lpProcName);
}

DWORD WINAPI CustomGetShortPathNameA(LPCSTR lpszLongPath, LPSTR lpszShortPath, DWORD cchBuffer)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetShortPathNameA(lpszLongPath, lpszShortPath, cchBuffer);
}

BOOL WINAPI CustomFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return FindNextFileA(hFindFile, lpFindFileData);
}

BOOL WINAPI CustomFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return FindNextFileW(hFindFile, lpFindFileData);
}

HMODULE WINAPI CustomLoadLibraryA(LPCSTR lpLibFileName)
{
    LoadOriginalLibrary();

    return LoadLibraryA(lpLibFileName);
}

HMODULE WINAPI CustomLoadLibraryW(LPCWSTR lpLibFileName)
{
    LoadOriginalLibrary();

    return LoadLibraryW(lpLibFileName);
}

BOOL WINAPI CustomFreeLibrary(HMODULE hLibModule)
{
    if (hLibModule != g_module_handle)
        return FreeLibrary(hLibModule);
    else
        return !NULL;
}

HANDLE WINAPI CustomCreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return CreateEventA(lpEventAttributes, bManualReset, bInitialState, lpName);
}

HANDLE WINAPI CustomCreateEventW(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
}

void WINAPI CustomGetSystemInfo(LPSYSTEM_INFO lpSystemInfo)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return GetSystemInfo(lpSystemInfo);
}

LONG WINAPI CustomInterlockedCompareExchange(LONG volatile* Destination, LONG ExChange, LONG Comperand)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return _InterlockedCompareExchange(Destination, ExChange, Comperand);
}

void WINAPI CustomSleep(DWORD dwMilliseconds)
{
    LoadAndRestoreIAT((uintptr_t)_ReturnAddress());
    return Sleep(dwMilliseconds);
}

DEFINE_GUID(CLSID_DirectInput, 0x25E609E0, 0xB259, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEFINE_GUID(CLSID_DirectInput8, 0x25E609E4, 0xB259, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEFINE_GUID(CLSID_WinInet, 0xC39EE728, 0xD419, 0x4BD4, 0xA3, 0xEF, 0xED, 0xA0, 0x59, 0xDB, 0xD9, 0x35);
HRESULT WINAPI CustomCoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    HRESULT hr = REGDB_E_KEYMISSING;
    HMODULE hDll = NULL;

    if (rclsid == CLSID_DirectInput8)
        hDll = ::LoadLibrary(L"dinput8.dll");
    else if (rclsid == CLSID_DirectInput)
        hDll = ::LoadLibrary(L"dinput.dll");
    else if (rclsid == CLSID_WinInet)
        hDll = ::LoadLibrary(L"wininet.dll");

    if (hDll == NULL)
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);

    typedef HRESULT(__stdcall *pDllGetClassObject)(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv);

    pDllGetClassObject GetClassObject = (pDllGetClassObject)::GetProcAddress(hDll, "DllGetClassObject");
    if (GetClassObject == NULL)
    {
        ::FreeLibrary(hDll);
        return hr;
    }

    IClassFactory *pIFactory;

    hr = GetClassObject(rclsid, IID_IClassFactory, (LPVOID *)&pIFactory);

    if (!SUCCEEDED(hr))
        return hr;

    hr = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
    pIFactory->Release();

    return hr;
}

bool HookKernel32IAT(HMODULE mod, bool exe)
{
    auto hExecutableInstance = (size_t)mod;
    IMAGE_NT_HEADERS*           ntHeader = (IMAGE_NT_HEADERS*)(hExecutableInstance + ((IMAGE_DOS_HEADER*)hExecutableInstance)->e_lfanew);
    IMAGE_IMPORT_DESCRIPTOR*    pImports = (IMAGE_IMPORT_DESCRIPTOR*)(hExecutableInstance + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    size_t                      nNumImports = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;

    if (exe)
    {
        Kernel32Data[eGetStartupInfoA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetStartupInfoA");
        Kernel32Data[eGetStartupInfoW][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetStartupInfoW");
        Kernel32Data[eGetModuleHandleA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetModuleHandleA");
        Kernel32Data[eGetModuleHandleW][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetModuleHandleW");
        Kernel32Data[eGetProcAddress][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetProcAddress");
        Kernel32Data[eGetShortPathNameA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetShortPathNameA");
        Kernel32Data[eFindNextFileA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "FindNextFileA");
        Kernel32Data[eFindNextFileW][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "FindNextFileW");
        Kernel32Data[eLoadLibraryA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "LoadLibraryA");
        Kernel32Data[eLoadLibraryW][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "LoadLibraryW");
        Kernel32Data[eFreeLibrary][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "FreeLibrary");
        Kernel32Data[eCreateEventA][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "CreateEventA");
        Kernel32Data[eCreateEventW][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "CreateEventW");
        Kernel32Data[eGetSystemInfo][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "GetSystemInfo");
        Kernel32Data[eInterlockedCompareExchange][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "InterlockedCompareExchange");
        Kernel32Data[eSleep][ProcAddress] = (size_t)GetProcAddress(GetModuleHandle(TEXT("KERNEL32.DLL")), "Sleep");
    }

    uint32_t matchedImports = 0;

    auto PatchIAT = [&](size_t start, size_t end, size_t exe_end)
    {
        for (size_t i = 0; i < nNumImports; i++)
        {
            if (hExecutableInstance + (pImports + i)->FirstThunk > start && !(end && hExecutableInstance + (pImports + i)->FirstThunk > end))
                end = hExecutableInstance + (pImports + i)->FirstThunk;
        }

        if (!end) { end = start + 0x100; }
        if (end > exe_end) //for very broken exes
        {
            start = hExecutableInstance;
            end = exe_end;
        }

        for (auto i = start; i < end; i += sizeof(size_t))
        {
            DWORD dwProtect[2];
            VirtualProtect((size_t*)i, sizeof(size_t), PAGE_EXECUTE_READWRITE, &dwProtect[0]);

            auto ptr = *(size_t*)i;
            if (!ptr)
                continue;

            if (ptr == Kernel32Data[eGetStartupInfoA][ProcAddress])
            {
                if (exe) Kernel32Data[eGetStartupInfoA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetStartupInfoA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetStartupInfoW][ProcAddress])
            {
                if (exe) Kernel32Data[eGetStartupInfoW][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetStartupInfoW;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetModuleHandleA][ProcAddress])
            {
                if (exe) Kernel32Data[eGetModuleHandleA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetModuleHandleA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetModuleHandleW][ProcAddress])
            {
                if (exe) Kernel32Data[eGetModuleHandleW][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetModuleHandleW;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetProcAddress][ProcAddress])
            {
                if (exe) Kernel32Data[eGetProcAddress][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetProcAddress;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetShortPathNameA][ProcAddress])
            {
                if (exe) Kernel32Data[eGetShortPathNameA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetShortPathNameA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eFindNextFileA][ProcAddress])
            {
                if (exe) Kernel32Data[eFindNextFileA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomFindNextFileA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eFindNextFileW][ProcAddress])
            {
                if (exe) Kernel32Data[eFindNextFileW][IATPtr] = i;
                *(size_t*)i = (size_t)CustomFindNextFileW;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eLoadLibraryA][ProcAddress])
            {
                if (exe) Kernel32Data[eLoadLibraryA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomLoadLibraryA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eLoadLibraryW][ProcAddress])
            {
                if (exe) Kernel32Data[eLoadLibraryW][IATPtr] = i;
                *(size_t*)i = (size_t)CustomLoadLibraryW;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eFreeLibrary][ProcAddress])
            {
                if (exe) Kernel32Data[eFreeLibrary][IATPtr] = i;
                *(size_t*)i = (size_t)CustomFreeLibrary;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eCreateEventA][ProcAddress])
            {
                if (exe) Kernel32Data[eCreateEventA][IATPtr] = i;
                *(size_t*)i = (size_t)CustomCreateEventA;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eCreateEventW][ProcAddress])
            {
                if (exe) Kernel32Data[eCreateEventW][IATPtr] = i;
                *(size_t*)i = (size_t)CustomCreateEventW;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eGetSystemInfo][ProcAddress])
            {
                if (exe) Kernel32Data[eGetSystemInfo][IATPtr] = i;
                *(size_t*)i = (size_t)CustomGetSystemInfo;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eInterlockedCompareExchange][ProcAddress])
            {
                if (exe) Kernel32Data[eInterlockedCompareExchange][IATPtr] = i;
                *(size_t*)i = (size_t)CustomInterlockedCompareExchange;
                matchedImports++;
            }
            else if (ptr == Kernel32Data[eSleep][ProcAddress])
            {
                if (exe) Kernel32Data[eSleep][IATPtr] = i;
                *(size_t*)i = (size_t)CustomSleep;
                matchedImports++;
            }

            VirtualProtect((size_t*)i, sizeof(size_t), dwProtect[0], &dwProtect[1]);
        }
    };

    auto PatchCoCreateInstance = [&](size_t start, size_t end, size_t exe_end)
    {
        for (size_t i = 0; i < nNumImports; i++)
        {
            if (hExecutableInstance + (pImports + i)->FirstThunk > start && !(end && hExecutableInstance + (pImports + i)->FirstThunk > end))
                end = hExecutableInstance + (pImports + i)->FirstThunk;
        }

        if (!end) { end = start + 0x100; }
        if (end > exe_end) //for very broken exes
        {
            start = hExecutableInstance;
            end = exe_end;
        }

        for (auto i = start; i < end; i += sizeof(size_t))
        {
            DWORD dwProtect[2];
            VirtualProtect((size_t*)i, sizeof(size_t), PAGE_EXECUTE_READWRITE, &dwProtect[0]);

            auto ptr = *(size_t*)i;
            if (!ptr)
                continue;

            if (ptr == (size_t)GetProcAddress(GetModuleHandle(TEXT("OLE32.DLL")), "CoCreateInstance"))
            {
                *(size_t*)i = (size_t)CustomCoCreateInstance;
                VirtualProtect((size_t*)i, sizeof(size_t), dwProtect[0], &dwProtect[1]);
                break;
            }

            VirtualProtect((size_t*)i, sizeof(size_t), dwProtect[0], &dwProtect[1]);
        }
    };

    static auto getSection = [](const PIMAGE_NT_HEADERS nt_headers, unsigned section) -> PIMAGE_SECTION_HEADER
    {
        return reinterpret_cast<PIMAGE_SECTION_HEADER>(
            (UCHAR*)nt_headers->OptionalHeader.DataDirectory +
            nt_headers->OptionalHeader.NumberOfRvaAndSizes * sizeof(IMAGE_DATA_DIRECTORY) +
            section * sizeof(IMAGE_SECTION_HEADER));
    };

    static auto getSectionEnd = [](IMAGE_NT_HEADERS* ntHeader, size_t inst) -> auto
    {
        auto sec = getSection(ntHeader, ntHeader->FileHeader.NumberOfSections - 1);
        // .bind section may have vanished from the executable (test case: Yakuza 4)
        // so back to the first valid section if that happened
        while (sec->Misc.VirtualSize == 0) sec--;

        auto secSize = max(sec->SizeOfRawData, sec->Misc.VirtualSize);
        auto end = inst + max(sec->PointerToRawData, sec->VirtualAddress) + secSize;
        return end;
    };

    auto hExecutableInstance_end = getSectionEnd(ntHeader, hExecutableInstance);

    // Find kernel32.dll
    for (size_t i = 0; i < nNumImports; i++)
    {
        if ((size_t)(hExecutableInstance + (pImports + i)->Name) < hExecutableInstance_end)
        {
            if (!_stricmp((const char*)(hExecutableInstance + (pImports + i)->Name), "KERNEL32.DLL"))
                PatchIAT(hExecutableInstance + (pImports + i)->FirstThunk, 0, hExecutableInstance_end);
            else if (!_stricmp((const char*)(hExecutableInstance + (pImports + i)->Name), "OLE32.DLL"))
                PatchCoCreateInstance(hExecutableInstance + (pImports + i)->FirstThunk, 0, hExecutableInstance_end);
        }
    }

    // Fixing ordinals
    std::wstring szSelfName = wrapperName + L".dll";

    static auto PatchOrdinals = [&szSelfName](size_t hInstance)
    {
        IMAGE_NT_HEADERS*           ntHeader = (IMAGE_NT_HEADERS*)(hInstance + ((IMAGE_DOS_HEADER*)hInstance)->e_lfanew);
        IMAGE_IMPORT_DESCRIPTOR*    pImports = (IMAGE_IMPORT_DESCRIPTOR*)(hInstance + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        size_t                      nNumImports = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;

        if (nNumImports == (size_t)-1)
            return;

        for (size_t i = 0; i < nNumImports; i++)
        {
            if ((size_t)(hInstance + (pImports + i)->Name) < getSectionEnd(ntHeader, (size_t)hInstance))
            {
                if (iequals(szSelfName, (StrToWstr((const char*)(hInstance + (pImports + i)->Name)))))
                {
                    PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)(hInstance + (pImports + i)->OriginalFirstThunk);
                    size_t j = 0;
                    while (thunk->u1.Function)
                    {
                        if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                        {
                            PIMAGE_IMPORT_BY_NAME import = (PIMAGE_IMPORT_BY_NAME)(hInstance + thunk->u1.AddressOfData);
                            void** p = (void**)(hInstance + (pImports + i)->FirstThunk);
                            if (iequals(szSelfName, L"dsound.dll"))
                            {
                                DWORD Protect;
                                VirtualProtect(&p[j], 4, PAGE_EXECUTE_READWRITE, &Protect);

                                const enum edsound
                                {
                                    DirectSoundCaptureCreate = 6,
                                    DirectSoundCaptureCreate8 = 12,
                                    DirectSoundCaptureEnumerateA = 7,
                                    DirectSoundCaptureEnumerateW = 8,
                                    DirectSoundCreate = 1,
                                    DirectSoundCreate8 = 11,
                                    DirectSoundEnumerateA = 2,
                                    DirectSoundEnumerateW = 3,
                                    DirectSoundFullDuplexCreate = 10,
                                    GetDeviceID = 9
                                };

                                switch (IMAGE_ORDINAL(thunk->u1.Ordinal))
                                {
                                case edsound::DirectSoundCaptureCreate:
                                    p[j] = _DirectSoundCaptureCreate;
                                    break;
                                case edsound::DirectSoundCaptureCreate8:
                                    p[j] = _DirectSoundCaptureCreate8;
                                    break;
                                case edsound::DirectSoundCaptureEnumerateA:
                                    p[j] = _DirectSoundCaptureEnumerateA;
                                    break;
                                case edsound::DirectSoundCaptureEnumerateW:
                                    p[j] = _DirectSoundCaptureEnumerateW;
                                    break;
                                case edsound::DirectSoundCreate:
                                    p[j] = _DirectSoundCreate;
                                    break;
                                case edsound::DirectSoundCreate8:
                                    p[j] = _DirectSoundCreate8;
                                    break;
                                case edsound::DirectSoundEnumerateA:
                                    p[j] = _DirectSoundEnumerateA;
                                    break;
                                case edsound::DirectSoundEnumerateW:
                                    p[j] = _DirectSoundEnumerateW;
                                    break;
                                case edsound::DirectSoundFullDuplexCreate:
                                    p[j] = _DirectSoundFullDuplexCreate;
                                    break;
                                case edsound::GetDeviceID:
                                    p[j] = _GetDeviceID;
                                    break;
                                default:
                                    break;
                                }
                            }
                            else if (iequals(szSelfName, L"dinput8.dll"))
                            {
                                DWORD Protect;
                                VirtualProtect(&p[j], 4, PAGE_EXECUTE_READWRITE, &Protect);

                                if ((IMAGE_ORDINAL(thunk->u1.Ordinal)) == 1)
                                    p[j] = _DirectInput8Create;
                            }
                            ++j;
                        }
                        ++thunk;
                    }
                    break;
                }
            }
        }
    };

    ModuleList dlls;
    dlls.Enumerate(ModuleList::SearchLocation::LocalOnly);
    for (auto& e : dlls.m_moduleList)
    {
        PatchOrdinals((size_t)std::get<HMODULE>(e));
    }
    return matchedImports > 0;
}
#endif

void Init_Wrappers()
{
    // Store root path
    rootPath = GetModuleFileNameW(g_module_handle);
    rootPath.resize(rootPath.find_last_of(L"/\\") + 1);

    // Store wrapper name without extension
    wrapperName = GetModuleFileNameW(g_module_handle);
    wrapperName = wrapperName.substr(wrapperName.find_last_of(L"/\\") + 1);
    wrapperName.resize(wrapperName.find_last_of(L'.'));
    std::transform(wrapperName.begin(), wrapperName.end(), wrapperName.begin(), ::tolower);

    // Store log path
    logPath = rootPath + wrapperName + L".log";

    // Read WrappedDLLPath from .ini file
    std::wstring iniPath = rootPath + wrapperName + L".ini";

    wchar_t userDllPath[MAX_PATH] = {};

    GetPrivateProfileStringW(L"MISC", L"WrappedDLLPath", L"", userDllPath, 256, iniPath.c_str());

    if (wcslen(userDllPath) > 0)
    {
        customDllPath = rootPath + userDllPath;
        
        // User has specified a DLL to wrap, make sure it exists first:
        if (!std::filesystem::exists(customDllPath))
        {
            spd::log()->info("Error: userDllPath specified, but the .dll doesn't exist");

            customDllPath.clear();
        }
    }

#ifdef EnableKernel32Hook
    bool nDontLoadFromDllMain = true;
    bool nFindModule = false;

    if (nDontLoadFromDllMain)
    {
        HMODULE mainModule = GetModuleHandle(NULL);
        bool hookedSuccessfully = HookKernel32IAT(mainModule, true);
        if (!hookedSuccessfully)
        {
            LoadOriginalLibrary();
        }

        HMODULE m = mainModule;
        if (nFindModule)
        {
            ModuleList dlls;
            dlls.Enumerate(ModuleList::SearchLocation::LocalOnly);

            auto ual = std::find_if(dlls.m_moduleList.begin(), dlls.m_moduleList.end(), [](auto const& it)
            {
                return std::get<HMODULE>(it) == g_module_handle;
            });

            auto sim = std::find_if(dlls.m_moduleList.rbegin(), dlls.m_moduleList.rend(), [&ual](auto const& it)
            {
                auto str1 = std::get<std::wstring>(*ual);
                auto str2 = std::get<std::wstring>(it);
                std::transform(str1.begin(), str1.end(), str1.begin(), [](wchar_t c) { return ::towlower(c); });
                std::transform(str2.begin(), str2.end(), str2.begin(), [](wchar_t c) { return ::towlower(c); });

                return (str2 != str1) && (str2.find(str1) != std::wstring::npos);
            });

            if (ual != dlls.m_moduleList.begin())
            {
                if (sim != dlls.m_moduleList.rend())
                    m = std::get<HMODULE>(*sim);
                else
                    m = std::get<HMODULE>(*std::prev(ual, 1));
            }
        }

        if (m != mainModule)
        {
            HookKernel32IAT(m, false);
        }
    }
    else
#endif
    {
        LoadOriginalLibrary();
    }
}
