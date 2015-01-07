#include "stdio.h"
#include "windows.h"
#include <tlhelp32.h>

void StringCopy()
{
	char *szSource = "Hello World !";
	char szTarget[MAX_PATH];

	strcpy(szTarget, szSource);
	printf("%s\n",szTarget);

	strlwr(szTarget);
	printf("b %s\n",szTarget);
}

void StringCat()
{
	char szMalware[MAX_PATH];
	GetSystemDirectory(szMalware, MAX_PATH);
	strcat(szMalware,"\\explorer.dll");

	printf("szMalware: %s\n", szMalware);

	char *p = strrchr(szMalware, '\\');
	printf("p: %s\n", p + 1);

	char *pp = strchr(szMalware, '\\');
	printf("pp: %s\n", pp);
}

int main(int argc, char *argv[])
{
	StringCopy();

	StringCat();
	
	return 0;
}