#include <windows.h>
#include <stdio.h>

void RunProcess()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	
	// Start the child process.
	if( !CreateProcess( NULL,
		"MyChildProcess",
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi )
		)
	{
		printf( "CreateProcess failed.\n" );
		return;
	}
	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );
	
	// Close process and thread handles.
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
}

void main(int argc, char *argv[])
{
	RunProcess();
}