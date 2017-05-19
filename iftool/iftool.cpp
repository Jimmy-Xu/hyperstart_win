// NanoServerApplication.cpp : Defines the entry point for the NanoServer console application.
//

#include "stdafx.h"

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x)) 
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/* Note: could also use malloc() and free() */

int getIfTable()
{

    // Declare and initialize variables.

    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i, j;

    /* variables used for GetIfTable and GetIfEntry */
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *)MALLOC(sizeof(MIB_IFTABLE));
    if (pIfTable == NULL) {
        printf("Error allocating memory needed to call GetIfTable\n");
        return 1;
    }
    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof(MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *)MALLOC(dwSize);
        if (pIfTable == NULL) {
            printf("Error allocating memory needed to call GetIfTable\n");
            return 1;
        }
    }
    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, FALSE)) == NO_ERROR) {
        printf("\tNum Entries: %ld\n\n", pIfTable->dwNumEntries);
        for (i = 0; i < pIfTable->dwNumEntries; i++) {
            pIfRow = (MIB_IFROW *)& pIfTable->table[i];
            printf("\tIndex[%d]:\t %ld\n", i, pIfRow->dwIndex);
            printf("\tInterfaceName[%d]:\t %ws", i, pIfRow->wszName);
            printf("\n");
            printf("\tDescription[%d]:\t ", i);
            for (j = 0; j < pIfRow->dwDescrLen; j++)
                printf("%c", pIfRow->bDescr[j]);
            printf("\n");
            printf("\tType[%d]:\t ", i);
            switch (pIfRow->dwType) {
            case IF_TYPE_OTHER:
                printf("Other\n");
                break;
            case IF_TYPE_ETHERNET_CSMACD:
                printf("Ethernet\n");
                break;
            case IF_TYPE_ISO88025_TOKENRING:
                printf("Token Ring\n");
                break;
            case IF_TYPE_PPP:
                printf("PPP\n");
                break;
            case IF_TYPE_SOFTWARE_LOOPBACK:
                printf("Software Lookback\n");
                break;
            case IF_TYPE_ATM:
                printf("ATM\n");
                break;
            case IF_TYPE_IEEE80211:
                printf("IEEE 802.11 Wireless\n");
                break;
            case IF_TYPE_TUNNEL:
                printf("Tunnel type encapsulation\n");
                break;
            case IF_TYPE_IEEE1394:
                printf("IEEE 1394 Firewire\n");
                break;
            default:
                printf("Unknown type %ld\n", pIfRow->dwType);
                break;
            }
            printf("\tMtu[%d]:\t\t %ld\n", i, pIfRow->dwMtu);
            printf("\tSpeed[%d]:\t %ld\n", i, pIfRow->dwSpeed);
            printf("\tPhysical Addr:\t ");
            if (pIfRow->dwPhysAddrLen == 0)
                printf("\n");
            for (j = 0; j < pIfRow->dwPhysAddrLen; j++) {
                if (j == (pIfRow->dwPhysAddrLen - 1))
                    printf("%.2X\n", (int)pIfRow->bPhysAddr[j]);
                else
                    printf("%.2X-", (int)pIfRow->bPhysAddr[j]);
            }
            printf("\tAdmin Status[%d]:\t %ld\n", i, pIfRow->dwAdminStatus);
            printf("\tOper Status[%d]:\t ", i);
            switch (pIfRow->dwOperStatus) {
            case IF_OPER_STATUS_NON_OPERATIONAL:
                printf("Non Operational\n");
                break;
            case IF_OPER_STATUS_UNREACHABLE:
                printf("Unreachable\n");
                break;
            case IF_OPER_STATUS_DISCONNECTED:
                printf("Disconnected\n");
                break;
            case IF_OPER_STATUS_CONNECTING:
                printf("Connecting\n");
                break;
            case IF_OPER_STATUS_CONNECTED:
                printf("Connected\n");
                break;
            case IF_OPER_STATUS_OPERATIONAL:
                printf("Operational\n");
                break;
            default:
                printf("Unknown status %ld\n", pIfRow->dwAdminStatus);
                break;
            }
            printf("\n");
        }
    }
    else {
        printf("GetIfTable failed with error: %ld\n", dwRetVal);
        if (pIfTable != NULL) {
            FREE(pIfTable);
            pIfTable = NULL;
        }
        return 1;
        // Here you can use FormatMessage to find out why 
        // it failed.
    }
    if (pIfTable != NULL) {
        FREE(pIfTable);
        pIfTable = NULL;
    }
    return 0;
}


