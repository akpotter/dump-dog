#ifndef LUCID_EXCEPTION_HANDLER
#define LUCID_EXCEPTION_HANDLER

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>

#define  PIPE_NAME "\\\\.\\pipe\\pipe_dweller2"

HANDLE g_hPipe = 0;


// Always check there isn't a mismatch between this and the one in l_WatchDog !!
#define pipe_buff_size sizeof(CONTEXT)+sizeof(EXCEPTION_RECORD)+sizeof(DWORD)

// (defined lower)
LONG WINAPI YoelUnhandledExceptionFilter(
	_EXCEPTION_POINTERS *pep);

void WriteIntoPipe(_EXCEPTION_POINTERS *pep, DWORD dwThreadID)
{
	ConnectNamedPipe(g_hPipe, NULL);

	char buff[pipe_buff_size];

	unsigned int uiPos = 0;
	memcpy(&buff[uiPos], pep->ContextRecord, sizeof(CONTEXT));
	uiPos+=sizeof(CONTEXT);

	memcpy(&buff[uiPos], pep->ExceptionRecord, sizeof(EXCEPTION_RECORD));
	uiPos+=sizeof(EXCEPTION_RECORD);

	memcpy(&buff[uiPos], &dwThreadID, sizeof(DWORD));
	uiPos+=sizeof(DWORD);

	DWORD dwBytesWritten = 0;
	DWORD dwTotalBytesWrittenSoFar = 0;

	while (1)
	{
		if (!WriteFile(g_hPipe,&buff[dwTotalBytesWrittenSoFar] , pipe_buff_size - dwTotalBytesWrittenSoFar, &dwBytesWritten, NULL))
		{
			printf("ExceptionHandler:  WriteFile failed\n");
			break;
		} 

		DWORD dwBuffSize = pipe_buff_size;
		printf("Wrote %d bytes (pipe_buff_size=%d)",dwBytesWritten,dwBuffSize);

		dwTotalBytesWrittenSoFar += dwBytesWritten;

		if (pipe_buff_size == dwTotalBytesWrittenSoFar)
		{
			printf("Done writing into pipe\n");
			break;
		}
	}
	
	//DWORD threadId = GetCurrentThreadId();

	CloseHandle(g_hPipe);
}

void CreatePipe()
{
	HANDLE		hOut;
	wchar_t		buf[1024];
	DWORD		len;
	DWORD		dwWritten;

	/*printf("ExceptionHandler:  pwrite: waiting for the pipe...\n");
	if (WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER) == 0)
	{
		printf("WaitNamedPipe failed. error=%d\n", GetLastError());
		return;
	}
	printf("ExceptionHandler:  pwrite: the pipe is ready\n");*/

	g_hPipe = CreateNamedPipeA(PIPE_NAME, 	// Name
		PIPE_ACCESS_DUPLEX | WRITE_DAC, // OpenMode
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, // PipeMode
		2, // MaxInstances
		1024, // OutBufferSize
		1024, // InBuffersize
		2000, // TimeOut
		NULL); // Security
	if (g_hPipe == INVALID_HANDLE_VALUE)
	{
		printf("ExceptionHandler:  Could not create the pipe\n");
		exit(1);
	}


	//printf("ExceptionHandler:  hPipe=%p\n", g_hPipe);
}


size_t ExecuteProcess(_EXCEPTION_POINTERS *pep,std::wstring FullPathToExe, std::wstring Parameters, size_t SecondsToWait)
{
	size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
	DWORD dwExitCode = 0;
	std::wstring sTempStr = L"";

	/* - NOTE - You should check here to see if the exe even exists */

	/* Add a space to the beginning of the Parameters */
	if (Parameters.size() != 0)
	{
		if (Parameters[0] != L' ')
		{
			Parameters.insert(0,L" ");
		}
	}

	/* The first parameter needs to be the exe itself */
	sTempStr = FullPathToExe;
	iPos = sTempStr.find_last_of(L"\\");
	sTempStr.erase(0, iPos +1);
	Parameters = sTempStr.append(Parameters);

	/* CreateProcessW can modify Parameters thus we allocate needed memory */
	wchar_t * pwszParam = new wchar_t[Parameters.size() + 1];
	if (pwszParam == 0)
	{
		return 1;
	}
	const wchar_t* pchrTemp = Parameters.c_str();
	wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

	/* CreateProcess API initialization */
	STARTUPINFOW siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);


	CreatePipe();
	
	if (CreateProcessW(const_cast<LPCWSTR>(FullPathToExe.c_str()),
		pwszParam, 0, 0, false,
		CREATE_DEFAULT_ERROR_MODE /*| CREATE_NEW_CONSOLE | CREATE_NO_WINDOW*/, 0, 0,
		&siStartupInfo, &piProcessInfo) != false)
	{
		// Wait for the process to end

		//printf("Creating pipe and sending message...\n");
		WriteIntoPipe(pep, GetCurrentThreadId());
		//printf("Done with pipe creation.\n");

		//printf("ExecutionHandler: Waiting for the process to end...\n");
		dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));
		//printf("ExecutionHandler: Process ended (or timeout met)\n");
	}
	else
	{
		// Create Process failed - oh no...
		// TODO: Create a log file describing this
		iReturnVal = GetLastError();
	}
	
	delete[]pwszParam;
	pwszParam = 0;

	CloseHandle(piProcessInfo.hProcess);
	CloseHandle(piProcessInfo.hThread);

	return iReturnVal;
} 


bool SpawnDumperProcess(_EXCEPTION_POINTERS *pep)
{
	DWORD dwID = GetCurrentProcessId();

	wchar_t processId[16];
	wsprintfW(processId,L"%d",dwID);

	std::wstring exe_path(L"C:/l_WatchDog.exe");
	std::wstring params(processId);

	size_t ret =  ExecuteProcess(pep,exe_path, params, 60*15);

	if (ret==0)
		return true;
	return false;
}

LONG WINAPI YoelUnhandledExceptionFilter(
	_EXCEPTION_POINTERS *pep)
{	
	//MessageBox(0,"Exception Al Halal!","Exception Al Halal!",0);
	//printf ("About to spawn dumping process...\n");
	SpawnDumperProcess(pep);

	//FatalAppExit(0, L"Unhandled exception occured");

	//printf("About to return: EXCEPTION_EXECUTE_HANDLER\n");

	return EXCEPTION_CONTINUE_SEARCH;
	//return EXCEPTION_EXECUTE_HANDLER;
}

#endif