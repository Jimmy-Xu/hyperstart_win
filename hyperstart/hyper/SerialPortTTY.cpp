#pragma region Includes
#include "SerialPortTTY.h"
#include "serial/serial.h"
#include "fmt/format.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <time.h>
#include <tchar.h>
#pragma endregion

// for AddSerialPort
#define COM_PORT_DEV_NAME(n)  TEXT("\\\\?\\ACPI#PNP0501#") TEXT(#n) TEXT("#{4D36E978-E325-11CE-BFC1-08002BE10318}")
#define COM_PORT_NAME(n)      TEXT("COM") TEXT(#n)

using namespace std;

//get system time
string GetTimeStr() {
    time_t now = time(NULL);
    char suffix[128];
    struct tm time_info;
    localtime_s(&time_info, &now);
    strftime(suffix, 128, "%Y:%m:%d %H:%M:%S", &time_info);
    return string(suffix);
}

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

wstring Str2WStr(const string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}


/* Add 'DosDeviceName' in SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\ */
void SetDosDeviceName(HKEY *hKey, TCHAR *pPortName) {
    cout << "\n[SetDosDeviceName()] " << TChar2Str(pPortName).c_str() << endl;
    if (ERROR_SUCCESS == RegSetValueEx(
        *hKey,
        TEXT("DosDeviceName"),
        0,
        REG_SZ,
        (BYTE *)pPortName,
        (DWORD)(_tcslen(pPortName) + 1) * sizeof(TCHAR)
    )) {
        cout << "Set DosDeviceName: " << TChar2Str(pPortName).c_str() << " OK!" << endl;
    }
    else {
        cout << "Set DosDeviceName: " << TChar2Str(pPortName).c_str() << " Failed:(" << endl;
    }
}

/* Add 'PortName' in SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\ */
void SetPortName(HKEY *hKey, TCHAR *pPortName) {
    cout << "\n[SetPortName()] " << TChar2Str(pPortName).c_str() << endl;
    if (ERROR_SUCCESS == RegSetValueEx(
        *hKey,
        TEXT("PortName"),
        0,
        REG_SZ,
        (BYTE *)pPortName,
        (DWORD)(_tcslen(pPortName) + 1) * sizeof(TCHAR)
    )) {
        cout << "Set PortName: " << TChar2Str(pPortName).c_str() << " OK!" << endl;
    }
    else {
        cout << "Set PortName: " << TChar2Str(pPortName).c_str() << " Failed:(" << endl;
    }
}


/* Add 'FriendlyName' in SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\? */
void SetFriendlyName(TCHAR *sActive, int i, TCHAR *pPortName) {
    cout << "\n[SetFriendlyName()] " << TChar2Str(pPortName).c_str() << endl;
    HKEY hKey = NULL;
    string curKey = fmt::format("{0}{1}\\", TChar2Str(sActive).c_str(), i);
    cout << "Current Key: " << curKey.c_str() << endl;;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, Str2TChar(curKey), &hKey) == ERROR_SUCCESS) {
        if (ERROR_SUCCESS == RegSetValueEx(
            hKey,
            TEXT("FriendlyName"),
            0,
            REG_SZ,
            (BYTE *)pPortName,
            (DWORD)(_tcslen(pPortName) + 1) * sizeof(TCHAR)
        )) {
            cout << "Set FriendlyName: " << TChar2Str(pPortName).c_str() << " OK!" << endl;
        }
        else {
            cout << "Set FriendlyName: " << TChar2Str(pPortName).c_str() << " Failed:(" << endl;
        }
    }
    RegCloseKey(hKey);
}

/* Add COM in HARDWARE\\DEVICEMAP\\SERIALCOMM */
void AddComPort(int i, TCHAR *pPortName) {
    cout << "\n[AddComPort()] " << TChar2Str(pPortName).c_str() << endl;

    HKEY hKey = NULL;
    TCHAR sActive[128] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");
    cout << "Current Key: " << TChar2Str(sActive).c_str() << endl;;

    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, sActive, &hKey)) {
        
        cout << "Add COM: " << TChar2Str(pPortName).c_str() << endl;
        //valueName
        string valueName = fmt::format("\\Device\\Serial{0}", i - 1);
        cout << " valueName: " << valueName.c_str() << "\n valueType: REG_SZ\n valueData: " << TChar2Str(pPortName).c_str() << " (size: " << (_tcslen(pPortName) + 1) * sizeof(TCHAR) << ")" << endl;

        if (ERROR_SUCCESS == RegSetValueEx(
            hKey,
            Str2WStr(valueName).c_str(),
            0,
            REG_SZ,
            (BYTE *)pPortName,
            (DWORD)(_tcslen(pPortName) + 1) * sizeof(TCHAR)
        )) {
            cout << "Add " << TChar2Str(pPortName).c_str() << " OK!" << endl;
        }
        else {
            cout << "Add " << TChar2Str(pPortName).c_str() << " Failed:(" << endl;
        }
    }
    else {
        cout << "Open Key " << TChar2Str(sActive).c_str() << " Failed:(" << endl;
    }
    RegCloseKey(hKey);
}

