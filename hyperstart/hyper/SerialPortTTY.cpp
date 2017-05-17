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
#define MAXWAIT               60000

using namespace std;

//get system time
const char* GetTimeStr() {
    time_t now = time(NULL);
    char suffix[128];
    struct tm time_info;
    localtime_s(&time_info, &now);
    strftime(suffix, 128, "%Y:%m:%d %H:%M:%S", &time_info);
    return string(suffix).c_str();
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
    string result(tempWstring.begin(), tempWstring.end());
    return result;
}

LPCWSTR Str2LPCWSTR(string str) {
    std::wstring result = std::wstring(str.begin(), str.end());
    return (LPCWSTR)result.c_str();
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

void InstallDriver() {
    cout << "\n" << GetTimeStr() << " - [InstallDriver()] Begin" << endl;

    string cmd = "";
    cout << "\n" << GetTimeStr() << " - [InstallDriver] Start install msports drivers with pnputil.exe" << endl;
    cmd = "pnputil.exe /add-driver c:\\drivers\\msports-driver\\*.inf /subdirs /install > c:\\hyper\\log\\pnputil-msport.log";
    cout << system(cmd.c_str()) << endl;

    cout << "\n" << GetTimeStr() << " - [InstallDriver] Start install network drivers with pnputil.exe" << endl;
    cmd = "pnputil.exe /add-driver c:\\drivers\\network-driver\\*.inf /subdirs /install > c:\\hyper\\log\\pnputil-network.log";
    cout << system(cmd.c_str()) << endl;

    cout << "\n" << GetTimeStr() << " - [InstallDriver] Start install bus drivers with pnputil.exe" << endl;
    cmd = "pnputil.exe /add-driver c:\\drivers\\bus-driver\\*.inf /subdirs /install > c:\\hyper\\log\\pnputil-bus.log";
    cout << system(cmd.c_str()) << endl;

    cout << "\n" << GetTimeStr() << " - [InstallDriver()] End" << endl;
}

void InstallSerialDriver() {
    cout << "\n[InstallSerialDrive()] Begin" << endl;
    string cmd = "";
    if ((GetFileAttributes(_T("c:\\drivers\\msports-driver\\msports.inf"))) == -1)
    {
        cout << "Missing c:\\drivers\\msports-driver\\msports.inf" << endl;
    }
    else {
        cout << "Start install msports.inf with pnputil.exe" << endl;
        cmd = "pnputil.exe /add-driver c:\\drivers\\msports-driver\\msports.inf /install > c:\\hyper\\log\\pnputil-msport.log";
        cout << system(cmd.c_str()) << endl;
    }
    cout << "\n[InstallSerialDrive()] End" << endl;
}


void InstallNetworkDriver() {
    cout << "\n[InstallNetworkDriver()] Begin" << endl;
    string cmd = "";


    if ((GetFileAttributes(_T("c:\\drivers\\network-driver\\e1000\\nete1g3e.inf"))) == -1)
    {
        cout << "Missing c:\\drivers\\network-driver\\e1000\\nete1g3e.inf" << endl;
    }
    else {
        cout << "Start install e1000 driver with pnputil.exe" << endl;
        cmd = "pnputil.exe /add-driver c:\\drivers\\network-driver\\e1000\\nete1g3e.inf > c:\\hyper\\log\\pnputil-network.log";
        cout << system(cmd.c_str()) << endl;
    }
    
    if ((GetFileAttributes(_T("c:\\drivers\\network-driver\\netkvm\\netkvm.inf"))) == -1)
    {
        cout << "Missing c:\\drivers\\network-driver\\netkvm\\netkvm.inf" << endl;
    }
    else {
        cout << "Start install netkvm driver with pnputil.exe" << endl;
        cmd = "pnputil.exe /add-driver c:\\drivers\\network-driver\\netkvm\\netkvm.inf >> c:\\hyper\\log\\pnputil-network.log";
        cout << system(cmd.c_str()) << endl;
    }

    if ((GetFileAttributes(_T("c:\\drivers\\network-driver\\rtl8139\\netrtl64.inf"))) == -1)
    {
        cout << "Missing c:\\drivers\\network-driver\\rtl8139\\netrtl64.inf" << endl;
    }
    else {
        cout << "Start install rtl8139 driver with pnputil.exe" << endl;
        cmd = "pnputil.exe /add-driver c:\\drivers\\network-driver\\rtl8139\\netrtl64.inf >> c:\\hyper\\log\\pnputil-network.log";
        cout << system(cmd.c_str()) << endl;
    }

    cout << "\n[InstallNetworkDriver()] End" << endl;
}

void InstallVirtIODriver() {
    cout << "\n[InstallVirtIODriver()] Begin" << endl;
    string cmd = "";

    if ((GetFileAttributes(_T("c:\\drivers\\virtio-win\\vioserial\\2k16\\amd64\\vioser.inf"))) == -1)
    {
        cout << "Missing c:\\drivers\\virtio-win\\vioserial\\2k16\\amd64\\vioser.inf" << endl;
    }
    else {
        cout << "Start install virtio driver with pnputil.exe" << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\Balloon\\2k16\\amd64\\balloon.inf > c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\NetKVM\\2k16\\amd64\\netkvm.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\pvpanic\\2k16\\amd64\\pvpanic.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\vioinput\\2k16\\amd64\\vioinput.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\viorng\\2k16\\amd64\\viorng.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\vioscsi\\2k16\\amd64\\vioscsi.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\vioserial\\2k16\\amd64\\vioser.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
        cmd = "pnputil /add-driver c:\\drivers\\virtio-win\\viostor\\2k16\\amd64\\viostor.inf >> c:\\hyper\\log\\pnputil-virtio.log";
        cout << system(cmd.c_str()) << endl;
    }
    cout << "\n[InstallVirtIODriver()] End" << endl;
}

void CreateSERIALCOMM() {
    cout << "\n" << GetTimeStr() << " - [EnsureSERIALCOMM()] Begin " << endl;

    HKEY hKey;
    //打开注册表键，不存在则创建它
    TCHAR sActive[128] = TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM");
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, sActive, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL)) {
        cout << "Key " << TChar2Str(sActive).c_str() << " existed" << endl;
        return;
    }
    else {
        cout << "Key " << TChar2Str(sActive).c_str() << " can not be created" << endl;;
    }
    cout << "\n" << GetTimeStr() << " - [EnsureSERIALCOMM()] End " << endl;
}

