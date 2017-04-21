#pragma once

/* Enable SerialPort COM1 and COM2 */
void EnsureSerialPort();

void ExportRegistry(char* cmd, char* subKey);

void ExecuteWMIC();

/* List all SerialPort */
void EnumerateSerialPorts();

/* Start Send Data via SerialPort */
int SerialPortCommunicate(char* cPort, char* cBaud);