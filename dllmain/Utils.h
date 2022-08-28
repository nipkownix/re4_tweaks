#pragma once
#include <string>

template <typename I> std::string IntToHexStr(I w, size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[((int)w >> j) & 0x0f];
	return std::string("0x") + rc;
}

std::string WstrToStr(const std::wstring& wstr);
std::wstring StrToWstr(const std::string& str);
std::string StrToUpper(std::string str);
float GetRandomFloat(float min, float max);

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);