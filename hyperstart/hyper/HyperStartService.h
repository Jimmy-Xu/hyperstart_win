/****************************** Module Header ******************************\
* Module Name:  SampleService.h
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

#pragma once

#include "ServiceBase.h"
#include "SerialPortTTY.h"
#include "serial/serial.h"
#include <fstream>


class CHyperStartService : public CServiceBase
{
public:

    CHyperStartService(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
    virtual ~CHyperStartService(void);

protected:

    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();

    void ServiceWorkerThread(void);

    struct SerialPort serialPort;

private:

    BOOL m_fStopping;
    HANDLE m_hStoppedEvent;

#ifdef UNICODE
    using tofstream = std::wofstream;
#else
    using tofstream = std::ofstream;
#endif

    tofstream m_logFile;

};