// NanoServerApplication.cpp : Defines the entry point for the NanoServer console application.
//

#include "stdafx.h"

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <netioapi.h>
#include <stdio.h>

#pragma comment(lib, "iphlpapi.lib")

#define HALF    (1UL << (sizeof(size_t) * 4))
void * REALLOC2(void *buf, size_t size, size_t count);
#define MALLOC2(size) REALLOC2(0, 1, (size))

void * REALLOC2(void *buf, size_t size, size_t count)
{
    void *buf2;

    if (size == 0 || count == 0) {
        free(buf);
        return NULL;
    }

    /* Check for overflow. First, check if both integers are small, which is
    * cheap on CPUs. Second, if a potential overflow exists, the check
    * using integer-division, which is expensive on CPUs. The expensive operation
    * will happen for allocations larger than 64k on 32bit processors,
    * and allocations larger than 4-gigabytes on 64bit processors */
    if (size >= HALF || count >= HALF) {
        if (count > SIZE_MAX / size)
            exit(1);
    }

    if (buf == NULL)
        buf2 = malloc(size * count);
    else
        buf2 = realloc(buf, size * count);

    if (buf2 == NULL)
        exit(1);

    return buf2;
}

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


#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
int __cdecl getAdaptersAddresses(ULONG family)
{

    /* Declare and initialize variables */

    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    unsigned int i = 0;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

//    // default to unspecified address family (both)
//    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
    IP_ADAPTER_PREFIX *pPrefix = NULL;

    /*
    if (argc != 2) {
        printf(" Usage: getadapteraddresses family\n");
        printf("        getadapteraddresses 4 (for IPv4)\n");
        printf("        getadapteraddresses 6 (for IPv6)\n");
        printf("        getadapteraddresses A (for both IPv4 and IPv6)\n");
        exit(1);
    }
   

    if (atoi(family) == 4)
        family = AF_INET;
    else if (atoi(argv[1]) == 6)
        family = AF_INET6;

    printf("Calling GetAdaptersAddresses function with family = ");
    if (family == AF_INET)
        printf("AF_INET\n");
    if (family == AF_INET6)
        printf("AF_INET6\n");
    if (family == AF_UNSPEC)
        printf("AF_UNSPEC\n\n");
    */

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
        if (pAddresses == NULL) {
            printf
            ("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
            exit(1);
        }

        dwRetVal =
            GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        while (pCurrAddresses) {
            printf("\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n",
                pCurrAddresses->Length);
            printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
            printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);

            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++)
                    pUnicast = pUnicast->Next;
                printf("\tNumber of Unicast Addresses: %d\n", i);
            }
            else
                printf("\tNo Unicast Addresses\n");

            pAnycast = pCurrAddresses->FirstAnycastAddress;
            if (pAnycast) {
                for (i = 0; pAnycast != NULL; i++)
                    pAnycast = pAnycast->Next;
                printf("\tNumber of Anycast Addresses: %d\n", i);
            }
            else
                printf("\tNo Anycast Addresses\n");

            pMulticast = pCurrAddresses->FirstMulticastAddress;
            if (pMulticast) {
                for (i = 0; pMulticast != NULL; i++)
                    pMulticast = pMulticast->Next;
                printf("\tNumber of Multicast Addresses: %d\n", i);
            }
            else
                printf("\tNo Multicast Addresses\n");

            pDnServer = pCurrAddresses->FirstDnsServerAddress;
            if (pDnServer) {
                for (i = 0; pDnServer != NULL; i++)
                    pDnServer = pDnServer->Next;
                printf("\tNumber of DNS Server Addresses: %d\n", i);
            }
            else
                printf("\tNo DNS Server Addresses\n");

            printf("\tDNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);
            printf("\tDescription: %wS\n", pCurrAddresses->Description);
            printf("\tFriendly name: %wS\n", pCurrAddresses->FriendlyName);

            if (pCurrAddresses->PhysicalAddressLength != 0) {
                printf("\tPhysical address: ");
                for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
                    i++) {
                    if (i == (pCurrAddresses->PhysicalAddressLength - 1))
                        printf("%.2X\n",
                        (int)pCurrAddresses->PhysicalAddress[i]);
                    else
                        printf("%.2X-",
                        (int)pCurrAddresses->PhysicalAddress[i]);
                }
            }
            printf("\tFlags: %ld\n", pCurrAddresses->Flags);
            printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
            printf("\tIfType: %ld\n", pCurrAddresses->IfType);
            printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
            printf("\tIpv6IfIndex (IPv6 interface): %u\n",
                pCurrAddresses->Ipv6IfIndex);
            printf("\tZoneIndices (hex): ");
            for (i = 0; i < 16; i++)
                printf("%lx ", pCurrAddresses->ZoneIndices[i]);
            printf("\n");

            printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
            printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

            pPrefix = pCurrAddresses->FirstPrefix;
            if (pPrefix) {
                for (i = 0; pPrefix != NULL; i++)
                    pPrefix = pPrefix->Next;
                printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
            }
            else
                printf("\tNumber of IP Adapter Prefix entries: 0\n");

            printf("\n");

            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n",
            dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                // Default language
                (LPTSTR)& lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    FREE(pAddresses);
                exit(1);
            }
        }
    }

    if (pAddresses) {
        FREE(pAddresses);
    }

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