/* Add COM in HARDWARE\\DEVICEMAP\\SERIALCOMM */
void AddComPort(int i, TCHAR *pPortName) {
    cout << "\n" << GetTimeStr() << " - [AddComPort()] " << TChar2Str(pPortName).c_str() << " Begin" << endl;

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

    cout << "\n" << GetTimeStr() << " - [AddComPort()] " << TChar2Str(pPortName).c_str() << " End" << endl;
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

    cout << "\n" << GetTimeStr() << " - [ScanSerialPort()] Begin" << endl;
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
        for (int i = 1; i <= 5; i++) {
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
            else {
                cout << "\nOpen Key: " << curKey.c_str() << " failed" << endl;;
            }
            dwSize = 256; //RegQueryValueEx执行完后，这个参数的值为实际的长度。因此在调用函数之前，要重新设置
            ZeroMemory(achKey, 128);
            cbMaxSubKey = 60;
        }
    }
    RegCloseKey(hKey_tmp);
    RegCloseKey(hKey);

    cout << "\n" << GetTimeStr() << " - [ScanSerialPort()] End" << endl;
}

/* Enumerate all Serial Ports */
void EnumerateSerialPorts(tofstream *m_logFile){
    *m_logFile << "\n" << GetTimeStr() << " - [EnumerateSerialPorts] Begin" << endl;
    const int MAX_PORT = 5;
    static char* result[MAX_PORT];

    *m_logFile << "\n" << GetTimeStr() << " - [EnumerateSerialPorts] export registry PNP05051 to ACPI.reg" << endl;
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Enum\\ACPI\\PNP0501\"", "ACPI");

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
                    *m_logFile << " " << TChar2Str(valueData).c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, Str2TChar(curKey), 0, 0, &hKey_tmp) == ERROR_SUCCESS) {
                retReg = RegQueryValueEx(hKey_tmp, valueName, NULL, &dwType, (BYTE *)&valueData, &dwSize);
                if (retReg == ERROR_SUCCESS) {
                    *m_logFile << " " << TChar2Str(valueData).c_str() << endl;
                }
                RegCloseKey(hKey_tmp);
            }
            else {
                cout << "\nOpen Key: " << curKey.c_str() << " failed" << endl;;
            }
            dwSize = 256; //RegQueryValueEx执行完后，这个参数的值为实际的长度。因此在调用函数之前，要重新设置
            ZeroMemory(achKey, 128);
            cbMaxSubKey = 60;
        }
    }
    RegCloseKey(hKey_tmp);
    RegCloseKey(hKey);

    *m_logFile << "\n" << GetTimeStr() << " - [EnumerateSerialPorts] End" << endl;
}

