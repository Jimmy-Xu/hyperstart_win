// main.cpp : Defines the entry point for the NanoServer console application.
// REF: https://social.technet.microsoft.com/Forums/windows/en-US/382b9e64-4823-48f3-b847-1b50f38fd83d/windows-pe-serial-com-support?forum=winserversetup

#include "stdafx.h"

/*

Microsoft Windows XP Service Pack 2 (SP2), RTM
OEM Preinstallation Kit (OPK)
Readme Document
June 25, 2004

---------------

4. KNOWN ISSUES
---------------

* Adding Symbolic Names of Serial Ports to Windows PE

Windows PE does not create symbolic names such as COM1 and COM2
for serial ports. This means that applications cannot detect
and use the serial ports.

Workaround: To create the symbolic names, you can write an
application that calls the DefineDosDevice function and
uses the full device name of the port. Add this application to
Startnet.cmd so that it runs as part of the Windows PE boot
process. When the application is finished, applications
can detect the serial ports by their symbolic names.

The full device name of COM1 when using the ACPI
HAL is:

\\?\ACPI#PNP0501#1#{4D36E978-E325-11CE-BFC1-08002BE10318}

To identify the full device names for serial ports on other HALs,
use the QueryDosDevice function to search for the string "PNP0501".

To use serial ports in custom applications, either refer to the
ports by their full device names or use DefineDosDevice to create
the symbolic names as in this example:

#define COM_PORT_DEV_NAME(n) \
TEXT("\\\\?\\ACPI#PNP0501#") TEXT(#n) TEXT("#{4D36E978-E325
-11CE-BFC1-08002BE10318}")

#define COM_PORT_NAME(n) \
TEXT("COM") TEXT(#n)

DefineDosDevice( 0, COM_PORT_NAME(1), COM_PORT_DEV_NAME(1) );

*/

#include <stdio.h>
#include <windows.h>
#include <iostream>

using namespace std;

#define COM_PORT_DEV_NAME(n)  TEXT("\\\\?\\ACPI#PNP0501#") TEXT(#n) TEXT("#{4D36E978-E325-11CE-BFC1-08002BE10318}")
#define COM_PORT_NAME(n)      TEXT("COM") TEXT(#n)


// convert TCHAR to std::string
string TChar2Str(TCHAR *pChar) {
    std::string str;
#ifndef UNICODE
    str = pChar;
#else
    std::wstring wStr = pChar;
    str = std::string(wStr.begin(), wStr.end());
#endif
    return str;
}


// convert std::string to TCHAR
TCHAR* Str2TChar(string pStr) {
    static TCHAR wc[MAX_PATH];
#ifdef UNICODE
    _stprintf_s(wc, MAX_PATH, _T("%S"), pStr.c_str());//%S宽字符
#else
    _stprintf_s(wc, MAX_PATH, _T("%s"), pStr.c_str());//%s单字符
#endif
    return wc;
}

string LPCWSTR2Str(LPCWSTR wString)
{
    wstring tempWstring(wString);
    string tempString(tempWstring.begin(), tempWstring.end());
    return tempString;
}


//枚举注册表值: RegEnumValue()
int EnumSerialPort1() {
    cout << "\n====================EmumSerialPort by 'HARDWARE\\DEVICEMAP\\SERIALCOMM'====================" << endl;

    HKEY hKey;
    if (ERROR_SUCCESS == ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), NULL, KEY_READ, &hKey))//打开串口注册表对应的键值 
    {
        int i = 0;
        TCHAR RegKeyName[128], SerialPortName[128];
        DWORD dwLong, dwSize;
        while (TRUE)
        {
            dwLong = dwSize = sizeof(RegKeyName);
            if (ERROR_NO_MORE_ITEMS == ::RegEnumValue(hKey, i, RegKeyName, &dwLong, NULL, NULL, (PUCHAR)SerialPortName, &dwSize))//枚举串口 
            {
                break;
            }
            cout << "\n Value: " << TChar2Str(RegKeyName).c_str() << "\n Data: " << TChar2Str(SerialPortName).c_str() << endl;
            i++;
        }
        RegCloseKey(hKey);
    }
    else
    {
        RegCloseKey(hKey);
        cerr << TEXT("您的计算机的注册表上没有HKEY_LOCAL_MACHINE:Hardware\\DeviceMap\\SerialComm项") << endl;
        return 1;
    }
    return 0;
}