// Generate COM name
TCHAR* getComName(int i) {
    TCHAR comNumEx[256];
    _itow_s(i, comNumEx, 10);
    TCHAR *result = (TCHAR *)malloc((lstrlen(_T("COM")) + lstrlen(comNumEx) + 1) * sizeof(TCHAR));
    lstrcpy(result, _T("COM"));
    lstrcat(result, comNumEx);
    return result;
}

/* Scan SerialPort by 'SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\' */
void ScanSerialPort() {
    cout << "\n[ScanSerialPort()]" << endl;
    const int MAX_PORT = 5;
    static char* result[MAX_PORT];

    TCHAR sActive[128] = TEXT("SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\");

    HKEY hKey_tmp = NULL;
    HKEY hKey = NULL;
    TCHAR comNumEx[256];
    TCHAR achKey[128];
    DWORD cbMaxSubKey = 60;
    DWORD dwType = 20;
    DWORD dwSize = sizeof(comNumEx);
    DWORD retReg;

    retReg = RegOpenKey(HKEY_LOCAL_MACHINE, sActive, &hKey);
    if (retReg == ERROR_SUCCESS) {
        for (int i=1; i<= 5; i++) {
            string curKey = fmt::format("{0}{1}\\Device Parameters\\", TChar2Str(sActive).c_str(), i);
            cout << "============================================================" << "\nScan Key: " << curKey.c_str() << endl;;

            if (RegOpenKey(HKEY_LOCAL_MACHINE, Str2TChar(curKey), &hKey_tmp) == ERROR_SUCCESS) {
                wcscpy_s(comNumEx, getComName(i));
                cout << "Found COM: " << TChar2Str(comNumEx).c_str() << endl;
                SetDosDeviceName(&hKey_tmp, comNumEx);
                SetPortName(&hKey_tmp, comNumEx);
                SetFriendlyName(sActive, i, comNumEx);
                AddComPort(i, comNumEx);
                RegCloseKey(hKey_tmp);
            }
            else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Str2TChar(curKey), 0, 0, &hKey_tmp) == ERROR_SUCCESS) {
                wcscpy_s(comNumEx, getComName(i));
                cout << "Found COM: " << TChar2Str(comNumEx).c_str() << endl;
                SetDosDeviceName(&hKey_tmp, comNumEx);
                SetPortName(&hKey_tmp, comNumEx);
                SetFriendlyName(sActive, i, comNumEx);
                AddComPort(i, comNumEx);
                RegCloseKey(hKey_tmp);
            }
            dwSize = 256; //RegQueryValueEx执行完后，这个参数的值为实际的长度。因此在调用函数之前，要重新设置
            ZeroMemory(achKey, 128);
            cbMaxSubKey = 60;
        }
    }
    RegCloseKey(hKey_tmp);
    RegCloseKey(hKey);
}


