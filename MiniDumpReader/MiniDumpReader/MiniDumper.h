#ifndef MINI_DUMPER
#define MINI_DUMPER

#include <tchar.h>
#include "windows.h"

class MiniDumper
{
public:

	MiniDumper();
	~MiniDumper();

	bool GenerateMiniDump(const WCHAR* pcDumpName,EXCEPTION_POINTERS* pep);

};






#endif