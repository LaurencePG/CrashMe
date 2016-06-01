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

typedef enum CrashCase {
	NULL_POINTER = 1,
	DIVIDE_BY_ZERO,
	STACK_OVERFLOW,
	DOUBLE_FREE,
	BUFFER_OVERFLOW
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
		\t5)Buffer Overflow\n";
	cin >> i;
	cout << "The value you entered is " << i;

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
	}

	return 0;
}

