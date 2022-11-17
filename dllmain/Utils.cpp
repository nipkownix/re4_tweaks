#include <iostream>
#include "dllmain.h"
#include <shellapi.h>

std::string WstrToStr(const std::wstring& wstr)
{
	size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

	if (len <= 1)
		return "";

	len -= 1;

	std::string result;
	result.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result.at(0), len, nullptr, nullptr);
	return result;
}

std::wstring StrToWstr(const std::string& str) 
{
	size_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);

    if (len <= 1)
        return L"";

    len -= 1;

    std::wstring result;
    result.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result.at(0), len);
    return result;
}

std::string StrToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

float GetRandomFloat(float min, float max)
{
	static std::mt19937 engi;
	std::uniform_real_distribution<float> num(min, max); // range min - max
	return num(engi);
}

float get_distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return hypot(hypot(x1 - x2, y1 - y2), z1 - z2);
}

bool WorldToScreen(Vec pos, Vec& screen, Mtx44 matrix, float fov, float windowWidth, float windowHeight)
{
	// Game sometimes passes 0.0f as the fov for some reason?
	if (fov <= 0.0)
		return false;

	float fovDegCorrected = fov / 1.3f; // Not sure why this game needs this

	float fovRadians = DirectX::XMConvertToRadians(fovDegCorrected);

	DirectX::XMMATRIX matrixProjection = DirectX::XMMatrixPerspectiveFovRH(fovRadians, windowWidth / windowHeight, 0.1f, 10000.0f);

	// Copy game matrix to a XMMATRIX in order to multiply later
	DirectX::XMMATRIX view{};

	for (size_t row = 0; row < 4; ++row)
		for (size_t column = 0; column < 4; ++column)
			view.r[row].m128_f32[column] = matrix[row][column];

	view.r[3].m128_f32[0] = 0.f;
	view.r[3].m128_f32[1] = 0.f;
	view.r[3].m128_f32[2] = 0.f;
	view.r[3].m128_f32[3] = 1.f;

	// Multiply with camera matrix to generate ViewProjection matrix
	matrixProjection *= view;

	Quaternion clip_coords{};
	clip_coords.x = pos.x * matrixProjection.r[0].m128_f32[0] + pos.y * matrixProjection.r[0].m128_f32[1] + pos.z * matrixProjection.r[0].m128_f32[2] + matrixProjection.r[0].m128_f32[3];
	clip_coords.y = pos.x * matrixProjection.r[1].m128_f32[0] + pos.y * matrixProjection.r[1].m128_f32[1] + pos.z * matrixProjection.r[1].m128_f32[2] + matrixProjection.r[1].m128_f32[3];
	clip_coords.z = pos.x * matrixProjection.r[2].m128_f32[0] + pos.y * matrixProjection.r[2].m128_f32[1] + pos.z * matrixProjection.r[2].m128_f32[2] + matrixProjection.r[2].m128_f32[3];
	clip_coords.w = pos.x * matrixProjection.r[3].m128_f32[0] + pos.y * matrixProjection.r[3].m128_f32[1] + pos.z * matrixProjection.r[3].m128_f32[2] + matrixProjection.r[3].m128_f32[3];

	if (clip_coords.w < 0.1f)
		return false;

	Vec NDC{};
	NDC.x = clip_coords.x / (clip_coords.w * 10);
	NDC.y = clip_coords.y / (clip_coords.w * 10);
	NDC.z = clip_coords.z / (clip_coords.w * 10);

	screen.x = (windowWidth / 2.0f * NDC.x) + (NDC.x + windowWidth / 2.0f);
	screen.y = -(windowHeight / 2.0f * NDC.y) + (NDC.y + windowHeight / 2.0f);

	return true;
}

ImU32 getHealthColor(float health)
{
	const float multiplier = 2.55f;

	// no health = max red, full health = max green
	int red = 255 - (int)(health * multiplier);
	int green = (int)(health * multiplier);
	int blue = 0;
	int alpha = 255;

	ImU32 Color = IM_COL32(red, green, blue, alpha);
	return Color;
}

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	CloseHandle(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}

std::string CmdIniReader::SearchCommandLineParamValue(std::string_view param_name, bool is_boolean)
{
	if (!argv)
		argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	// Convert param name to wide char to make comparing against wide args easier
	WCHAR param_buf[256];
	if (!SUCCEEDED(MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, param_name.data(), -1, param_buf, 256)))
		return "";

	BOOL lpUsedDefaultChar = FALSE;
	char arg_buf[256];
	for (int i = 1; i < argc; i++)
	{
		// Make sure the arg is at least large enough for a switch char (- or /) along with 1 extra char
		if (wcslen(argv[i]) <= 1)
			continue;

		// Is this arg the param we're looking for?
		if (_wcsicmp(param_buf, argv[i] + 1) != 0) // argv[i] + 1 to skip the switch character
			continue;

		// Found the param, grab the value
		if (i + 1 < argc)
		{
			if (SUCCEEDED(WideCharToMultiByte(CP_ACP, 0, argv[i + 1], -1, arg_buf, 256, NULL, &lpUsedDefaultChar)))
				return arg_buf;
		}
		else
		{
			// Param was specified but no value given, if it's a bool then we'll treat specifying it as true
			if (is_boolean)
				return "true";
		}
	}

	return "";
}

CmdIniReader::~CmdIniReader()
{
	if (argv)
		LocalFree(argv);
	argv = nullptr;
	argc = 0;
}

bool CmdIniReader::ReadBoolean(std::string_view szSection, std::string_view szKey, bool bolDefaultValue)
{
	bool value = CIniReader::ReadBoolean(szSection, szKey, bolDefaultValue);

	std::string cmd_value = SearchCommandLineParamValue(szKey, true);
	if (!cmd_value.empty())
		value = !(_stricmp(cmd_value.c_str(), "false") == 0 || _stricmp(cmd_value.c_str(), "0") == 0);

	return value;
}

float CmdIniReader::ReadFloat(std::string_view szSection, std::string_view szKey, float fltDefaultValue)
{
	float value = CIniReader::ReadFloat(szSection, szKey, fltDefaultValue);

	std::string cmd_value = SearchCommandLineParamValue(szKey);
	if (!cmd_value.empty())
	{
		try
		{
			value = (float)std::stod(cmd_value);
		}
		catch (std::exception&) {}
	}

	return value;
}

int CmdIniReader::ReadInteger(std::string_view szSection, std::string_view szKey, int iDefaultValue)
{
	int value = CIniReader::ReadInteger(szSection, szKey, iDefaultValue);

	std::string cmd_value = SearchCommandLineParamValue(szKey);
	if (!cmd_value.empty())
	{
		try
		{
			value = std::stoi(cmd_value, nullptr, 0);
		}
		catch (std::exception&) {}
	}

	return value;
}

std::string CmdIniReader::ReadString(std::string_view szSection, std::string_view szKey, std::string_view szDefaultValue)
{
	std::string value = CIniReader::ReadString(szSection, szKey, szDefaultValue);

	std::string cmd_value = SearchCommandLineParamValue(szKey);
	if (!cmd_value.empty())
		value = cmd_value;

	return value;
}
