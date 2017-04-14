/****************************** Module Header ******************************\
* Module Name:  SampleService.cpp
* Project:      HyperStartService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma region Includes
#include "HyperStartService.h"
#include "ThreadPool.h"
#include "fmt/format.h"
#include "serial/serial.h"
#include <time.h>
#include <iostream>
#pragma endregion

using namespace std;

CHyperStartService::CHyperStartService(PWSTR pszServiceName, 
                               BOOL fCanStop, 
                               BOOL fCanShutdown, 
                               BOOL fCanPauseContinue)
: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
    m_fStopping = FALSE;

    // Create a manual-reset event that is not signaled at first to indicate 
    // the stopped signal of the service.
    m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_hStoppedEvent == NULL)
    {
        throw GetLastError();
    }
}


CHyperStartService::~CHyperStartService(void)
{
    if (m_hStoppedEvent)
    {
        CloseHandle(m_hStoppedEvent);
        m_hStoppedEvent = NULL;
    }
}


//
//   FUNCTION: CHyperStartService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void CHyperStartService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
    m_logFile.close();

    // TODO(Olster): Read this path from registry of from command line arguments.
    // This doesn't create non-existent dirs.
    m_logFile.open("c:\\hyper\\hyperstart.log");

    if (!m_logFile.is_open()) {
        WriteErrorLogEntry(L"Can't open log file", EVENTLOG_ERROR_TYPE);
    }
    else {
         WriteEventLogEntry(L"Open log file OK", EVENTLOG_INFORMATION_TYPE);
    }
    
    if (m_logFile.is_open()) {
        //write time to logfile
        time_t calendar_time = time(NULL);
        char buffer[80];
        struct tm time_info;
        localtime_s(&time_info, &calendar_time);
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &time_info);
        m_logFile << fmt::format("{0} : HyperStartService Started", buffer).c_str() << endl;

        //write arguments to logfile
        for (unsigned int i = 0; i < dwArgc; ++i)
        {
            m_logFile << "argv[" << i << "] = '" << lpszArgv[i] << "'" << endl;
        }
    }

    // Log a service start message to the Application log.
    WriteEventLogEntry(L"HyperStartService in OnStart", EVENTLOG_INFORMATION_TYPE);

    // Queue the main service function for execution in a worker thread.
    CThreadPool::QueueUserWorkItem(&CHyperStartService::ServiceWorkerThread, this);


}


//
//   FUNCTION: CHyperStartService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void CHyperStartService::ServiceWorkerThread(void)
{
    WriteEventLogEntry(L"Enter ServiceWorkerThread", EVENTLOG_INFORMATION_TYPE);
    // Periodically check if the service is stopping.
    while (!m_fStopping)
    {
        // Perform main service function here...
        ReadFromSerialPort();
    }
    WriteEventLogEntry(L"Exit ServiceWorkerThread", EVENTLOG_INFORMATION_TYPE);
    // Signal the stopped event.
    SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CHyperStartService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void CHyperStartService::OnStop()
{
    // Log a service stop message to the Application log.
    WriteEventLogEntry(L"HyperStartService in OnStop", 
        EVENTLOG_INFORMATION_TYPE);

    // Indicate that the service is stopping and wait for the finish of the 
    // main service function (ServiceWorkerThread).
    m_fStopping = TRUE;
    if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
    {
        throw GetLastError();
    }
}

void CHyperStartService::enumerate_ports()
{
    vector<serial::PortInfo> devices_found = serial::list_ports();

    vector<serial::PortInfo>::iterator iter = devices_found.begin();

    while (iter != devices_found.end())
    {
        serial::PortInfo device = *iter++;
        //output to out.txt
        cout << fmt::format("({0}, {1}, {2})", device.port.c_str(), device.description.c_str(), device.hardware_id.c_str()) << endl;
    }
}

int CHyperStartService::ReadFromSerialPort()
{
    std::ofstream out("c:\\hyper\\run.log");
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

    // list all serial port
    enumerate_ports();

    // Argument 1 is the serial port or enumerate flag
    //string port(argv[1]);
    string port("com1");

    // Argument 2 is the baudrate
    unsigned long baud = 115200;

    // port, baudrate, timeout in milliseconds
    serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(1000));

    cout << "Is the serial port open?";
    if (my_serial.isOpen())
        cout << " Yes." << endl;
    else
        cout << " No." << endl;

    // Get the Test string
    int count = 0;
    string test_string = "Testing.";

    // Test the timeout, there should be 1 second between prints
    cout << "Timeout == 1000ms, asking for 1 more byte than written." << endl;
    while (count < 10) {
        size_t bytes_wrote = my_serial.write(test_string);

        string result = my_serial.read(test_string.length() + 1);

        cout << "Iteration: " << count << ", Bytes written: ";
        cout << bytes_wrote << ", Bytes read: ";
        cout << result.length() << ", String read: " << result << endl;

        count += 1;
    }

    // Test the timeout at 250ms
    my_serial.setTimeout(serial::Timeout::max(), 250, 0, 250, 0);
    count = 0;
    cout << "Timeout == 250ms, asking for 1 more byte than written." << endl;
    while (count < 10) {
        size_t bytes_wrote = my_serial.write(test_string);

        string result = my_serial.read(test_string.length() + 1);

        cout << "Iteration: " << count << ", Bytes written: ";
        cout << bytes_wrote << ", Bytes read: ";
        cout << result.length() << ", String read: " << result << endl;

        count += 1;
    }

    // Test the timeout at 250ms, but asking exactly for what was written
    count = 0;
    cout << "Timeout == 250ms, asking for exactly what was written." << endl;
    while (count < 10) {
        size_t bytes_wrote = my_serial.write(test_string);

        string result = my_serial.read(test_string.length());

        cout << "Iteration: " << count << ", Bytes written: ";
        cout << bytes_wrote << ", Bytes read: ";
        cout << result.length() << ", String read: " << result << endl;

        count += 1;
    }

    // Test the timeout at 250ms, but asking for 1 less than what was written
    count = 0;
    cout << "Timeout == 250ms, asking for 1 less than was written." << endl;
    while (count < 10) {
        size_t bytes_wrote = my_serial.write(test_string);

        string result = my_serial.read(test_string.length() - 1);

        cout << "Iteration: " << count << ", Bytes written: ";
        cout << bytes_wrote << ", Bytes read: ";
        cout << result.length() << ", String read: " << result << endl;

        count += 1;
    }

    std::cout.rdbuf(coutbuf); //reset to standard output again
    return 0;
}