int getIfEntry()
{

    // Declare and initialize variables.

    // Declare and initialize variables.
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i, j;

    // variables used for GetIfTable and GetIfEntry
    MIB_IFTABLE *pIfTable;
    MIB_IFROW *pIfRow;

    // Allocate memory for our pointers.
    pIfTable = (MIB_IFTABLE *)MALLOC(sizeof(MIB_IFTABLE));
    if (pIfTable == NULL) {
        printf("Error allocating memory needed to call GetIfTable\n");
        return 1;
    }
    // Before calling GetIfEntry, we call GetIfTable to make
    // sure there are entries to get and retrieve the interface index.

    // Make an initial call to GetIfTable to get the
    // necessary size into dwSize
    dwSize = sizeof(MIB_IFTABLE);
    if (GetIfTable(pIfTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER) {
        FREE(pIfTable);
        pIfTable = (MIB_IFTABLE *)MALLOC(dwSize);
        if (pIfTable == NULL) {
            printf("Error allocating memory\n");
            return 1;
        }
    }
    // Make a second call to GetIfTable to get the actual
    // data we want.
    if ((dwRetVal = GetIfTable(pIfTable, &dwSize, 0)) == NO_ERROR) {
        if (pIfTable->dwNumEntries > 0) {
            pIfRow = (MIB_IFROW *)MALLOC(sizeof(MIB_IFROW));
            if (pIfRow == NULL) {
                printf("Error allocating memory\n");
                if (pIfTable != NULL) {
                    FREE(pIfTable);
                    pIfTable = NULL;
                }
                return 1;
            }

            printf("\tNum Entries: %ld\n\n", pIfTable->dwNumEntries);
            for (i = 0; i < pIfTable->dwNumEntries; i++) {
                pIfRow->dwIndex = pIfTable->table[i].dwIndex;
                if ((dwRetVal = GetIfEntry(pIfRow)) == NO_ERROR) {
                    printf("\tIndex:\t %ld\n", pIfRow->dwIndex);
                    printf("\tInterfaceName[%d]:\t ", i);
                    if (pIfRow->wszName != NULL)
                        printf("%ws", pIfRow->wszName);
                    printf("\n");

                    printf("\tDescription[%d]:\t ", i);
                    for (j = 0; j < pIfRow->dwDescrLen; j++)
                        printf("%c", pIfRow->bDescr[j]);
                    printf("\n");

                    printf("\tIndex[%d]:\t\t %d\n", i, pIfRow->dwIndex);

                    printf("\tType[%d]:\t\t ", i);
                    switch (pIfRow->dwType) {
                    case IF_TYPE_OTHER:
                        printf("Other\n");
                        break;
                    case IF_TYPE_ETHERNET_CSMACD:
                        printf("Ethernet\n");
                        break;
                    case IF_TYPE_ISO88025_TOKENRING:
                        printf("Token Ring\n");
                        break;
                    case IF_TYPE_PPP:
                        printf("PPP\n");
                        break;
                    case IF_TYPE_SOFTWARE_LOOPBACK:
                        printf("Software Lookback\n");
                        break;
                    case IF_TYPE_ATM:
                        printf("ATM\n");
                        break;
                    case IF_TYPE_IEEE80211:
                        printf("IEEE 802.11 Wireless\n");
                        break;
                    case IF_TYPE_TUNNEL:
                        printf("Tunnel type encapsulation\n");
                        break;
                    case IF_TYPE_IEEE1394:
                        printf("IEEE 1394 Firewire\n");
                        break;
                    default:
                        printf("Unknown type %ld\n", pIfRow->dwType);
                        break;
                    }

                    printf("\tMtu[%d]:\t\t %ld\n", i, pIfRow->dwMtu);

                    printf("\tSpeed[%d]:\t\t %ld\n", i, pIfRow->dwSpeed);

                    printf("\tPhysical Addr:\t\t ");
                    if (pIfRow->dwPhysAddrLen == 0)
                        printf("\n");
                    //                    for (j = 0; j < (int) pIfRow->dwPhysAddrLen; j++) {
                    for (j = 0; j < pIfRow->dwPhysAddrLen; j++) {
                        if (j == (pIfRow->dwPhysAddrLen - 1))
                            printf("%.2X\n", (int)pIfRow->bPhysAddr[j]);
                        else
                            printf("%.2X-", (int)pIfRow->bPhysAddr[j]);
                    }
                    printf("\tAdmin Status[%d]:\t %ld\n", i,
                        pIfRow->dwAdminStatus);

                    printf("\tOper Status[%d]:\t ", i);
                    switch (pIfRow->dwOperStatus) {
                    case IF_OPER_STATUS_NON_OPERATIONAL:
                        printf("Non Operational\n");
                        break;
                    case IF_OPER_STATUS_UNREACHABLE:
                        printf("Unreasonable\n");
                        break;
                    case IF_OPER_STATUS_DISCONNECTED:
                        printf("Disconnected\n");
                        break;
                    case IF_OPER_STATUS_CONNECTING:
                        printf("Connecting\n");
                        break;
                    case IF_OPER_STATUS_CONNECTED:
                        printf("Connected\n");
                        break;
                    case IF_OPER_STATUS_OPERATIONAL:
                        printf("Operational\n");
                        break;
                    default:
                        printf("Unknown status %ld\n",
                            pIfRow->dwAdminStatus);
                        break;
                    }
                    printf("\n");
                }

                else {
                    printf("GetIfEntry failed for index %d with error: %ld\n",
                        i, dwRetVal);
                    // Here you can use FormatMessage to find out why 
                    // it failed.

                }
            }
        }
        else {
            printf("\tGetIfTable failed with error: %ld\n", dwRetVal);
        }

    }

    return 0;
}



int __cdecl getAdaptersInfo()
{

    // Declare and initialize variables //

    // It is possible for an adapter to have multiple
    // IPv4 addresses, gateways, and secondary WINS servers
    // assigned to the adapter. 
    //
    // Note that this sample code only prints out the 
    // first entry for the IP address/mask, and gateway, and
    // the primary and secondary WINS server for each adapter. 

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    UINT i;

    // variables used to print DHCP time info //
    struct tm newtime;
    char buffer[32];
    errno_t error;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return 1;
    }
    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return 1;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            printf("\tComboIndex: \t%d\n", pAdapter->ComboIndex);
            printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
            printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
            printf("\tAdapter Addr: \t");
            for (i = 0; i < pAdapter->AddressLength; i++) {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%.2X\n", (int)pAdapter->Address[i]);
                else
                    printf("%.2X-", (int)pAdapter->Address[i]);
            }
            printf("\tIndex: \t%d\n", pAdapter->Index);
            printf("\tType: \t");
            switch (pAdapter->Type) {
            case MIB_IF_TYPE_OTHER:
                printf("Other\n");
                break;
            case MIB_IF_TYPE_ETHERNET:
                printf("Ethernet\n");
                break;
            case MIB_IF_TYPE_TOKENRING:
                printf("Token Ring\n");
                break;
            case MIB_IF_TYPE_FDDI:
                printf("FDDI\n");
                break;
            case MIB_IF_TYPE_PPP:
                printf("PPP\n");
                break;
            case MIB_IF_TYPE_LOOPBACK:
                printf("Lookback\n");
                break;
            case MIB_IF_TYPE_SLIP:
                printf("Slip\n");
                break;
            default:
                printf("Unknown type %ld\n", pAdapter->Type);
                break;
            }

            printf("\tIP Address: \t%s\n",
                pAdapter->IpAddressList.IpAddress.String);
            printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);

            printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
            printf("\t***\n");

            if (pAdapter->DhcpEnabled) {
                printf("\tDHCP Enabled: Yes\n");
                printf("\t  DHCP Server: \t%s\n",
                    pAdapter->DhcpServer.IpAddress.String);

                printf("\t  Lease Obtained: ");
                // Display local time //
                error = _localtime32_s(&newtime, (__time32_t*)&pAdapter->LeaseObtained);
                if (error)
                    printf("Invalid Argument to _localtime32_s\n");
                else {
                    // Convert to an ASCII representation 
                    error = asctime_s(buffer, 32, &newtime);
                    if (error)
                        printf("Invalid Argument to asctime_s\n");
                    else
                        // asctime_s returns the string terminated by \n\0 //
                        printf("%s", buffer);
                }

                printf("\t  Lease Expires:  ");
                error = _localtime32_s(&newtime, (__time32_t*)&pAdapter->LeaseExpires);
                if (error)
                    printf("Invalid Argument to _localtime32_s\n");
                else {
                    // Convert to an ASCII representation 
                    error = asctime_s(buffer, 32, &newtime);
                    if (error)
                        printf("Invalid Argument to asctime_s\n");
                    else
                        // asctime_s returns the string terminated by \n\0 //
                        printf("%s", buffer);
                }
            }
            else
                printf("\tDHCP Enabled: No\n");

            if (pAdapter->HaveWins) {
                printf("\tHave Wins: Yes\n");
                printf("\t  Primary Wins Server:    %s\n",
                    pAdapter->PrimaryWinsServer.IpAddress.String);
                printf("\t  Secondary Wins Server:  %s\n",
                    pAdapter->SecondaryWinsServer.IpAddress.String);
            }
            else
                printf("\tHave Wins: No\n");
            pAdapter = pAdapter->Next;
            printf("\n");
        }
    }
    else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);

    }
    if (pAdapterInfo)
        FREE(pAdapterInfo);

    return 0;
}