int win32_list_interfaces()
{
    // Declare and initialize variables
    PIP_INTERFACE_INFO pInfo = NULL;
    DWORD ulOutBufLen = 0;

    DWORD dwRetVal = 0;
    int iReturn = 1;

    int i;

    // Make an initial call to GetInterfaceInfo to get
    // the necessary size in the ulOutBufLen variable
    dwRetVal = GetInterfaceInfo(NULL, &ulOutBufLen);
    if (dwRetVal != ERROR_INSUFFICIENT_BUFFER) {
        fprintf(stderr, "listif: unexpected condition\n");
        return 1;
    }

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
        printf("GetInterfaceInfo failed with error: %u\n", (unsigned)dwRetVal);
        iReturn = 1;
    }

    free(pInfo);
    return (iReturn);
}


/* Declare and initialize variables */

// It is possible for an adapter to have multiple
// IPv4 addresses, gateways, and secondary WINS servers
// assigned to the adapter. 
//
// Note that this sample code only prints out the 
// first entry for the IP address/mask, and gateway, and
// the primary and secondary WINS server for each adapter. 
int win32_list_adapters()
{
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    UINT i;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC2(sizeof(IP_ADAPTER_INFO));
    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC2(ulOutBufLen);
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            printf("\tComboIndex: \t%u\n", (unsigned)pAdapter->ComboIndex);
            printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
            printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
            printf("\tAdapter Addr: \t");
            for (i = 0; i < pAdapter->AddressLength; i++) {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%.2X\n", (int)pAdapter->Address[i]);
                else
                    printf("%.2X-", (int)pAdapter->Address[i]);
            }
            printf("\tIndex: \t%u\n", (unsigned)pAdapter->Index);
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
                printf("Unknown type %u\n", (unsigned)pAdapter->Type);
                break;
            }

            printf("\tIP Address: \t%s\n",
                pAdapter->IpAddressList.IpAddress.String);
            printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);

            printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
            printf("\t***\n");


            pAdapter = pAdapter->Next;
        }
    }
    else {
        printf("GetAdaptersInfo failed with error: %u\n", (unsigned)dwRetVal);

    }
    if (pAdapterInfo)
        free(pAdapterInfo);

    return 0;
}


int win32_getifentry()
{
    // Declare and initialize variables.
    // Declare and initialize variables.
    DWORD dwRetVal = 0;
    unsigned int i;
    MIB_IF_TABLE2 *pIfTable;
    MIB_IF_ROW2 *pIfRow;
    dwRetVal = GetIfTable2(&pIfTable);
    if (dwRetVal != NO_ERROR)
        return 0;
    if (pIfTable->NumEntries == 0)
        return 0;
    printf("\tNum Entries: %ld\n\n", pIfTable->NumEntries);
    for (i = 0; i < pIfTable->NumEntries; i++) {
        pIfRow = &pIfTable->Table[i];

        //printf("\tIndex:\t %ld\n", pIfRow->InterfaceIndex);
        printf("[%d]:\t ", pIfRow->InterfaceIndex);
        printf("%ws", pIfRow->Alias);

        printf("   \t(%ws)", pIfRow->Description);
        printf("\n");

        printf("\tType[%d]:\t\t ", i);
        switch (pIfRow->Type) {
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
            printf("Unknown type %ld\n", pIfRow->Type);
            break;
        }
        printf("\tMtu[%d]:\t\t %ld\n", i, pIfRow->Mtu);
        printf("\tReceiveLinkSpeed[%d]:\t\t %lld\n", i, pIfRow->ReceiveLinkSpeed);
        printf("\tTransmitLinkSpeed[%d]:\t\t %lld\n", i, pIfRow->TransmitLinkSpeed);
        printf("\tPhysical Addr:\t\t ");
        if (pIfRow->PhysicalAddressLength == 0)
            printf("\n");

        for (unsigned int j = 0; j < pIfRow->PhysicalAddressLength; j++) {
            if (j == (pIfRow->PhysicalAddressLength - 1))
                printf("%.2X\n", (int)pIfRow->PhysicalAddress[j]);
            else
                printf("%.2X-", (int)pIfRow->PhysicalAddress[j]);
        }
        printf("\tAdmin Status[%d]:\t %ld\n", i,
            pIfRow->AdminStatus);
        printf("\tOper Status[%d]:\t ", i);
        switch (pIfRow->OperStatus) {
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
                pIfRow->AdminStatus);
            break;
        }
        printf("\n");
    }
    return 0;
}


