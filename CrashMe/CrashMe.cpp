// CrashMe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;

#include <Windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Strsafe.h>
#include <windows.h>
#include <stdio.h>
#include <process.h>

int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
	BOOL bMiniDumpSuccessful;
	WCHAR szPath[MAX_PATH];
	WCHAR szFileName[MAX_PATH];
	WCHAR* szAppName = L"CrashMe";
	WCHAR* szVersion = L"v1.0";
	DWORD dwBufferSize = MAX_PATH;
	HANDLE hDumpFile;
	SYSTEMTIME stLocalTime;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	GetLocalTime(&stLocalTime);
	GetTempPath(dwBufferSize, szPath);

	StringCchPrintf(szFileName, MAX_PATH, L"%s%s", szPath, szAppName);
	CreateDirectory(szFileName, NULL);

	StringCchPrintf(szFileName, MAX_PATH, L"%s%s\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
		szPath, szAppName, szVersion,
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
		GetCurrentProcessId(), GetCurrentThreadId());
	hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	printf("szFileName: %s", szFileName);
	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
		hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

	return EXCEPTION_EXECUTE_HANDLER;
}

LONG CALLBACK unhandled_handler(EXCEPTION_POINTERS* e)
{
	GenerateDump(e);
	return EXCEPTION_CONTINUE_SEARCH;
}


void NullPointer()
{
	int *pBadPtr = NULL;
	*pBadPtr = 0;
}

void DoubleFree()
{
	char* pBuffer = new char[32];
	delete[] pBuffer;
	delete[] pBuffer;
}

void DivdeByZero()
{
	int a = 0;
	int b = 5 / a;
}

void StackOverflow()
{
	while (1) {
		StackOverflow();
	}
}

void BuffeeOverflow()
{
	const int ALLOCATION_SIZE = 16;
	char overflow[] = "This buffer size is great then allocation size,it will cuase buffer overflow, and then crash";
	char* pBuffer = new char[ALLOCATION_SIZE];
	//
	// Allocate a buffer and zip past the end of it
	//
	for (int i = 0; i < strlen(overflow); i++) {

		strcpy_s(pBuffer, strlen(overflow), overflow);
	}
}

#define BUFFER_SZIE 64
char* g_pBuf = NULL;
HANDLE g_hExitEvent;
unsigned __stdcall ThreadCreateResourceFunc(void* pArguments)
{
	printf("In ThreadCreateResourceFunc...\n");

	g_pBuf = new char[BUFFER_SZIE];

	WaitForSingleObject(
		g_hExitEvent, // event handle
		INFINITE);    // infinite wait

	delete[] g_pBuf;
	::Sleep(5);
	g_pBuf = NULL;
	printf("delete buffer...\n");
	return 0;
}

unsigned __stdcall ThreadUseResourceFunc(void* pArguments)
{
	printf("In ThreadUseResourceFunc...\n");
	int count = 1;
	while (true) {
		if (g_pBuf) {
			sprintf_s(g_pBuf, BUFFER_SZIE, "fill the buffer with string: %d\n", count);
			printf(g_pBuf);
			count++;
		}
	}

	return 0;
}


void RaceCondition()
{
	HANDLE hHandles[2];
	unsigned threadIDs[2];
	char a;

	g_hExitEvent = CreateEventA(
		NULL,               // default security attributes
		TRUE,               // manual-reset event
		FALSE,              // initial state is nonsignaled
		"ExitEvent"  // object name
		);

	printf("Creating two threads...\n");
	// Create the frist thread.
	hHandles[0] = (HANDLE)_beginthreadex(NULL, 0, &ThreadCreateResourceFunc, NULL, NULL, &threadIDs[0]);
	// Create the second thread.
	hHandles[1] = (HANDLE)_beginthreadex(NULL, 0, &ThreadUseResourceFunc, NULL, CREATE_SUSPENDED, &threadIDs[1]);

	cin >> a;
	printf("Resume worker threads...\n");
	ResumeThread(hHandles[1]);

	cin >> a;
	SetEvent(g_hExitEvent);

	WaitForMultipleObjects(2, hHandles, true, INFINITE);
	// Destroy the thread object.
	CloseHandle(hHandles[0]);
	CloseHandle(hHandles[1]);
}



typedef enum CrashCase {
	NULL_POINTER = 1,
	DIVIDE_BY_ZERO,
	STACK_OVERFLOW,
	DOUBLE_FREE,
	BUFFER_OVERFLOW,
	RACE_CONDITION,
	FIXED_RACE_CONDITION
};




int _tmain(int argc, _TCHAR* argv[])
{

	SetUnhandledExceptionFilter(unhandled_handler);

	int i;
	cout << "Please choose the crash case:\n \
		\t1)Access Null pointer\n \
		\t2)Divide By Zero\n \
		\t3)Stack Overflow\n \
		\t4)Duble free\n \
		\t5)Buffer Overflow\n \
		\t6)Race Condition\n \
		\t7)Fixed Race Condition\n";
	cin >> i;
	cout << "The value you entered is " << i <<endl;

	switch (i) {
	case NULL_POINTER:
		NullPointer();
		break;
	case DIVIDE_BY_ZERO:
		DivdeByZero();
		break;
	case STACK_OVERFLOW:
		StackOverflow();
		break;
	case DOUBLE_FREE:
		DoubleFree();
		break;
	case BUFFER_OVERFLOW:
		BuffeeOverflow();
		break;
	case RACE_CONDITION:
		RaceCondition();
		break;
	case FIXED_RACE_CONDITION:

		break;
	}

	return 0;
}

