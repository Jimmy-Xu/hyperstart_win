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
#include "SerialPortTTY.h"
#include "fmt/format.h"
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

    // list all available serial port
    EnumerateSerialPorts();

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

    std::ofstream outlog(fmt::format("c:\\hyper\\log\\serial.log"), std::fstream::app); //append mode
    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    std::cout.rdbuf(outlog.rdbuf()); //redirect std::cout to serial.log!

    std::ofstream errlog(fmt::format("c:\\hyper\\log\\serial.err"), std::fstream::app); //append mode
    std::streambuf *cerrbuf = std::cerr.rdbuf(); //save old buf
    std::cerr.rdbuf(errlog.rdbuf()); //redirect std::cerr to serial.err!

    // Periodically check if the service is stopping.
    int rlt;
    serialPort.ctl = NULL;
    serialPort.tty = NULL;
    while (!m_fStopping)
    {
        rlt = CreateSerialPort(&serialPort);
        if (rlt == 0) {
            rlt = OpenSerialPort(&serialPort);
            if (rlt == 0) {
                // Perform main service function here...
                ReceiveCommand(&serialPort);
                continue;
            }
        }
        ::Sleep(500);  // Simulate some lengthy operations.
    }


    // Reset to standard output again
    std::cout.rdbuf(coutbuf);
    std::cerr.rdbuf(cerrbuf);

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
