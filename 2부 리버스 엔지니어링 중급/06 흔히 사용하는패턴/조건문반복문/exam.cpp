#include "stdio.h"
#include "windows.h"
#include <tlhelp32.h>

void IfFunction(int z)
{
	DWORD x = z;
	
if (x >0x100 || x < 0x200)
	printf("x > 100\n");
//else if (x < 0x200)
	//printf("x > 100 && x < 200\n");
else
	printf("else");
}

int loop(int c)
{
    int d = 0;    

    for (int i=0; i<=0x100; i++)
    {
		if (i==c)
			return 0;

		printf("loop : %d\n", d);
        d++;
    }

    return c+d;
}

int main(int argc, char *argv[])
{
	loop(1000);
	
	IfFunction(10);
	
	return 0;
}