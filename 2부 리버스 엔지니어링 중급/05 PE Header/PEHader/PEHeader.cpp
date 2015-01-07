#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

void ImportDescriptorParser(LPCTSTR szFileName)
{
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	HANDLE hImgMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hImgMap == NULL)
		return;

	PVOID pImgView = MapViewOfFile(hImgMap, FILE_MAP_READ, 0, 0, 0);
	if (pImgView == NULL)
		return;

	PIMAGE_DOS_HEADER pSehIDH = (PIMAGE_DOS_HEADER)pImgView;
	PIMAGE_NT_HEADERS pSehINH = (PIMAGE_NT_HEADERS)( (DWORD)pSehIDH + pSehIDH->e_lfanew ); 
	PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER)&pSehINH->OptionalHeader; 

	PIMAGE_DATA_DIRECTORY pIDD = &pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_SECTION_HEADER pSec = (PIMAGE_SECTION_HEADER)((PBYTE)pIOH + sizeof(IMAGE_OPTIONAL_HEADER));
	PIMAGE_SECTION_HEADER pISH = NULL;

	PIMAGE_FILE_HEADER pIFH = &pSehINH->FileHeader;
	int wNumOfSec = pIFH->NumberOfSections;

	// .idata 섹션이 따로 존재하지 않고 다른 섹션에 포함되어 있는 경우가 많음
	// 그래서 섹션헤더를 하나씩 검사하면서 .idata 섹션의 위치를 구함
	for (int i=0; i<wNumOfSec; ++i)
	{
		if (pIDD->VirtualAddress>=pSec[i].VirtualAddress &&
			pIDD->VirtualAddress<pSec[i].VirtualAddress+pSec[i].Misc.VirtualSize)
		{
			pISH = &pSec[i];
			break;
		}
	}

	if (pISH == NULL)
	{
		printf("== No Imports Table Found ==\r\n");
		return;
	}

	DWORD dwDelta = pISH->VirtualAddress - pISH->PointerToRawData;
	if (pIDD->VirtualAddress-dwDelta >= pIOH->SizeOfImage)
	{
		printf("== No Imports Table Found ==\r\n");
		return;
	}
	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)((PBYTE)pImgView + pIDD->VirtualAddress - dwDelta);

	printf("== Import Table ==\r\n\r\n");

	for (i=0; pIID[i].OriginalFirstThunk||pIID[i].FirstThunk; ++i)
	{
		if (pIID[i].Name-dwDelta < pIOH->SizeOfImage)
		{
			printf("  %s\r\n", (LPCSTR)((PBYTE)pImgView + pIID[i].Name - dwDelta));	
		}

		printf("  OriginalFirstThunk : 0x%X\r\n", pIID[i].OriginalFirstThunk);

		printf("  TimeDateStamp      : 0x%X\r\n", pIID[i].TimeDateStamp);

		printf("  ForwarderChain     : 0x%X\r\n", pIID[i].ForwarderChain);

		printf("  FirstThunk         : 0x%X\r\n", pIID[i].FirstThunk);

		if (pIID[i].OriginalFirstThunk)
		{
			if (pIID[i].OriginalFirstThunk-dwDelta>=pIOH->SizeOfImage ||
				pIID[i].FirstThunk-dwDelta>=pIOH->SizeOfImage)
				goto $end;
			
			PIMAGE_THUNK_DATA32 pOFT = (PIMAGE_THUNK_DATA32)((PBYTE)pImgView + pIID[i].OriginalFirstThunk - dwDelta);
			PIMAGE_THUNK_DATA32 pIAT = (PIMAGE_THUNK_DATA32)((PBYTE)pImgView + pIID[i].FirstThunk - dwDelta);
			for (int j=0; *((PDWORD)pOFT+j); ++j)
			{
				if (*((PDWORD)pOFT+j) & 0x80000000)
				{
					printf("    %4d", *((PDWORD)pOFT+j)&0x0000FFFF);
				}
				else
				{
					if (*((PDWORD)pOFT+j)-dwDelta < pIOH->SizeOfImage)
					{
						PIMAGE_IMPORT_BY_NAME pIIBN = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pImgView + *((PDWORD)pOFT+j) - dwDelta);
						printf("    %4d %s", pIIBN->Hint, pIIBN->Name);
					}
				}
				
				printf(" (IAT: 0x%X)\r\n", pIAT[j].u1.Function);
			}
		}
		else if (pIID[i].FirstThunk)
		{
			if (pIID[i].FirstThunk-dwDelta >= pIOH->SizeOfImage)
				goto $end;

			PIMAGE_THUNK_DATA32 pIAT = (PIMAGE_THUNK_DATA32)((PBYTE)pImgView + pIID[i].FirstThunk - dwDelta);
			for (int j=0; *((PDWORD)pIAT+j); ++j)
			{
				if (*((PDWORD)pIAT+j) & 0x80000000)
				{
					printf("    %4d", *((PDWORD)pIAT+j)&0x0000FFFF);
				}
				else
				{
					if (*((PDWORD)pIAT+j)-dwDelta < pIOH->SizeOfImage)
					{
						PIMAGE_IMPORT_BY_NAME pIIBN = (PIMAGE_IMPORT_BY_NAME)((PBYTE)pImgView + *((PDWORD)pIAT+j) - dwDelta);
						printf("   %4d %s", pIIBN->Hint, pIIBN->Name);
					}
				}
					
				printf(" (IAT: 0x%X)\r\n", pIAT[j].u1.Function);
			}
		}

