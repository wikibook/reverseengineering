#include <windows.h>
#include <stdio.h>

int Plus(int a, int b)
{
	return a + b;
}
__declspec(naked) PlusAsm(int a, int b)
{
	__asm
	{
		mov ebx, dword ptr ss:[esp+8]
		mov edx, dword ptr ss:[esp+4]
		add edx, ebx
		mov eax, edx
		retn
	}
}
void main(int argc, char *argv[])
{
	int value = Plus(3,4);
	printf("value: %d\n", value);

	int value2 = PlusAsm(3,4);
	printf("value2: %d\n", value2);

	return;
}