/* Enumerate all Serial Ports */
void EnumerateSerialPorts(){
    cout << "\n[EnumerateSerialPorts()]" << endl;
    const int MAX_PORT = 5;
    static char* result[MAX_PORT];

    TCHAR sActive[128] = TEXT("SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\\");
    LPCWSTR valueName = _T("PortName");

    HKEY hKey_tmp = NULL;
    HKEY hKey = NULL;
    TCHAR valueData[256];
    TCHAR achKey[128];
    DWORD cbMaxSubKey = 60;
    DWORD dwType = 20;
    DWORD dwSize = sizeof(valueData);
    DWORD retReg;

    retReg = RegOpenKey(HKEY_LOCAL_MACHINE, sActive, &hKey);
    if (retReg == ERROR_SUCCESS) {
        for (int i = 1; i <= 5; i++) {
            string friendlyName = "";
            string curKey = fmt::format("{0}{1}\\Device Parameters\\", TChar2Str(sActive).c_str(), i);
            if (RegOpenKey(HKEY_LOCAL_MACHINE, Str2TChar(curKey), &hKey_tmp) == ERROR_SUCCESS) {
                retReg = RegQueryValueEx(hKey_tmp, valueName, NULL, &dwType, (BYTE *)&valueData, &dwSize);
                if (retReg == ERROR_SUCCESS) {
                    cout << " " << TChar2Str(valueData).c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Str2TChar(curKey), 0, 0, &hKey_tmp) == ERROR_SUCCESS) {
                retReg = RegQueryValueEx(hKey_tmp, valueName, NULL, &dwType, (BYTE *)&valueData, &dwSize);
                if (retReg == ERROR_SUCCESS) {
                    cout << " " << TChar2Str(valueData).c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            dwSize = 256; //RegQueryValueEx执行完后，这个参数的值为实际的长度。因此在调用函数之前，要重新设置
            ZeroMemory(achKey, 128);
            cbMaxSubKey = 60;
        }
    }
    RegCloseKey(hKey_tmp);
    RegCloseKey(hKey);
}


int SerialPortCommunicate(char* cPort, char* cBaud)
{
    std::ofstream ttylog(fmt::format("c:\\hyper\\tty.log"), std::fstream::app); //append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(ttylog.rdbuf()); //redirect std::cout to tty.log!

    try
    {
        // Argument 1 is the serial port or enumerate flag
        //string port(argv[1]);
        string port(cPort);

        // Argument 2 is the baudrate
        unsigned long baud = strtoul(cBaud, NULL, 0);

        cout << "==> Try to open Serial Port: '" << cPort << "'" << endl;

        // port, baudrate, timeout in milliseconds
        serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(1000));

        cout << "Is the serial port '" << cPort << "' open?";
        if (my_serial.isOpen())
            cout << " Yes." << endl;
        else
            cout << " No." << endl;

        // Get the Ready string
        int count = 0;
        string test_string = "Ready";

        // Send Ready string
        size_t bytes_wrote = my_serial.write(test_string);
        string result = my_serial.read(test_string.length() + 1);
        cout << "[" << GetTimeStr() << "] Iteration: " << count << ", Bytes written: ";
        cout << bytes_wrote << ", Bytes read: ";
        cout << result.length() << ", String read: " << result << endl;
    }
    catch (...) {
        cout << "Operate Serial Port '" << cPort << "' Failed!" << endl;
    }
    // Reset to standard output again
    std::cout.rdbuf(coutbuf);
    return 0;
}

void ExportRegistry()
{
    string cmd = "";

    // skip import registry
    /*
    //import SERIALCOMM.reg
    cmd = "reg import c:\\hyper\\reg\\SERIALCOMM.reg";
    cout << system(cmd.c_str()) << endl;

    //import PNP0501.reg
    cmd = "reg import c:\\hyper\\reg\\PNP0501.reg";
    cout << system(cmd.c_str()) << endl;
    */

    // can not run wmic
    /*
    cmd = "wmic path Win32_PnPEntity where \"PNPClass = 'Ports'\" get 'Name,Service,Status,DeviceID' > wmic.log";
    cout << system(cmd.c_str()) << endl;
    */

    //export SERIALCOMM.reg
    if ((GetFileAttributes(_T("c:\\hyper\\SERIALCOMM.reg"))) != -1)
    {
        DeleteFile(_T("c:\\hyper\\SERIALCOMM.reg"));
    }
    cmd = "reg export \"HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM\" c:\\hyper\\SERIALCOMM.reg";
    cout << system(cmd.c_str()) << endl;

    //export PNP0501.reg
    if ((GetFileAttributes(_T("c:\\hyper\\PNP0501.reg"))) != -1)
    {
        DeleteFile(_T("c:\\hyper\\PNP0501.reg"));
    }
    cmd = "reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\" c:\\hyper\\PNP0501.reg";
    cout << system(cmd.c_str()) << endl;
}


void EnsureSerialPort()
{
    std::ofstream comlog(fmt::format("c:\\hyper\\com.log"), std::fstream::app); //append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(comlog.rdbuf()); //redirect std::cout to com.log!

    // REF: https://social.technet.microsoft.com/Forums/windowsserver/en-US/382b9e64-4823-48f3-b847-1b50f38fd83d/windows-pe-serial-com-support?forum=winserversetup
    DefineDosDevice(0, COM_PORT_NAME(nPort), COM_PORT_DEV_NAME(nPort));

    // Export Registry to File
    ExportRegistry();

    // Generate SerialPort
    ScanSerialPort();

    // Enumerate SerialPort
    EnumerateSerialPorts();

    // Reset to standard output again
    std::cout.rdbuf(coutbuf);
}
