// l_WatchDog.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "DbgEngWrapper.h"

#define  PIPE_NAME "\\\\.\\pipe\\pipe_dweller2"

#include <dbghelp.h>
#include <stdio.h>
#include <crtdbg.h>

#pragma comment ( lib, "dbghelp.lib" )


// Always check there isn't a mismatch between this and the one in ExceptionHandler !!
#define pipe_buff_size sizeof(CONTEXT)+sizeof(EXCEPTION_RECORD)+sizeof(DWORD)

char g_Buff[pipe_buff_size];

//_EXCEPTION_POINTERS g_Pep;
CONTEXT*			g_pContextRecord = NULL;
EXCEPTION_RECORD*	g_pExceptionRecord = NULL;
DWORD				g_dwThreadIdCausingTheException = -1;

void AttachAndGenerateMiniDump(unsigned int pid)
{
	DbgEngWrapper dbgEng;

	//printf("Initing dbgeng\n");
	dbgEng.Init();

	//printf("Done Initing dbgeng\n");


	//printf("Attach+Dump\n");

	dbgEng.AttachAndDump(pid, "c:/unhandled_exception.dmp");

	dbgEng.Exit(0,"Done dbgeng session.\n");

	//printf("Done Attach+Dump\n");
}

void CreateExceptionDescription(CONTEXT& context, EXCEPTION_RECORD& except_rec)
{

	printf("Exception Code: 0x%8X [");

	switch(except_rec.ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		printf("EXCEPTION_ACCESS_VIOLATION");		
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		printf("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");		
		break;
	case EXCEPTION_BREAKPOINT:
		printf("EXCEPTION_BREAKPOINT");		
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		printf("EXCEPTION_DATATYPE_MISALIGNMENT");		
		break;
	case EXCEPTION_FLT_DENORMAL_OPERAND:
		printf("EXCEPTION_FLT_DENORMAL_OPERAND");		
		break;
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		printf("EXCEPTION_FLT_DIVIDE_BY_ZERO");		
		break;
	case EXCEPTION_FLT_INEXACT_RESULT:
		printf("EXCEPTION_FLT_INEXACT_RESULT");		
		break;
	case EXCEPTION_FLT_INVALID_OPERATION:
		printf("EXCEPTION_FLT_INVALID_OPERATION");		
		break;
	case EXCEPTION_FLT_OVERFLOW:
		printf("EXCEPTION_FLT_OVERFLOW");		
		break;
	case EXCEPTION_FLT_STACK_CHECK:
		printf("EXCEPTION_FLT_STACK_CHECK");		
		break;
	case EXCEPTION_FLT_UNDERFLOW:
		printf("EXCEPTION_FLT_UNDERFLOW");		
		break;
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		printf("EXCEPTION_ILLEGAL_INSTRUCTION");		
		break;
	case EXCEPTION_IN_PAGE_ERROR:
		printf("EXCEPTION_IN_PAGE_ERROR");		
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		printf("EXCEPTION_INT_DIVIDE_BY_ZERO");		
		break;
	case EXCEPTION_INT_OVERFLOW:
		printf("EXCEPTION_INT_OVERFLOW");		
		break;
	case EXCEPTION_INVALID_DISPOSITION:
		printf("EXCEPTION_INVALID_DISPOSITION");		
		break;
	case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		printf("EXCEPTION_NONCONTINUABLE_EXCEPTION");		
		break;
	case EXCEPTION_PRIV_INSTRUCTION:
		printf("EXCEPTION_PRIV_INSTRUCTION");		
		break;
	case EXCEPTION_SINGLE_STEP:
		printf("EXCEPTION_SINGLE_STEP");		
		break;
	case EXCEPTION_STACK_OVERFLOW:
		printf("EXCEPTION_STACK_OVERFLOW");		
		break;

	default:
		break;
	}
	printf("] ");

	// what about 64 bit ? will we see the process Inside the wow64 ? or the outside virtual 64

	DWORD  exception_address = (DWORD) except_rec.ExceptionAddress;

	printf("At address 0x%8X ",exception_address);

	if (0 == except_rec.ExceptionFlags)
		printf(" continuable exception");
	else if (EXCEPTION_NONCONTINUABLE == except_rec.ExceptionFlags)
		printf(" continuable exception");
	else if (EXCEPTION_NONCONTINUABLE_EXCEPTION == except_rec.ExceptionFlags)
		printf(" EXCEPTION_NONCONTINUABLE_EXCEPTION");
	else
		printf("exception_flag = 0x%8X",except_rec.ExceptionFlags);

	printf("\n");
}