//枚举注册表键: RegEnumKeyEx()
int EnumSerialPort2() {
    cout << "\n====================EmumSerialPort by 'SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\'====================" << endl;

    TCHAR sActive[128] = TEXT("SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\");
    LPCWSTR valueName = _T("PortName");

    //TCHAR sActive[128] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\");
    //LPCWSTR valueName = _T("Display");

    //TCHAR sActive[128] = TEXT("SYSTEM\\ControlSet001\\Enum\\ACPI\\ACPI0003\\");
    //LPCWSTR valueName = _T("HardwareID");

    HKEY hKey_tmp = NULL;
    HKEY hKey = NULL;
    TCHAR comNum[256];
    TCHAR comNumEx[256];
    TCHAR achKey[128];
    DWORD i = 0;
    DWORD cbMaxSubKey = 60;
    DWORD dwType = 20;
    DWORD dwSize = sizeof(comNum);
    DWORD retReg;
    
    retReg = RegOpenKey(HKEY_LOCAL_MACHINE, sActive, &hKey);
    if (retReg == ERROR_SUCCESS) {
        while (RegEnumKeyEx(hKey, i, achKey, &cbMaxSubKey, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS/*ERROR_SUCCESS*/) {
            string curKey = TChar2Str(sActive);
            curKey.append(TChar2Str(achKey));
            curKey.append("\\Device Parameters\\");
            //cout << "Current Key: " << curKey.c_str() << endl;;

            if (RegOpenKey(HKEY_LOCAL_MACHINE, Str2TChar(curKey), &hKey_tmp) == ERROR_SUCCESS) {
                retReg = RegQueryValueEx(hKey_tmp, valueName, NULL, &dwType, (BYTE *)&comNumEx, &dwSize);
                if (retReg == ERROR_SUCCESS) {
                    cout << "\n Key: " << curKey.c_str() << "\n Value: " << LPCWSTR2Str(valueName).c_str() << "\n Data: " << TChar2Str(comNumEx).c_str() << endl;
                }
                else {
                    cerr << "error1: " << retReg << " Key:" << curKey.c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Str2TChar(curKey), 0, 0, &hKey_tmp) == ERROR_SUCCESS) {
                retReg = RegQueryValueEx(hKey_tmp, valueName, NULL, &dwType, (BYTE *)&comNumEx, &dwSize);
                if (retReg == ERROR_SUCCESS) {
                    cout << "\n Key: " << curKey.c_str() << "\n Value: " << LPCWSTR2Str(valueName).c_str() << "\n Data: " << TChar2Str(comNumEx).c_str() << endl;
                }
                else {
                    cerr << "error2: " << retReg << " Key:" << curKey.c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            dwSize = 256; //RegQueryValueEx执行完后，这个参数的值为实际的长度。因此在调用函数之前，要重新设置
            ZeroMemory(achKey, 128);
            cbMaxSubKey = 60;
            i++;
        }
    }
    else {
        cerr << "error0:" << GetLastError() << "\n Key:" << TChar2Str(sActive).c_str() << endl;
    }
    RegCloseKey(hKey_tmp);
    RegCloseKey(hKey);

    return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
    //add com1 and com2
    DefineDosDevice(0, COM_PORT_NAME(1), COM_PORT_DEV_NAME(1));  //COM1
    cout << "Add COM1 OK" << endl;
    
    DefineDosDevice(0, COM_PORT_NAME(2), COM_PORT_DEV_NAME(2));  //COM2
    cout << "Add COM2 OK" << endl;

    EnumSerialPort1();
    EnumSerialPort2();

	return 0;
}