static unsigned char hexval(const char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
    if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

int
memcasecmp(const void *lhs, const void *rhs, int length)
{
    int i;
    for (i = 0; i<length; i++) {
        if (tolower(((char*)lhs)[i]) != tolower(((char*)rhs)[i]))
            return -1;
    }
    return 0;
}

GUID name_to_guid(const char *in_name)
{
    GUID result;
    char *name = (char*)in_name;
    unsigned i;
    if (memcasecmp(name, "\\Device\\NPF_{", 13) == 0)
        name += 13;

    result.Data1 = strtoul(name, &name, 16);
    if (*name == '-')
        name++;
    result.Data2 = (unsigned short)strtoul(name, &name, 16);
    if (*name == '-')
        name++;
    result.Data3 = (unsigned short)strtoul(name, &name, 16);
    if (*name == '-')
        name++;
    for (i = 0; i<8; i++) {
        if (*name == '-')
            name++;
        if (isxdigit(name[0] & 0xFF)) {
            result.Data4[i] = hexval(*name) << 4;
            name++;
        }
        if (isxdigit(name[0] & 0xFF)) {
            unsigned char x = hexval(*name);
            result.Data4[i] |= x;
            name++;
        }
    }

    return result;
}


unsigned win32_index(MIB_IF_TABLE2 *pIfTable, const char *name)
{
    unsigned i;
    GUID guid = name_to_guid(name);

    for (i = 0; i<pIfTable->NumEntries; i++) {
        MIB_IF_ROW2 *row = &pIfTable->Table[i];

        if (memcmp(&row->InterfaceGuid, &guid, sizeof(guid)) == 0) {
            return row->InterfaceIndex;
        }
    }
    return 0;
}

void win32_name(MIB_IF_TABLE2 *pIfTable, const char *name, char *namebuf, size_t sizeof_namebuf)
{
    unsigned i;
    GUID guid = name_to_guid(name);

    for (i = 0; i<pIfTable->NumEntries; i++) {
        MIB_IF_ROW2 *row = &pIfTable->Table[i];

        if (memcmp(&row->InterfaceGuid, &guid, sizeof(guid)) == 0) {
            unsigned j;
            for (j = 0; row->Alias[j] && j < sizeof_namebuf - 1; j++)
                namebuf[j] = (char)row->Alias[j];
            namebuf[j] = '\0';
        }
    }
}


int _tmain(int argc, _TCHAR* argv[])
{
    printf("\n########## BEGIN ##########\n\n");

    printf("\n========== win32_list_adapters ==========\n");
    if (win32_list_adapters() == 0) {
        printf("win32_list_adapters OK\n");
    }
    else {
        printf("win32_list_adapters failed\n");
        exit(5);
    }

    printf("\n========== win32_list_interfaces ==========\n");
    if (win32_list_interfaces() == 0) {
        printf("win32_list_interfaces OK\n");
    }
    else {
        printf("win32_list_interfaces failed\n");
        exit(6);
    }


    printf("\n========== win32_getifentry ==========\n");
    if (win32_getifentry() == 0) {
        printf("win32_getifentry OK\n");
    }
    else {
        printf("win32_getifentry failed\n");
        exit(7);
    }

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

    printf("\n========== GetAdaptersAddresses(AF_UNSPEC) ==========\n");
    if (getAdaptersAddresses(AF_UNSPEC) == 0) {
        printf("GetAdaptersAddresses(AF_UNSPEC) OK\n");
    }
    else {
        printf("GetAdaptersAddresses(AF_UNSPEC) failed\n");
        exit(40);
    }

    printf("\n========== GetAdaptersInfo ==========\n");
    if (getAdaptersInfo() == 0) {
        printf("GetAdaptersInfo OK\n");
    }
    else {
        printf("GetAdaptersInfo failed\n");
        exit(50);
    }

    printf("\n\n########## END ##########\n");

    return 0;
}