bool ReadExceptionDataFromNamedPipe()
{
	HANDLE		hIn;
	DWORD		dwBytesRead;

	while (WaitNamedPipeA(PIPE_NAME, NMPWAIT_WAIT_FOREVER) == 0)
	{
		printf("watchdog: still waiting... \n");
		Sleep(5000);
	}

	
	//printf("l_watchdog:  pwrite: the pipe is ready\n");

	LPOFSTRUCT lpofstruct;
	ZeroMemory(&lpofstruct,sizeof(LPOFSTRUCT));


	//printf("l_watchdog: Trying to open file...\n");


	hIn = CreateFileA(PIPE_NAME,
		GENERIC_READ,
		0,
		NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hIn == INVALID_HANDLE_VALUE)
	{
		//printf("CreateFile failed with error %d\n", GetLastError());
		return false;
	}


	//printf("l_watchdog: Done Opening File.\n");

	
	printf("Starting read from pipe\n");

	
	DWORD dwReadSoFar = 0;
	BOOL bRead = false;

	while (1)
	{
		dwBytesRead = 0;
		bRead = ReadFile(hIn, &g_Buff[dwReadSoFar], pipe_buff_size-dwReadSoFar, &dwBytesRead, NULL);

		dwReadSoFar+= dwBytesRead;

		if (!bRead)
		{			
			printf("l_WatchDog:  ReadFile failed -- probably EOF\n");
			break;
		}

		if (pipe_buff_size == dwReadSoFar)
		{
			printf("Done read from pipe\n");
			break;
		}
	}

	

	// The information was sent [CONTEXT,EXCEPTION_RECORD]
	unsigned int uiCurrPos = 0;

	g_pContextRecord				= (CONTEXT*)			&g_Buff[uiCurrPos];
	uiCurrPos+= sizeof(CONTEXT);

	g_pExceptionRecord				= (EXCEPTION_RECORD*)	&g_Buff[uiCurrPos];
	uiCurrPos+= sizeof(EXCEPTION_RECORD);

	g_dwThreadIdCausingTheException = *(DWORD*)				&g_Buff[uiCurrPos];
	uiCurrPos+= sizeof(DWORD);

	CreateExceptionDescription(*g_pContextRecord,*g_pExceptionRecord);

	printf("--------------------------------------------\n");

	//printf("exception Code:0x%X Address:0x%X\n",
		//g_pExceptionRecord->ExceptionCode,
		//g_pExceptionRecord->ExceptionAddress);

	//printf("Context: eax=0x%X flags=0x%X\n",
		//g_pContextRecord->Eax,
		//g_pContextRecord->ContextFlags);

	//printf("--------------------------------------------\n");

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// This function determines whether we need data sections of the given module 
//

bool IsDataSectionNeeded( const WCHAR* pModuleName ) 
{
	// Check parameters 

	if( pModuleName == 0 ) 
	{
		_ASSERTE( _T("Parameter is null.") ); 
		return false; 
	}


	// Extract the module name 

	WCHAR szFileName[_MAX_FNAME] = L""; 

	_wsplitpath( pModuleName, NULL, NULL, szFileName, NULL ); 


	// Compare the name with the list of known names and decide 

	// Note: For this to work, the executable name must be "mididump.exe"
	if( wcsicmp( szFileName, L"mididump" ) == 0 ) 
	{
		return true; 
	}
	else if( wcsicmp( szFileName, L"ntdll" ) == 0 ) 
	{
		return true; 
	}


	// Complete 

	return false; 

}

///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback 
//

BOOL CALLBACK MyMiniDumpCallback(
								 PVOID                            pParam, 
								 const PMINIDUMP_CALLBACK_INPUT   pInput, 
								 PMINIDUMP_CALLBACK_OUTPUT        pOutput 
								 ) 
{
	BOOL bRet = FALSE; 


	// Check parameters 

	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 


	// Process the callbacks 

	switch( pInput->CallbackType ) 
	{
	case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

	case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

	case ModuleCallback: 
		{
			// Are data sections available for this module ? 

			if( pOutput->ModuleWriteFlags & ModuleWriteDataSeg ) 
			{
				// Yes, they are, but do we need them? 

				if( !IsDataSectionNeeded( pInput->Module.FullPath ) ) 
				{
					wprintf( L"Excluding module data sections: %s \n", pInput->Module.FullPath ); 

					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg); 
				}
			}

			bRet = TRUE; 
		}
		break; 

	case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

	case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

	case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 

	case CancelCallback: 
		break; 
	}

	return bRet; 

}

