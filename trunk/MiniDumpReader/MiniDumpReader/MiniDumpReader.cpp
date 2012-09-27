//----------------------------------------------------------------------------
//
// Simple example of how to open a dump file and get its stack.
//
// This is not a debugger extension.  It is a tool that can be used to replace
// the debugger.
//
//
// Copyright (C) Microsoft Corporation, 2000.
//
//----------------------------------------------------------------------------
#include "stdafx.h"
#include "MiniDumper.h"
#include "DbgEngWrapper.h"
#include <assert.h>
#include <iostream>


// You need to use "SetUnhandledExceptionFilter( CustomExceptionFilter );" to make this active
LONG __stdcall CustomExceptionFilter( EXCEPTION_POINTERS* pep ) 
{
	MiniDumper miniDumper;

	miniDumper.GenerateMiniDump(L"c:/my_minidump.dbg",pep);

	return EXCEPTION_EXECUTE_HANDLER; 
}


//#define COL_1 (BACKGROUND_RED | BACKGROUND_INTENSITY)
#define COL_1 (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COL_2 (BACKGROUND_GREEN | BACKGROUND_INTENSITY)

#define CONSOLE_COLOR(x) {SetConsoleTextAttribute(hOut, x);}


////////////////////////////////////////////////////////////////
// IMPORTANT - READ THIS BEFORE YOU CHANGE ANYTHING HERE !!!!!!!
// Any change in the output generated in this function
// might break DumpTextParser.py!!!!!
////////////////////////////////////////////////////////////////

void ReadDump(const char* pcDumpFile, const char* pcAddToSymbolPath,bool bConsole)
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	assert(pcDumpFile);
	DbgEngWrapper dbgEngWrapper;

	dbgEngWrapper.Init();

	//dbgEngWrapper.SetSymbolPath("c:/");
	
	char exec_command[MAX_PATH*2];

	//dbgEngWrapper.ExecuteCommand(".sympath\n");
	
	//SRV*your local symbol folder*http://msdl.microsoft.com/download/symbols 
	const char* pcSymbolServer = "srv*\\\\lucidfs\\public\\RE_PlayGround\\symstore*http://msdl.microsoft.com/download/symbols";
	sprintf(exec_command,".sympath+ %s\n",pcSymbolServer);
	
	dbgEngWrapper.ExecuteCommand(exec_command);
	//dbgEngWrapper.ExecuteCommand(".sympath+ srv*\\\\lucidfs\\public\\re_playground\\symstore");


	if (pcAddToSymbolPath)
	{		
		sprintf(exec_command,".sympath+ %s\n",pcAddToSymbolPath);
		dbgEngWrapper.ExecuteCommand(exec_command);
	}

	dbgEngWrapper.ExecuteCommand(".lines -e\n");

	/*dbgEngWrapper.ExecuteCommand("!sym noisy\n");

	printf("---===Displaying symbol path current state===---\n");
	dbgEngWrapper.ExecuteCommand(".sympath\n");
	printf("---===                Done                ===---\n");*/


	//temp console for debugging

	CONSOLE_COLOR(COL_2);
	printf("{m}/--\\{m} Opening Dump File\n");
	CONSOLE_COLOR(COL_1);
	dbgEngWrapper.OpenDumpFile(pcDumpFile);
	CONSOLE_COLOR(COL_2);
	printf("{m}/--\\{m} Done\n");
	
	if (bConsole)
	{
		printf("{m}/--\\{m} Starting Console\n");
		bool bContinue = true;

		char text[MAX_PATH*2];
		while (bConsole && bContinue)
		{		
			CONSOLE_COLOR(COL_2);
			fputs("{m}/--\\{m}>> ", stdout);
			CONSOLE_COLOR(COL_1);
			fflush(stdout); /* http://c-faq.com/stdio/fflush.html */
			fgets(text, sizeof text, stdin);

			int iRes = strcmp(text,"quit\n");
			if (0 == iRes)
				break;

			dbgEngWrapper.ExecuteCommand(text);			
		}
		CONSOLE_COLOR(COL_2);
		printf("{m}/--\\{m} Done\n");
	}
	
	printf("{m}/--\\{m} Analyzing\n");

	CONSOLE_COLOR(COL_1);
	dbgEngWrapper.ExecuteCommand("!analyze -v\n");

	CONSOLE_COLOR(COL_2);
	printf("{m}/--\\{m} Done\n");

	printf("{m}/--\\{m} Dumping All Stacks\n");
	CONSOLE_COLOR(COL_1);
	dbgEngWrapper.DumpAllStacks();
	CONSOLE_COLOR(COL_2);
	printf("\n{m}/--\\{m} Done\n");
	printf("{m}/--\\{m} Displaying thread times\n");
	CONSOLE_COLOR(COL_1);
	dbgEngWrapper.ExecuteCommand("!runaway");
	CONSOLE_COLOR(COL_2);
	printf("\n{m}/--\\{m} Done\n");
	printf("\n"); // do not remove
	dbgEngWrapper.Exit(0, NULL);
}

// Arguments:
// MiniDumpReader.exe [dump_file.dmp] [path to add to sympath]
// Note - _NT_SYMBOL_PATH should be set to:
// srv*\\lucidfs\public\RE_PlayGround\symstore;\\lucidfs\public\RE_PlayGround\symstore\lucid*http://msdl.microsoft.com/download/symbols
int __cdecl main(int Argc, __in_ecount(Argc) char** Argv)
{
	//SetUnhandledExceptionFilter( CustomExceptionFilter ); 
	/*int* pI = (int*) 0x0;
	*pI = 15;*/
	bool bAllowConsole = false;

	if (Argc > 1)
	{
		if (strcmp(Argv[1],"-console")==0)
			bAllowConsole = true;
	}

	if (Argc < 2 || (bAllowConsole && Argc < 3))
	{
		printf("No Dump File Provided !\n");
		printf("DumpReader.exe [dump_file.dmp] [path to add to sympath]\n");
		return 1;
	}

	unsigned int uiBaseIndex = 0;

	if (bAllowConsole)
		uiBaseIndex++;

	const char* pcAddToSymbolPath = NULL;

	if (Argc > 2)
		pcAddToSymbolPath = Argv[uiBaseIndex+2];

	ReadDump(Argv[uiBaseIndex+1],pcAddToSymbolPath,bAllowConsole);

	printf("Done.");
}
