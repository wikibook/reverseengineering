#include <stdio.h>
#include <Windows.h>

bool FindDebugger()
{
	bool bDetect = false;
	HWND hDebugger = ::FindWindow(NULL, "OllyDBG");
	if (hDebugger)
	{
		bDetect = true;
		printf("이 프로그램은 디버거와 함께 사용될 수 없습니다\n");
	}
	return bDetect;
}

bool IsSoftIceLoaded()
{
	HANDLE hFile;
	hFile = CreateFile( "\\\\.\\NTICE", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFile);
		return true;
	}

	return false;
}

int main(int argc, char *argv[])
{
	// 1. FindWindow
	FindDebugger();

	IsSoftIceLoaded();

	return 0;
}