#ifndef SUBPROCESS_H_
#define SUBPROCESS_H_

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h> 

#ifdef __unix__


#else

#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

using namespace std;

class SubProcess
{
private:
	HANDLE _process = NULL;
	HANDLE _writeHandler = NULL;
	HANDLE _readHandler = NULL;

	void print_error(const char* message)
	{
		LPVOID lpvMessageBuffer;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpvMessageBuffer, 0, NULL);
		fprintf(stderr, "%s: %s\n", message, lpvMessageBuffer);
		LocalFree(lpvMessageBuffer);
	}

public:
	explicit SubProcess(const string& path, const vector<string>& args = vector<string>())
	{
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;
		if (!CreatePipe(&_readHandler, &_writeHandler, &saAttr, 0))
			print_error("CreatePipe");
		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.cb = sizeof(si);
        si.hStdError = _writeHandler;
        si.hStdOutput = _writeHandler;
        si.hStdInput = _readHandler;
        si.dwFlags |= STARTF_USESTDHANDLES;
		si.wShowWindow = SW_HIDE;
		string str = path;
		for (const string& arg : args)
			str += " " + arg;
		const char* cstr = str.c_str();
		wchar_t wc[1024];
		size_t n;
		mbstowcs_s(&n, wc, cstr, strlen(cstr));
		if (!CreateProcess(NULL, wc, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
			print_error("CreateProcess");
		_process = pi.hProcess;
        if (!CloseHandle(pi.hThread))
			print_error("CloseHandle");
	}

	~SubProcess()
	{
		close();
	}

	void close()
	{
		WaitForSingleObject(_process, INFINITE);
		CloseHandle(_readHandler);
		CloseHandle(_writeHandler);
		DWORD exitCode;
		GetExitCodeProcess(_process, &exitCode);
	}

	bool write(const char* buf, const int& size_to_write) const
	{
		if (WaitForSingleObject(_process, 1) == WAIT_OBJECT_0) return false;
		int size = 0;
		while (size < size_to_write)
		{
			DWORD s;
			if (!WriteFile(_writeHandler, buf + size, size_to_write - size, &s, NULL)) return false;
			size += s;
		}
		return true;
	}

	int read(char* buf, const int& size_to_read) const
	{
		int size = 0;
		DWORD total;
		while (size < size_to_read)
		{
			if (WaitForSingleObject(_process, 1) == WAIT_OBJECT_0) return 0;
			if (PeekNamedPipe(_readHandler, NULL, 0, NULL, &total, NULL) == 0) continue;
			DWORD s = 0;
			auto success = ReadFile(_readHandler, buf + size, size_to_read - size, &s, NULL);
			if (!success) return false;
			size += s;
		}
		return size;
	}

};

#endif

#endif