void ExecuteWMIC()
{
    cout << "\n" << GetTimeStr() << " - [ExecuteWMIC] Begin" << endl;
    
    string cmd = "";
    // execute wmic

    cmd = "wmic path Win32_PNPEntity where \"PNPClass = 'Net' and Service != 'kdnic'\" get Name,Status,ConfigManagerErrorCode,DeviceID,LastErrorCode,Present,Service > c:\\hyper\\log\\Win32_PnPEntity.log";
    cout << system(cmd.c_str()) << endl;

    cmd = "WMIC path Win32_SerialPort > c:\\hyper\\log\\Win32_SerialPort.log";
    cout << system(cmd.c_str()) << endl;

    cmd = "reg query \"HKLM\\SYSTEM\\ControlSet001\\Enum\\PCI\" > c:\\hyper\\log\\PCI.reg";
    cout << system(cmd.c_str()) << endl;

    cmd = "pnputil /enum-drivers > c:\\hyper\\log\\enum-drivers.log";
    cout << system(cmd.c_str()) << endl;

    //cmd = "reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\" > c:\\hyper\\log\\NetworkCards.reg";
    //cout << system(cmd.c_str()) << endl;

    //cmd = "wmic path Win32_PnPSignedDriver get devicename, driverversion > c:\\hyper\\log\\Win32_PnPSignedDriver.log";
    //cout << system(cmd.c_str()) << endl;
    
    cout << "\n" << GetTimeStr() << " - [ExecuteWMIC] End" << endl;
}


void ExportRegistry(char* cmd, char* subKey)
{
    cout << "\n" << GetTimeStr() << " - [ExportRegistry] Begin " << cmd << " - " << subKey << endl;
   
    LPCWSTR filename = Str2LPCWSTR(fmt::format("c:\\hyper\\log\\{0}.reg", subKey));
    if (GetFileAttributes(filename) != -1)
    {
        DeleteFile(filename);
    }
    cout << system(fmt::format("{0} {1}", cmd, LPCWSTR2Str(filename)).c_str()) << endl;
    
    cout << "\n" << GetTimeStr() << " - [ExportRegistry] End " << cmd << " - " << subKey << endl;
}


void ImportRegistry() {

    string cmd = "";

    //import SERIALCOMM.reg
    cmd = "reg import c:\\hyper\\reg\\SERIALCOMM.reg";
    cout << system(cmd.c_str()) << endl;

    //import PNP0501.reg
    cmd = "reg import c:\\hyper\\reg\\PNP0501.reg";
    cout << system(cmd.c_str()) << endl;
}

void EnsureSerialPort()
{
    std::ofstream comlog(fmt::format("c:\\hyper\\log\\1_EnsureSerialPort.log"), std::fstream::trunc); //trunc mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::streambuf *cerrbuf = std::cerr.rdbuf(); //save old buf
    std::cout.rdbuf(comlog.rdbuf()); //redirect std::cout to logfile!
    std::cerr.rdbuf(comlog.rdbuf()); //redirect std::cerr to logfile!

    cout << "\n" << GetTimeStr() << " - ###### [EnsureSerialPort] Begin #####" << endl;

    // REF: https://social.technet.microsoft.com/Forums/windowsserver/en-US/382b9e64-4823-48f3-b847-1b50f38fd83d/windows-pe-serial-com-support?forum=winserversetup
    DefineDosDevice(0, COM_PORT_NAME(nPort), COM_PORT_DEV_NAME(nPort));

    // Ensure HARDWARE\\DEVICEMAP\\SERIALCOMM
    CreateSERIALCOMM();

    // Install Driver for SerialDriver
    //InstallSerialDriver();
    //InstallNetworkDriver();
    //InstallVirtIODriver();
    InstallDriver();

    // list all drivers
    cout << "Start list all drivers with pnputil.exe" << endl;
    cout << system( "pnputil.exe /e > c:\\hyper\\log\\pnputil-e.log" ) << endl;

    // Generate SerialPort
    ScanSerialPort();

    // Enumerate SerialPort
    // EnumerateSerialPorts();

    // Execute wmic command
    //ExecuteWMIC();
  
    // Export Registry to File
    /*
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\"", "DEVICEMAP");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Enum\\ACPI\"", "ACPI");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Services\\Serial\"", "Serial");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM\"", "SERIALCOMM");
    */
    /*
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Control\\Class\"", "Class");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\DRIVERS\\DriverDatabase\"", "DriverDatabase");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\DRIVERS\"", "_DRIVERS");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\HARDWARE\"", "_HARDWARE");
    //ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SOFTWARE\"", "_SOFTWARE");
    ExportRegistry("reg export \"HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\"", "_ControlSet001");
    */

    cout << "\n" << GetTimeStr() << " - ###### [EnsureSerialPort] End #####" << endl;

    // Reset to standard output again
    std::cout.rdbuf(coutbuf);
    std::cerr.rdbuf(cerrbuf);
}