$end:
		printf("\r\n");
	}
}

void StructureSize()
{
	printf("\n\n\nNT Header size: 0x%x\n", sizeof(IMAGE_NT_HEADERS));
	printf("File Header size: 0x%x\n", sizeof(IMAGE_FILE_HEADER));
	printf("Optional Header size: 0x%x\n", sizeof(IMAGE_OPTIONAL_HEADER32));
}

void PrintTimeStamp()
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD) pDosHeader + pDosHeader->e_lfanew);

	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		printf("This is not PE format.\n");
	}

	TCHAR *szTime;
	szTime = ctime((long *)&pNtHeader->FileHeader.TimeDateStamp);

	DWORD dwLen = _tcslen(szTime);
	szTime[dwLen - 1] = '\0';

	printf(_T("TimeStamps: %s\n"), szTime);
}

void PrintKernel32CodeSize()
{
	HMODULE hKernel32 = GetModuleHandle("kernel32.dll");

	PIMAGE_DOS_HEADER dosHdr = (PIMAGE_DOS_HEADER)hKernel32;
	PIMAGE_NT_HEADERS ntHdrs = (PIMAGE_NT_HEADERS)((DWORD)hKernel32 + dosHdr->e_lfanew);

	DWORD kernel32Start = (DWORD)hKernel32 + ntHdrs->OptionalHeader.BaseOfCode;
	DWORD kernel32End   = kernel32Start + ntHdrs->OptionalHeader.SizeOfCode;

	printf("kernel32.dll code size: %x\n", ntHdrs->OptionalHeader.SizeOfCode);
	printf("kernel32.dll code address : (0x%x ~ 0x%x)\n", kernel32Start, kernel32End);
}

void EnumDataDirectory()
{
	PIMAGE_DOS_HEADER pSehIDH = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS pSehINH = (PIMAGE_NT_HEADERS)( (DWORD)pSehIDH + pSehIDH->e_lfanew ); 
	PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER)&pSehINH->OptionalHeader; 

	LPCSTR pcszIDD[] = {
		"EXPORT        ", "IMPORT        ", "RESOURCE      ",
		"EXCEPTION     ", "SECURITY      ", "BASERELOC     ",
		"DEBUG         ", "ARCHITECTURE  ",	"GLOBALPTR     ",
		"TLS           ", "LOAD_CONFIG   ",	"BOUND_IMPORT  ",
		"IAT           ", "DELAY_IMPORT  ",	"COM_DESCRIPTOR"
	};

	UINT uNumOfIDD = sizeof(pcszIDD) / sizeof(pcszIDD[0]);

	printf("== Data Directory ==\r\n");
	printf("Name                RVA        Size\r\n");
	printf("--------------  ----------  ----------\r\n");

	for (DWORD i=0; i<pIOH->NumberOfRvaAndSizes-1 && i<uNumOfIDD*3; ++i)
	{
		if (uNumOfIDD > i)
			printf("%s  0x%-8X  0x%-8X\r\n", pcszIDD[i],
				pIOH->DataDirectory[i].VirtualAddress, 
				pIOH->DataDirectory[i].Size);
		else
			printf("%s  0x%-8X  0x%-8X\r\n", "Unused        ",
				pIOH->DataDirectory[i].VirtualAddress, 
				pIOH->DataDirectory[i].Size);
	}
}

