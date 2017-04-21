#pragma region Includes
#include <stdio.h>
#include <windows.h>
#include <strsafe.h>
#include <iostream>
#include "ServiceInstaller.h"
#include "ServiceBase.h"
#include "HyperStartService.h"
#include "SerialPortTTY.h"
#pragma endregion


// 
// Settings of the service
// 

// Internal name of the service
#define SERVICE_NAME             L"HyperStartService"

// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"HyperStart Service"

// Service start options.
#define SERVICE_START_TYPE       SERVICE_AUTO_START

// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""

// The name of the account under which the service should run
#define SERVICE_ACCOUNT          L"LocalSystem"

// The password to the service account name
#define SERVICE_PASSWORD         NULL

// Parameters for Serial Port
#define DEFAULT_COM_PORT         L"com1"

#define DEFAULT_COM_BAUD         L"115200"


std::string GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::string(); //No error message has been recorded

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    std::string message(messageBuffer, size);

    //Free the buffer.
    LocalFree(messageBuffer);
    return message;
}

void ErrorExit(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    wprintf((LPCTSTR)lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

//
//  FUNCTION: wmain(int, wchar_t *[])
//
//  PURPOSE: entrypoint for the application.
// 
//  PARAMETERS:
//    argc - number of command line arguments
//    argv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    wmain() either performs the command line task, or run the service.
//
int wmain(int argc, wchar_t *argv[])
{
    LPCWSTR logdir = TEXT("c:\\hyper\\log");
    if (GetFileAttributes(logdir) == -1)
    {
        if (CreateDirectory(logdir, NULL) == -1) {
            wprintf(L"\nCreate logdir failed...\n");
            return -1;
        }
    }

    wprintf(L"\nChecking SerialPort...\n");
    EnsureSerialPort();

    if (argc == 3) {
        wprintf(L"Starting HyperStartService:\n");
        CHyperStartService service(SERVICE_NAME);
        if (!CServiceBase::Run(service))
        {
            wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
            std::cout << GetLastErrorAsString().c_str() << std::endl;
            ErrorExit(TEXT("CServiceBase::Run"));
        }
        else {
            wprintf(L"HyperStartService Started\n");
        }
    }
    else if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
    {
        if (_wcsicmp(L"install", argv[1] + 1) == 0)
        {
            // Install the service when the command is 
            // "-install" or "/install".
            InstallService(
                SERVICE_NAME,               // Name of service
                SERVICE_DISPLAY_NAME,       // Name to display
                SERVICE_START_TYPE,         // Service start type
                SERVICE_DEPENDENCIES,       // Dependencies
                SERVICE_ACCOUNT,            // Service running account
                SERVICE_PASSWORD,           // Password of the account
                DEFAULT_COM_PORT,           // Serial Port Number
                DEFAULT_COM_BAUD            // Serial Port Baud
                );
        }
        else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
        {
            // Uninstall the service when the command is 
            // "-remove" or "/remove".
            UninstallService(SERVICE_NAME);
        }
    }
    else
    {
        wprintf(L"\nParameters:\n");
        wprintf(L" <COM> <BAUD>            start service.\n");
        wprintf(L" -install <COM> <BAUD>   to install the service.\n");
        wprintf(L" -remove                 to remove the service.\n");
    }

    return 0;
}