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
#include "DbgEngWrapper.h"

#include "out.hpp"

//ULONG64 g_TraceFrom[3];

DbgEngWrapper::DbgEngWrapper()
{

}

DbgEngWrapper::~DbgEngWrapper()
{

}

void DbgEngWrapper::Exit(int Code, PCSTR Format, ...)
{

	HRESULT status = m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,"q",DEBUG_EXECUTE_ECHO);

    // Clean up any resources.
    if (m_Symbols != NULL)
    {
        m_Symbols->Release();
    }
    if (m_Control != NULL)
    {
        m_Control->Release();
    }
    if (m_Client != NULL)
    {
        //
        // Request a simple end to any current session.
        // This may or may not do anything but it isn't
        // harmful to call it.
        //

        // We don't want to see any output from the shutdown.
        m_Client->SetOutputCallbacks(NULL);
        
        m_Client->EndSession(DEBUG_END_PASSIVE);
        
        m_Client->Release();
    }

    // Output an error message if given.
    if (Format != NULL)
    {
        va_list Args;

        va_start(Args, Format);
        vfprintf(stderr, Format, Args);
        va_end(Args);
    }
    
    exit(Code);
}

void DbgEngWrapper::Init()
{
	CreateInterfaces();

}

void DbgEngWrapper::CreateInterfaces(void)
{
    HRESULT Status;

    // Start things off by getting an initial interface from
    // the engine.  This can be any engine interface but is
    // generally IDebugClient as the client interface is
    // where sessions are started.
    if ((Status = DebugCreate(__uuidof(IDebugClient),
                              (void**)&m_Client)) != S_OK)
    {
        Exit(1, "DebugCreate failed, 0x%X\n", Status);
    }

    // Query for some other interfaces that we'll need.
    if ((Status = m_Client->QueryInterface(__uuidof(IDebugControl),
                                           (void**)&m_Control)) != S_OK ||
        (Status = m_Client->QueryInterface(__uuidof(IDebugSymbols),
                                           (void**)&m_Symbols)) != S_OK)
    {
        Exit(1, "QueryInterface failed, 0x%X\n", Status);
    }

	// Install output callbacks so we get any output that the
	// later calls produce.
	if ((Status = m_Client->SetOutputCallbacks(&g_OutputCb)) != S_OK)
	{
		Exit(1, "SetOutputCallbacks failed, 0x%X\n", Status);
	}


}


bool DbgEngWrapper::SetImagePath(const char* pcPath)
{
	HRESULT status;
	if ((status = m_Symbols->SetImagePath(pcPath)) != S_OK)
	{
		//Exit(1, "SetImagePath failed, 0x%X\n", status);
		return false;
	}

	return true;
}

bool DbgEngWrapper::SetSymbolPath(const char* pcPath)
{
	HRESULT status;
	if (pcPath != NULL)
	{
		if ((status = m_Symbols->SetSymbolPath(pcPath)) != S_OK)
		{
			//Exit(1, "SetSymbolPath failed, 0x%X\n", Status);
			return false;
		}
	}

	return true;
}

bool DbgEngWrapper::ExecuteCommand(const char* pcCommand)
{
	HRESULT status = m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,pcCommand,DEBUG_EXECUTE_ECHO);

	if (status != S_OK)
		return false;

	return true;
}

void DbgEngWrapper::DumpAllStacks(void)
{
	ExecuteCommand("~* k");
}

bool DbgEngWrapper::OpenDumpFile(const char* pcDumpFile)
{
	// Everything's set up so open the dump file.
	HRESULT status;
    if ((status = m_Client->OpenDumpFile(pcDumpFile)) != S_OK)
    {
        //Exit(1, "OpenDumpFile failed, 0x%X\n", Status);
		return false;
    }

	// Finish initialization by waiting for the event that
	// caused the dump.  This will return immediately as the
	// dump file is considered to be at its event.
	if ((status = m_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
		INFINITE)) != S_OK)
	{
		//Exit(1, "WaitForEvent failed, 0x%X\n", status);
		return false;
	}

	return true;
}

bool DbgEngWrapper::AttachToProcess_NoneInvasive(unsigned int pid)
{
	HRESULT status;
	if ((status = m_Client->AttachProcess(0,pid,DEBUG_ATTACH_NONINVASIVE)) != S_OK)
	{
		//Exit(1, "AttachProcess failed, 0x%X\n", Status);
		return false;
	}

	//m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,".attach -f 5976",DEBUG_EXECUTE_ECHO);


	// Finish initialization by waiting for the event that
	// caused the dump.  This will return immediately as the
	// dump file is considered to be at its event.
	if ((status = m_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
		INFINITE)) != S_OK)
	{
		//Exit(1, "WaitForEvent failed, 0x%X\n", Status);
		return false;
	}

	return true;

}