int CreateSerialPort(struct SerialPort *serialPort) {
    int result = 0;
    string ctlPort("COM1");
    string ttyPort("COM2");
    unsigned long baud = strtoul("115200", NULL, 0);

    try {
        // port, baudrate, timeout in milliseconds
        if (serialPort->ctl == NULL) {
            serialPort->ctl = new serial::Serial(ctlPort, baud, serial::Timeout::simpleTimeout(1000));
            SendReadyStr(serialPort->ctl, "ctl");
        }
    }
    catch (...) {
        cout << "[Error] Create ctl on COM1 failed" << endl;
        serialPort->ctl = NULL;
        result += 1;
    }

    try {
        if (serialPort->tty == NULL) {
            serialPort->tty = new serial::Serial(ttyPort, baud, serial::Timeout::simpleTimeout(1000));
            SendReadyStr(serialPort->tty, "tty");
        }
    }
    catch (...) {
        cout << "[Error] Create tty on COM2 failed" << endl;
        serialPort->tty = NULL;
        result += 2;
    }
    return result;
}

/*
Result:
    0: both ctl and tty opened
    1: ctl open failed
    2: tty open failed
    3: both ctl and tty open failed
*/
int OpenSerialPort(struct SerialPort *serialPort) {
    int result = 0;
    string port;

    //open ctl serial port
    try {
        port = serialPort->ctl->getPort().c_str();
        //cout << "==> Try to open Control Serial Port: '" << port << "'" << endl;
        if (!serialPort->ctl->isOpen()) {
            serialPort->ctl->open();
            SendReadyStr(serialPort->ctl, "ctl");
        }
    }
    catch (...) {
        cout << "Operate Serial Port '" << port << "' Failed!" << endl;
        result += 1; // 01
    }

    //open tty serial port
    try
    {
        port = serialPort->tty->getPort().c_str();
        //cout << "==> Try to open TTY Serial Port: '" << port << "'" << endl;
        if (!serialPort->tty->isOpen()) {
            serialPort->tty->open();
            SendReadyStr(serialPort->tty, "tty");
        }
    }
    catch (...) {
        cout << "Operate Serial Port '" << port << "' Failed!" << endl;
        result += 2; // 10
    }
    return result;
}

/*
Send Ready String When open serial port everytime
Result:
0: both ctl and tty send Ready ok
1: send Ready to ctl failed
2: send Ready to tty failed
3: both ctl and tty send Ready failed
*/
int SendReadyStr(serial::Serial *pPort, char *pName) {
    int result = 0;
    size_t bytes_wrote;

    try {
        bytes_wrote = pPort->write(READY_STR);
        cout << "Send Ready via " << pName << endl;
        cout << "[" << GetTimeStr() << "] Bytes written(ctl): " << bytes_wrote << endl;
    }
    catch (...) {
        cout << "Send Ready via " << pName << " failed" << endl;
        if (pName == "ctl")
            result += 1;
        else if (pName == "tty") {
            result += 2;
        }
    }
    return result;
}

