#ifndef DBG_ENG_WRAPPER
#define DBG_ENG_WRAPPER

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

//todo: move into unicode!

class DbgEngWrapper
{
public:

	DbgEngWrapper();
	~DbgEngWrapper();

	void Init();
		
	bool ExecuteCommand(const char* pcCommand);

	// Attaching/Spawning a process

	bool AttachToProcess_NoneInvasive(unsigned int pid);
	bool GenerateMiniDump(const char* pcDumpFile);

	bool AttachAndDump(unsigned int pid, const char* pcDumpFile);


	// Analysis

	bool SetImagePath(const char* pcPath);
	bool SetSymbolPath(const char* pcPath);

	bool OpenDumpFile(const char* pcDumpFile);

	//void DumpStack(void);
	void DumpAllStacks(void);

	void Exit(int Code, PCSTR Format, ...);


protected:

	void CreateInterfaces(void);
	
	IDebugClient*	m_Client;
	IDebugControl*	m_Control;
	IDebugSymbols*	m_Symbols;
};


#endif