void CreateMiniDump_Helper(DWORD dwPid)
{
	EXCEPTION_POINTERS pep;
	pep.ContextRecord = g_pContextRecord;
	pep.ExceptionRecord = g_pExceptionRecord;


	EXCEPTION_POINTERS* pPep = &pep;

	// Open the file 

	HANDLE hFile = CreateFile( _T("c:/minidumpwritedump.dmp"), GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		// Create the minidump 

		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = g_dwThreadIdCausingTheException;
		mdei.ExceptionPointers  = pPep;
		mdei.ClientPointers     = FALSE; 

		/*MINIDUMP_CALLBACK_INFORMATION mci; 

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
		mci.CallbackParam       = 0; */

		//"midi dump"
		/*MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
			MiniDumpWithDataSegs | 
			MiniDumpWithHandleData |
			MiniDumpWithFullMemoryInfo | 
			MiniDumpWithThreadInfo | 
			MiniDumpWithUnloadedModules ); */

		//"max dump"
		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | 
			MiniDumpWithFullMemoryInfo | 
			MiniDumpWithHandleData | 
			MiniDumpWithThreadInfo | 
			MiniDumpWithUnloadedModules ); 
		

		DWORD dwProcessID = dwPid;
		HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, FALSE, dwPid );
		
		if( hProcess == NULL ) 
		{
			DWORD ErrCode = GetLastError();
			printf( "OpenProcess failed! (lastError=0x%X)\n",ErrCode); 
			return;
		}

		BOOL rv = MiniDumpWriteDump( /*GetCurrentProcess()*/hProcess, /*GetCurrentProcessId()*/dwProcessID, 
			hFile, mdt, /*NULL */&mdei , 0, 0/*&mci*/ ); 


		if( !rv ) 
			printf( "MiniDumpWriteDump failed. Error: %u \n", GetLastError() ); 
		else 
			printf( "Minidump created.\n" ); 

		// Close the file 

		CloseHandle( hFile ); 

	}
	else 
	{
		printf( "CreateFile for minidump failed. Error: %u \n", GetLastError() ); 
	}
}


void CreateMiniDump(DWORD dwPid)
{
	if (g_pContextRecord && g_pExceptionRecord)
	{
		CreateMiniDump_Helper(dwPid);

	} else
	{
		printf("CreateMiniDump: ""g_pContextRecord && g_pExceptionRecord"" not true!\n");
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	ReadExceptionDataFromNamedPipe();

	if (argc<2)
	{
		// No process ID passed as process parameter !!
		printf("No process ID passed as process parameter\n");
		return 1;
	} 

	int iProcessID = _wtoi(argv[1]);	

	printf("Attaching into process %d\n",iProcessID);

	//AttachAndGenerateMiniDump(iProcessID);
	CreateMiniDump(iProcessID);

	printf("Done, about to return from process.\n");

	return 0;
}

