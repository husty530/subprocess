#ifndef SUBPROCESS_H_
#define SUBPROCESS_H_

#include <iostream>
#include <vector>
#include <stdio.h> 

using namespace std;

#ifdef __unix__

#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

class SubProcess
{
private:
	int _fdr;
	int _fdw;

public:
	explicit SubProcess(const string& path, const vector<string>& args = vector<string>())
	{
		int pid = 0;
		_fdr = fileno(stdin);
		_fdw = fileno(stdout);
		int c2p[2]; // child -> parent
		int p2c[2]; // parent -> child
		int R = 0;
		int W = 1;
    if (pipe(c2p) < 0) 
		{
			perror("popen2");
			return;
    }
    if (pipe(p2c) < 0) 
		{
			perror("popen2");
			close(c2p[R]);
			close(c2p[W]);
			return;
    }
    if ((pid = fork()) < 0) 
		{
			perror("popen2");
			close(c2p[R]);
			close(c2p[W]);
			close(p2c[R]);
			close(p2c[W]);
			return;
    }
    if (pid == 0) 
		{
			close(p2c[W]);
			close(c2p[R]);
			if (dup2(p2c[R],  STDIN_FILENO) < 0) perror("popen2");
			if (dup2(c2p[W], STDOUT_FILENO) < 0) perror("popen2");
			close(p2c[R]);
			close(c2p[W]);
			char** argv = new char*[args.size() + 2];
			argv[0] = new char[strlen(path.c_str())];
			strcpy(argv[0], path.c_str());
			for (int i = 0; i < (int)args.size(); i++)
			{
				argv[i + 1] = new char[strlen(args[i].c_str())];
				strcpy(argv[i + 1], args[i].c_str());
			}
			argv[(int)args.size() + 1] = new char[1];
			argv[(int)args.size() + 1] = (char*)NULL;
			if (execv(path.c_str(), argv) < 0) 
			{
				perror("popen2");
				close(p2c[R]);
				close(c2p[W]);
				for (int i = 0; i < args.size() + 2; i++)
					delete[] argv[i];
				delete[] argv;
				return;
			}
		}
		close(p2c[R]);
		close(c2p[W]);
		_fdw = p2c[W];
		_fdr = c2p[R];
	}

	~SubProcess()
	{
		close_();
	}

	void close_()
	{
		close(_fdw);
		close(_fdr);
		_exit(EXIT_SUCCESS);
	}

	bool try_write(const char* buf, const int& size_to_write) const
	{
		int size = 0;
		while (size < size_to_write)
		{
			int s = write(_fdw, buf + size, size_to_write - size);
			if (s <= 0) return false;
			size += s;
		}
		return true;
	}

	int try_read(char* buf, const int& size_to_read) const
	{
		int size = 0;
		while (size < size_to_read)
		{
			int s = read(_fdr, buf + size, size_to_read - size);
			if (s < 0) return false;
			size += s;
		}
		return true;
	}

};

#else

#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <windows.h>

class SubProcess
{
private:
	HANDLE _process = NULL;
	HANDLE p2c = NULL;
	HANDLE c2p = NULL;

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
		if (!CreatePipe(&c2p, &p2c, &saAttr, 0))
			print_error("CreatePipe");
		PROCESS_INFORMATION pi = {};
		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.cb = sizeof(si);
        si.hStdError = p2c;
        si.hStdOutput = p2c;
        si.hStdInput = c2p;
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
		close_();
	}

	void close_()
	{
		WaitForSingleObject(_process, INFINITE);
		CloseHandle(c2p);
		CloseHandle(p2c);
		DWORD exitCode;
		GetExitCodeProcess(_process, &exitCode);
	}

	bool try_write(const char* buf, const int& size_to_write) const
	{
		if (WaitForSingleObject(_process, 10) == WAIT_OBJECT_0) return false;
		int size = 0;
		while (size < size_to_write)
		{
			DWORD s;
			if (!WriteFile(p2c, buf + size, size_to_write - size, &s, NULL)) return false;
			size += s;
		}
		return true;
	}

	bool try_read(char* buf, const int& size_to_read) const
	{
		int size = 0;
		DWORD total;
		while (size < size_to_read)
		{
			if (WaitForSingleObject(_process, 1) == WAIT_OBJECT_0) return 0;
			if (PeekNamedPipe(c2p, NULL, 0, NULL, &total, NULL) == 0) continue;
			DWORD s = 0;
			auto success = ReadFile(c2p, buf + size, size_to_read - size, &s, NULL);
			if (!success) return false;
			size += s;
		}
		return true;
	}

};

#endif

#endif
