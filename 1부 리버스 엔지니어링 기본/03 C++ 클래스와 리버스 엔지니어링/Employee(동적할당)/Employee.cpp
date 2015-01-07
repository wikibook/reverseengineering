
#include "windows.h"
#include "stdio.h"
#include "tchar.h"

class Employee
{
	public :
		int number;
		char name[128];
		long pay;
		void ShowData();
		void Test();
};

void Employee::ShowData()
{
	printf("number: %d\n", number);
	printf("name: %s\n", name);
	printf("pay: %d\n", pay);

	Test();
	return;
}

void Employee::Test()
{
	printf("Test fuction\n");
	return;
}

int main(int argc, char* argv[])
{
	Employee *pkang;	
	pkang = new Employee;

	pkang->number = 0x1111;
	_tcscpy(pkang->name, _T("°­º´Å¹"));
	pkang->pay = 0x100;

	pkang->ShowData();

	delete pkang;
	
	return 0;
}