void SectionHeacerParser()
{
	PIMAGE_DOS_HEADER pSehIDH = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS pSehINH = (PIMAGE_NT_HEADERS)( (DWORD)pSehIDH + pSehIDH->e_lfanew ); 
	PIMAGE_FILE_HEADER pIFH = &pSehINH->FileHeader;
	PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER)&pSehINH->OptionalHeader; 
	PIMAGE_SECTION_HEADER pISH = (PIMAGE_SECTION_HEADER)((PBYTE)pIOH + sizeof(IMAGE_OPTIONAL_HEADER));

	INT iCharacteristics[] = {
		IMAGE_SCN_CNT_CODE, IMAGE_SCN_CNT_INITIALIZED_DATA,
		IMAGE_SCN_CNT_UNINITIALIZED_DATA, IMAGE_SCN_MEM_DISCARDABLE,
		IMAGE_SCN_MEM_NOT_CACHED, IMAGE_SCN_MEM_NOT_PAGED,
		IMAGE_SCN_MEM_SHARED, IMAGE_SCN_MEM_EXECUTE,
		IMAGE_SCN_MEM_READ, IMAGE_SCN_MEM_WRITE
	};
	LPCSTR pcszCharacteristics[] = {
		"CODE", "INITIALIZED_DATA", "UNINITIALIZED_DATA",
		"MEM_DISCARDABLE", "MEM_NOT_CACHED", "MEM_NOT_PAGED",
		"MEM_SHARED", "MEM_EXECUTE", "MEM_READ", "MEM_WRITE"
	};
		
	printf("== Section Table : %d ==\r\n\r\n", pIFH->NumberOfSections);

	for (int i=0; i<pIFH->NumberOfSections; ++i)
	{
		printf("  %02d %s\r\n", i+1, pISH[i].Name);

		printf("      VirtSize        : 0x%-8X  VirtAddr      : 0x%-8X\r\n",
			pISH[i].Misc.VirtualSize, pISH[i].VirtualAddress);

		printf("      raw data offs   : 0x%-8X  raw data size : 0x%-8X\r\n",
			pISH[i].PointerToRawData, pISH[i].SizeOfRawData);

		printf("      relocation offs : 0x%-8X  relocations   : 0x%-8X\r\n",
			pISH[i].PointerToRelocations, pISH[i].NumberOfRelocations);

		printf("      line # offs     : 0x%-8X  line #'s      : 0x%-8X\r\n",
			pISH[i].PointerToLinenumbers, pISH[i].NumberOfLinenumbers);

		printf("      characteristics : 0x%-8X\r\n", pISH[i].Characteristics);

		for (int j=0; j<sizeof(iCharacteristics)/sizeof(INT); ++j)
			if (pISH[i].Characteristics & iCharacteristics[j])
			{
				printf("        %s\r\n", pcszCharacteristics[j]);
			}
	}
}


int main(int argc, char *argv[])
{
	// VirtualProtect 예제.
	DWORD dwOldProtect = 0;
	VirtualProtect((LPVOID)0x401000, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	StructureSize();

	SectionHeacerParser();

	EnumDataDirectory();
	
	PrintKernel32CodeSize();

	PrintTimeStamp();	

	ImportDescriptorParser(argv[0]);

	return 0;
}