#define UNICODE 1
#define _UNICODE 1

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <WinError.h>
#include <string>
#include <malloc.h>

#define DEVICE_PRIMARY "\\\\.\\AppleTrackpad"
#define DEVICE_ALTERNATE "\\\\.\\AppleWirelessTrackpad"
#define REG_SUBKEY "Software\\Apple Inc.\\Trackpad"
#define REG_VALUE "mode"

union Options {
	struct {
		unsigned short tapToClick		: 1;
		unsigned short dragging			: 1;
		unsigned short dragLock			: 1;
		unsigned short reserved1		: 1;
		unsigned short reserved2		: 1;
		unsigned short secondaryTap		: 1;
		unsigned short secondaryClick	: 1;
		unsigned short bottomLeft		: 1;
	} bits;
	DWORD bytes;
};

BOOL ReadTrackpadOptions(DWORD *opt) {
	HANDLE hDevice;
	BOOL bResult;
	DWORD dwBytesReturned;

	hDevice = CreateFile(TEXT(DEVICE_PRIMARY),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice == INVALID_HANDLE_VALUE) {
		hDevice = CreateFile(TEXT(DEVICE_ALTERNATE),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hDevice == INVALID_HANDLE_VALUE) {
			return (FALSE);
		}
	}

	bResult = DeviceIoControl(hDevice,
		CTL_CODE(FILE_DEVICE_MOUSE, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS), //0xf2000,
		NULL, 0,
		opt, sizeof(*opt),
		&dwBytesReturned,
		(LPOVERLAPPED) NULL);

	CloseHandle(hDevice);

	return (bResult);
}

BOOL WriteTrackpadOptions(DWORD *opt) {
    HANDLE hDevice;
    BOOL bResult;
    DWORD junk;
    
    hDevice = CreateFile(TEXT(DEVICE_PRIMARY),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        hDevice = CreateFile(TEXT(DEVICE_ALTERNATE),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);
        if (hDevice == INVALID_HANDLE_VALUE) {
            return (FALSE);
        }
    }
    
    bResult = DeviceIoControl(hDevice,
        CTL_CODE(FILE_DEVICE_MOUSE, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS), //0xf2004,
        opt, sizeof(*opt),
        NULL, 0,
        &junk,
        (LPOVERLAPPED) NULL);
    
    CloseHandle(hDevice);
    
    return (bResult);
}

BOOL ReadRegistryOptions(DWORD *optbytes) {
	LONG lResult;
	DWORD dwDisposition;
	HKEY hKey = NULL;
	DWORD dwSize = 0;
	DWORD dwDataType = 0;
	LPBYTE lpValue = NULL;
	LPCTSTR const lpValueName = TEXT(REG_VALUE);

	lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
		(LPCTSTR)TEXT(REG_SUBKEY),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition);
	if (lResult != ERROR_SUCCESS) {
		printf("Error loading/creating key");
		return (FALSE);
	}

	lResult = RegQueryValueEx(hKey,
		lpValueName,
		0,
		&dwDataType,
		lpValue,
		&dwSize);
	lpValue = (LPBYTE)malloc(dwSize);
	lResult = RegQueryValueEx(hKey,
		lpValueName,
		0,
		&dwDataType,
		lpValue,
		&dwSize);
	RegCloseKey(hKey);
	if (lResult != ERROR_SUCCESS) {
		return (FALSE);
	}

	*optbytes = (DWORD)*lpValue;

	if (lResult != ERROR_SUCCESS) {
		return (FALSE);
	}
	return (TRUE);
}

int main(int argc, char *argv[]) {
    BOOL bResult;

	Options opts;

	opts.bytes = 0;

	bResult = ReadRegistryOptions(&opts.bytes);
	if (!bResult || opts.bytes == 0) {
		//load defaults
		printf("Unable to read registry, loading defaults...");
		opts.bits.reserved1 = TRUE;
		opts.bits.reserved2 = FALSE;

		opts.bits.bottomLeft = FALSE;
		opts.bits.secondaryClick = TRUE;
		opts.bits.secondaryTap = TRUE;
		opts.bits.dragLock = TRUE;
		opts.bits.dragging = TRUE;
		opts.bits.tapToClick = TRUE;
	} else {
		printf("Read settings from registry...");
	}
    
    bResult = WriteTrackpadOptions (&opts.bytes);
    
    if (bResult) {
         printf("Success writing settings to trackpad.");
    } else {
        printf("Failed writing settings to trackpad.");
    }
    
    return ((int)bResult);
}