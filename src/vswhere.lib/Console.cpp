// <copyright file="Console.cpp" company="Microsoft Corporation">
// Copyright (C) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt in the project root for license information.
// </copyright>

#include "stdafx.h"

using namespace std;

namespace
{
	std::string WcsToMbsString(const std::wstring &wstr, int cp)
	{
		auto len = WideCharToMultiByte(cp, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
		std::vector<char> mbuf(len, 0);
		WideCharToMultiByte(cp, 0, wstr.c_str(), wstr.length(), mbuf.data(), len, NULL, NULL);
		return std::string(mbuf.begin(), mbuf.end());
	}
	std::string WcsToMbsString(const wchar_t *wstr, int wlen, int cp)
	{
		auto len = WideCharToMultiByte(cp, 0, wstr, wlen, NULL, 0, NULL, NULL);
		std::vector<char> mbuf(len, 0);
		WideCharToMultiByte(cp, 0, wstr, wlen, mbuf.data(), len, NULL, NULL);
		return std::string(mbuf.begin(), mbuf.end());
	}
}

void Console::Initialize() noexcept
{
    if (IsConsole(stdout))
    {
		if (!m_args.get_IsUtf8Output())
		{
			::_setmode(_fileno(stdout), _O_WTEXT);
		}
		else
		{
			::_setmode(_fileno(stdout), _O_TEXT);
		}
    }
    else
    {
        char sz[10];
        ::sprintf_s(sz, ".%d", ::GetConsoleCP());

        ::setlocale(LC_CTYPE, sz);
    }
}

void __cdecl Console::Write(_In_ LPCWSTR wzFormat, ...)
{
    va_list args;

    va_start(args, wzFormat);
    Write(wzFormat, args);
    va_end(args);
}

void __cdecl Console::Write(_In_ const std::wstring& value)
{
    Write(value.c_str(), NULL);
}

void __cdecl Console::WriteLine(_In_ LPCWSTR wzFormat, ...)
{
    if (wzFormat)
    {
        va_list args;

        va_start(args, wzFormat);
        Write(wzFormat, args);
        va_end(args);
    }

    Write(L"\n", NULL);
}

void __cdecl Console::WriteLine(_In_ const std::wstring& value)
{
    Write(L"%ls\n", value.c_str());
}

void Console::Write(_In_ LPCWSTR wzFormat, va_list args)
{
	if (m_args.get_IsUtf8Output())
	{
		auto wlen = ::_vscwprintf_p(wzFormat, args);
		std::vector<wchar_t> wbuf(wlen + 1, 0);
		::_vsnwprintf_s(wbuf.data(), wlen + 1, _TRUNCATE, wzFormat, args);
		auto mstr = WcsToMbsString(wbuf.data(), wlen, CP_UTF8);
		printf("%s", mstr.c_str());
	}
	else {
		::_vwprintf_p(wzFormat, args);
	}
}

bool Console::IsConsole(_In_ FILE* f) const noexcept
{
    auto fno = ::_fileno(f);
    auto hFile = (HANDLE)::_get_osfhandle(fno);
    auto dwType = ::GetFileType(hFile);

    dwType &= ~FILE_TYPE_REMOTE;

    if (FILE_TYPE_CHAR != dwType)
    {
        return false;
    }

    DWORD dwMode;
    if (!::GetConsoleMode(hFile, &dwMode))
    {
        return false;
    }

    return true;
}
