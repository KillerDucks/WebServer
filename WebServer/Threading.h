#pragma once

#include <windows.h>

class Threading
{
public:
	// Structs
	typedef struct ThreadData
	{
		DWORD	threadID;
		char*	port;
	} THREADDATA, *PTHREADDATA;

	Threading(int threads);		// Create Threads
	~Threading();				// Close all handles to threads
};