int __cdecl getInterfaceInfo() {
    // Declare and initialize variables
    PIP_INTERFACE_INFO pInfo = NULL;
    ULONG ulOutBufLen = 0;

    DWORD dwRetVal = 0;
    int iReturn = 1;

    int i;

    // Make an initial call to GetInterfaceInfo to get
    // the necessary size in the ulOutBufLen variable
    dwRetVal = GetInterfaceInfo(NULL, &ulOutBufLen);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pInfo = (IP_INTERFACE_INFO *)MALLOC(ulOutBufLen);
        if (pInfo == NULL) {
            printf
            ("Unable to allocate memory needed to call GetInterfaceInfo\n");
            return 1;
        }
    }
    // Make a second call to GetInterfaceInfo to get
    // the actual data we need
    dwRetVal = GetInterfaceInfo(pInfo, &ulOutBufLen);
    if (dwRetVal == NO_ERROR) {
        printf("Number of Adapters: %ld\n\n", pInfo->NumAdapters);
        for (i = 0; i < pInfo->NumAdapters; i++) {
            printf("Adapter Index[%d]: %ld\n", i,
                pInfo->Adapter[i].Index);
            printf("Adapter Name[%d]: %ws\n\n", i,
                pInfo->Adapter[i].Name);
        }
        iReturn = 0;
    }
    else if (dwRetVal == ERROR_NO_DATA) {
        printf
        ("There are no network adapters with IPv4 enabled on the local system\n");
        iReturn = 0;
    }
    else {
        printf("GetInterfaceInfo failed with error: %d\n", dwRetVal);
        iReturn = 1;
    }

    FREE(pInfo);
    return (iReturn);
}


int _tmain(int argc, _TCHAR* argv[])
{
    printf("\n########## BEGIN ##########\n\n");

    printf("\n========== GetIfTable ==========\n");
    if (getIfTable() == 0) {
        printf("GetIfTable OK\n");
    }
    else {
        printf("GetIfTable failed\n");
        exit(10);
    }

    printf("\n========== GetIfEntry ==========\n");
    if (getIfEntry() == 0) {
        printf("GetIfEntry OK\n");
    }
    else {
        printf("GetIfEntry failed\n");
        exit(20);
    }

    printf("\n========== GetInterfaceInfo ==========\n");
    if (getInterfaceInfo() == 0) {
        printf("GetInterfaceInfo OK\n");
    }
    else {
        printf("GetInterfaceInfo failed\n");
        exit(30);
    }

    printf("\n========== GetAdaptersInfo ==========\n");
    if (getAdaptersInfo() == 0) {
        printf("GetAdaptersInfo OK\n");
    }
    else {
        printf("GetAdaptersInfo failed\n");
        exit(40);
    }

    printf("\n\n########## END ##########\n");

    return 0;
}

