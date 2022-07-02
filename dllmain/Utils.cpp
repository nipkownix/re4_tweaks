#include <iostream>
#include "dllmain.h"

// ImGui can't render special chars such as "Ç" without this. Meh.
std::string StrToUTF8(std::string const& str)
{
	int slength = (int)str.length() + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	std::wstring wide(len, L'\0');
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &wide[0], len);

	slength = (int)wide.length() + 1;
	len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), slength, 0, 0, 0, 0);
	std::string utf8(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), slength, &utf8[0], len, 0, 0);

	return utf8;
}

std::string WstrToStr(const std::wstring& wstr)
{
	const int BUFF_SIZE = 7;
	if (MB_CUR_MAX >= BUFF_SIZE) throw std::invalid_argument("BUFF_SIZE too small");
	std::string result;
	bool shifts = std::wctomb(nullptr, 0);  // reset the conversion state
	for (const wchar_t wc : wstr)
	{
		std::array<char, BUFF_SIZE> buffer;
		const int ret = std::wctomb(buffer.data(), wc);
		if (ret < 0) throw std::invalid_argument("inconvertible wide characters in the current locale");
		buffer[ret] = '\0';  // make 'buffer' contain a C-style string
		result = result + std::string(buffer.data());
	}
	return result;
}

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	CloseHandle(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}