bool DbgEngWrapper::GenerateMiniDump(const char* pcDumpFile)
{
	char pcCommand[1024];
	sprintf(pcCommand,".dump /ma /o %s",pcDumpFile);
	ExecuteCommand(pcCommand);
	return true;
}

bool DbgEngWrapper::AttachAndDump(unsigned int pid, const char* pcDumpFile)
{
	AttachToProcess_NoneInvasive(pid);
	GenerateMiniDump(pcDumpFile);

	return true;
}

/*void DbgEngWrapper::ApplyCommandLineArguments(void)
{

	if ((Status = m_Client->AttachProcess(0,2972,DEBUG_ATTACH_NONINVASIVE)) != S_OK)
    {
        Exit(1, "AttachProcess failed, 0x%X\n", Status);
    }

	//m_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,".attach -f 5976",DEBUG_EXECUTE_ECHO);


    // Finish initialization by waiting for the event that
    // caused the dump.  This will return immediately as the
    // dump file is considered to be at its event.
    if ((Status = m_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
                                          INFINITE)) != S_OK)
    {
        Exit(1, "WaitForEvent failed, 0x%X\n", Status);
    }

    // Everything is now initialized and we can make any
    // queries we want.
}*/


/*void DbgEngWrapper::DumpStack(void)
{
    HRESULT Status;
    PDEBUG_STACK_FRAME Frames = NULL;
    int Count = 50;

    printf("\nFirst %d frames of the call stack:\n", Count);

    if (g_TraceFrom[0] || g_TraceFrom[1] || g_TraceFrom[2])
    {
        ULONG Filled;
        
        Frames = new DEBUG_STACK_FRAME[Count];
        if (Frames == NULL)
        {
            Exit(1, "Unable to allocate stack frames\n");
        }
        
        if ((Status = m_Control->
             GetStackTrace(g_TraceFrom[0], g_TraceFrom[1], g_TraceFrom[2],
                           Frames, Count, &Filled)) != S_OK)
        {
            Exit(1, "GetStackTrace failed, 0x%X\n", Status);
        }

        Count = Filled;
    }
    
	


    // Print the call stack.
    if ((Status = m_Control->
         OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, Frames,
                          Count, DEBUG_STACK_SOURCE_LINE |
                          DEBUG_STACK_FRAME_ADDRESSES |
                          DEBUG_STACK_COLUMN_NAMES |
                          DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
    {
        Exit(1, "OutputStackTrace failed, 0x%X\n", Status);
    }

    delete[] Frames;
}*/

/*void DbgEngWrapper::ParseCommandLine(int Argc, __in_ecount(Argc) char** Argv)
{
    int i;
    
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-a32"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a32 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                int Addr;
                
                Argv++;
                Argc--;

                if (sscanf_s(*Argv, "%i", &Addr) == EOF)
                {
                    Exit(1, "-a32 illegal argument type\n");
                }

                g_TraceFrom[i] = (ULONG64)(LONG64)(LONG)Addr;
            }
        }
        else if (!strcmp(*Argv, "-a64"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a64 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                Argv++;
                Argc--;

                if (sscanf_s(*Argv, "%I64i", &g_TraceFrom[i]) == EOF)
                {
                    Exit(1, "-a64 illegal argument type\n");
                }
            }
        }
        else if (!strcmp(*Argv, "-i"))
        {
            if (Argc < 2)
            {
                Exit(1, "-i missing argument\n");
            }

            Argv++;
            Argc--;

            g_ImagePath = *Argv;
        }
        else if (!strcmp(*Argv, "-y"))
        {
            if (Argc < 2)
            {
                Exit(1, "-y missing argument\n");
            }

            Argv++;
            Argc--;

            g_SymbolPath = *Argv;
        }
        else if (!strcmp(*Argv, "-z"))
        {
            if (Argc < 2)
            {
                Exit(1, "-z missing argument\n");
            }

            Argv++;
            Argc--;

            g_DumpFile = *Argv;
        }
        else
        {
            Exit(1, "Unknown command line argument '%s'\n", *Argv);
        }
    }

    if (g_DumpFile == NULL)
    {
        Exit(1, "No dump file specified, use -z <file>\n");
    }
}*/