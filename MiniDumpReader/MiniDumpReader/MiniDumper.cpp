#include "stdafx.h"
#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
#include <crtdbg.h>
#include "MiniDumper.h"

#pragma comment ( lib, "dbghelp.lib" )


// Do we need this module data? (is it really used by the process?)

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



// MiniDump creation callback
// This is called back to us when "MiniDumpWriteDump" is called, in order to ask us what info we want inside the dump

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



///////////////////////////////////////////////////////////////////////////////
// Function declarations 
//


MiniDumper::MiniDumper()
{

}

MiniDumper::~MiniDumper()
{

}

bool MiniDumper::GenerateMiniDump(const WCHAR* pcDumpName,EXCEPTION_POINTERS* pep)
{
	// Open the file 

	HANDLE hFile = CreateFile( pcDumpName, GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		// Create the minidump 

		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 

		MINIDUMP_CALLBACK_INFORMATION mci; 

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
		mci.CallbackParam       = 0; 

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | 
			MiniDumpWithDataSegs | 
			MiniDumpWithHandleData |
			MiniDumpWithFullMemoryInfo | 
			MiniDumpWithThreadInfo | 
			MiniDumpWithUnloadedModules ); 

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

		if( !rv ) 
			_tprintf( _T("MiniDumpWriteDump failed. Error: %u \n"), GetLastError() ); 
		else 
			_tprintf( _T("Minidump created.\n") ); 

		// Close the file 

		CloseHandle( hFile ); 
		return true;

	}
	else 
	{
		_tprintf( _T("CreateFile failed. Error: %u \n"), GetLastError() ); 
	}

	return false;
}

/*
///////////////////////////////////////////////////////////////////////////////
// Test data and code 
//

struct A 
{
	int a; 

	A() 
		: a( 0 ) {}

	void Print() 
	{
		_tprintf( _T("a: %d\n"), a ); 
	}
};

struct B 
{
	A* pA; 

	B() 
		: pA( 0 ) {}

	void Print() 
	{
		_tprintf( _T("pA: %x\n"), pA ); 
		pA->Print(); 
	}

};

void DoWork() 
{
	B* pB = new B(); // but forget to initialize B::pA 

	pB->Print(); // here it should crash 
}


///////////////////////////////////////////////////////////////////////////////
// main() function 
//

int main( int argc, char* argv[] ) 
{
	__try 
	{
		DoWork(); 
	}
	__except( CreateMiniDump( GetExceptionInformation() ), EXCEPTION_EXECUTE_HANDLER ) 
	{
	}

	return 0; 
}




*/