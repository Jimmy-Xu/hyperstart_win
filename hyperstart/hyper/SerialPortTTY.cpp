#pragma region Includes
#include "SerialPortTTY.h"
#include "serial/serial.h"
#include "fmt/format.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <time.h>
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

void EnumeratePorts()
{
    std::ofstream ttylog(fmt::format("c:\\hyper\\tty.log"), std::fstream::app); //append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(ttylog.rdbuf()); //redirect std::cout to out file

    vector<serial::PortInfo> devices_found = serial::list_ports();
    vector<serial::PortInfo>::iterator iter = devices_found.begin();
    cout << "\n[" << GetTimeStr() << "] Enumerate all ports:" << endl;
    while (iter != devices_found.end())
    {
        serial::PortInfo device = *iter++;
        //output to out
        cout << fmt::format("({0}, {1}, {2})", device.port.c_str(), device.description.c_str(), device.hardware_id.c_str()) << endl;
    }

    //reset to standard output again
    std::cout.rdbuf(coutbuf);
}

int SerialPortCommunicate(char* cPort, char* cBaud)
{
    std::ofstream ttylog(fmt::format("c:\\hyper\\tty.log"), std::fstream::app); //append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(ttylog.rdbuf()); //redirect std::cout to out.txt!

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


// REF: https://social.technet.microsoft.com/Forums/windowsserver/en-US/382b9e64-4823-48f3-b847-1b50f38fd83d/windows-pe-serial-com-support?forum=winserversetup
void EnsureSerialPort(int nPort)
{
    DefineDosDevice(0, COM_PORT_NAME(nPort), COM_PORT_DEV_NAME(nPort));
}