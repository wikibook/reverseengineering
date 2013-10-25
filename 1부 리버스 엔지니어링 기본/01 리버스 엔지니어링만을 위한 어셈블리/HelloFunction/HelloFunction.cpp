#include <windows.h>
#include <stdio.h>

DWORD HelloFunction(DWORD dwParam1, DWORD dwParam2, DWORD dwParam3)
{
	DWORD dwRetAddr = 0;
	__asm
	{
		push eax
		mov eax, [ebp+4]
		mov dwRetAddr, eax
		pop eax
	}
	printf("dwRetAddr: %08x\n", dwRetAddr);

	return 0;
}

void main(int argc, char *argv[])
{
	HelloFunction(1,2,3);

	return;
}