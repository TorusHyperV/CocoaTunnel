// CuantoTunnel.c - An Xbox One pure C flash dumper
// TorusHyperV (c) 2023

//////////////////////////////////////////////////
// Dependencies
//////////////////////////////////////////////////
#include <Windows.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//////////////////////////////////////////////////
// Macros / Constants
//////////////////////////////////////////////////
#define UNUSED(x) (void)(x)
#define LOCAL_BUF_SIZE 1024

//////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////
LPCWSTR      SYSTEMOS_FLASH_FILE = L"\\\\.\\Xvuc\\Flash";


//////////////////////////////////////////////////
// Auxiliar Functions
//////////////////////////////////////////////////

void ShowUsage()
{
	printf("Usage:\n");
	printf("CuantoTunnel.exe [dump_path]\n");
	printf("                 Ensure you have at least 5Gb of storage\n");
	printf("                 available in the drive letter of [dump_path]\n\n");
	printf("Example: CuantoTunnel.exe E:\\my_nand.bin\n");
}

void ShowMenu()
{
	printf("==========================================================\n");
	printf("================        CuantoTunnel       ===============\n");
	printf("================   Xbox One Flash Dumper   ===============\n");
	printf("==========================================================\n\n");
}

void ShowExecutionCtx()
{
	// This function just retrieves and prints
	// Some information about the console where
	// the tool is being ran.

	// User
	long unsigned int bufsize = 100;
	char userbuf[100];
	GetUserNameA(userbuf, &bufsize);
	printf("[i] user: %s\n", userbuf);

	// Is the process being run with admin privileges
	printf("[i] Elevated process: %s\n", "unknown");

	// Build version
}

char* GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0) {
		return NULL; //No error message has been recorded
	}

	LPSTR messageBuffer = NULL;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	return messageBuffer;
}

//////////////////////////////////////////////////
// Entrypoint
//////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	HANDLE  flash_pipe_hdnl;
	uint8_t read_buf[LOCAL_BUF_SIZE];
	LPDWORD total_read_bytes = 0;
	LPDWORD read_bytes       = 0;
	bool    success          = true;

	// Step 0: Check args, and show some info
	if(argc != 2)
	{
		ShowUsage();
		return -1;
	}
	ShowMenu();
	ShowExecutionCtx();

	// Step 1: Get a handle the RAW Flash named pipe with CreateFile
	flash_pipe_hdnl = CreateFile(SYSTEMOS_FLASH_FILE,
								 GENERIC_READ,
								 FILE_SHARE_READ | FILE_SHARE_WRITE,
								 NULL,
								 OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL,
								 NULL);

	if(flash_pipe_hdnl == INVALID_HANDLE_VALUE)
	{
		char* error_str = GetLastErrorAsString();
		printf("[!] CreateFile failed when opening the Flash PIPE with error: %s\n", error_str);
		LocalFree(error_str);
		return -1;
	}

	// Step 2: Run sanity checks
	
	// 2.1. Check for FILE_LIST_DIRECTORY and WRITE permissions
	//		TODO
	// 2.2. Check for enough storage space
	//		TODO

	// Step 3: Read flash to file
	for(;;)
	{
		success = ReadFile(flash_pipe_hdnl, read_buf, LOCAL_BUF_SIZE, read_bytes, NULL);

		// We reached EOF
		if (success && read_bytes == 0)
		{
			printf("\n[i] Flash dumped completed successfully. Total size: %ld bytes\n", *total_read_bytes);
			break;
		}
		else if (!success)
		{
			printf("\n[x] Flash dump failed. Read %ld bytes\n", *total_read_bytes);
			break;
		}

		total_read_bytes += *read_bytes;

		// Print some informative dots
		if ((*total_read_bytes % LOCAL_BUF_SIZE * 10))
			printf(".");
	}


	// Step 4: Cleanup
	CloseHandle(flash_pipe_hdnl);

	return 0;
}

