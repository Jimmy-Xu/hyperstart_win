#pragma once

void EnsureSerialPort(int nPort);
int SerialPortCommunicate(char* cPort, char* cBaud);

void EnumeratePorts();