int ReceiveCommand(struct SerialPort *serialPort)
{
    //cout << "[Begin] SerialPortCommunicate" << endl;
    string cmd = "";
    try {
        cout << ".";
        cout.flush();
        cmd = serialPort->ctl->readline(MAX_CMD_LENGTH);
        if (cmd.length() > 0) {
            cout << "\n[" << GetTimeStr() << "] Bytes read(ctl): ";
            cout << cmd.length() << ", String read(ctl): " << cmd.c_str() << endl;
            SendCmdResult(serialPort, cmd.c_str());
        }
    }
    catch (...) {
        cout << "[ReceiveCommand] receive command via ctl failed" << endl;
        return 1;
    }
    //cout << "[End] SerialPortCommunicate" << endl;
    return 0;
}

static wchar_t * CStr2WStr(const char *cStr)
{
    // MultiByteToWideChar( CP_ACP, 0, chr,
    //     strlen(chr)+1, wchar, size/sizeof(wchar[0]) );

    // First: get count of multi-byte string.
    const DWORD cCh = MultiByteToWideChar(CP_ACP,           // Character Page.
        0,                // Flag, always be 0.
        cStr,             // Multi-byte string.
        -1,               // '-1' is to determind length automatically.
        NULL,             // 'NULL' means ignore result. This is based
                          // on next argument is '0'.
        0);               // '0' is to get the count of character needed
                          // instead of to translate.

                          // Second: allocate enough memory to save result in wide character.
    wchar_t* wStr = new wchar_t[cCh];
    ZeroMemory(wStr, cCh * sizeof(wStr[0]));

    // Third: Translate it!
    MultiByteToWideChar(CP_ACP,                             // Character Page.
        0,                                  // Flag, always be 0.
        cStr,                               // Multi-byte string.
        -1,                                 // Count of character of string above.
        wStr,                               // Target wide character buffer.
        cCh);                               // Count of character of wide character string buffer.

    return wStr;
}

string ExeCmd(const char *pszCmd)
{
    cout << "[ExeCmd] execute cmd '" << pszCmd << "'" << endl;
    //创建匿名管道
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0))
    {
        cout << "[ExeCmd] CreatePipe failed" << endl;
        return "\n[ExeCmd] CreatePipe failed\n";
    }

    //设置命令行进程启动信息(以隐藏方式启动命令并定位其输出到hWrite)
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    GetStartupInfo(&si);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
   
    //启动命令行
    PROCESS_INFORMATION pi;
    if (!CreateProcess(NULL, CStr2WStr(pszCmd), NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
    {
        cout << "[ExeCmd] CreateProcess failed" << endl;
        return "\n[ExeCmd] CreateProcess failed\n";
    }
    else
    {
        DWORD retVal = WaitForSingleObject(pi.hProcess, MAXWAIT);
        if (retVal == WAIT_TIMEOUT)
        {
            if (TerminateProcess(pi.hProcess, 0)) {
                // Success 
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                cout << "[ExeCmd] CreateProcess timeout, terminate OK" << endl;
                return "\n[ExeCmd] CreateProcess timeout, terminate OK\n";
            }
            else 
            {
                // Failed to terminate process
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                cout << "[ExeCmd] CreateProcess timeout, terminate failed" << endl;
                return "\n[ExeCmd] CreateProcess timeout, terminate failed\n";
            }
        }
    }

    //立即关闭hWrite
    CloseHandle(hWrite);

    //读取命令行返回值
    std::string strRet;
    char buff[1024] = { 0 };
    DWORD dwRead = 0;
    while (ReadFile(hRead, buff, 1024, &dwRead, NULL))
    {
        strRet.append(buff, dwRead);
    }
    CloseHandle(hRead);

    return strRet;
}

int SendCmdResult(SerialPort *serialPort, const char *cmd) {
    try {
        cout << "[ExecuteCommand] execute command:" << cmd << endl;
        string result = ExeCmd(cmd);
        cout << "[ExecuteCommand] send result via tty:" << result << endl; 
        size_t bytes_wrote = serialPort->tty->write(result);
        serialPort->tty->write("\n\nDONE\n\n");
        //serialPort->tty->flush();
        cout << "[" << GetTimeStr() << "] Bytes written(tty): ";
        cout << bytes_wrote << endl;
    }
    catch (...) {
        cout << "[ExecuteCommand] send result via tty failed" << endl;
        return 1;
    }
    return 0;
}

