#include "serial/serial.h"
#pragma once

struct SerialPort {
    serial::Serial *ctl;
    serial::Serial *tty;
};

#define READY_STR "Ready"
#define MAX_CMD_LENGTH 255

/* Enable SerialPort COM1 and COM2 */
void EnsureSerialPort();

void ExportRegistry(char* cmd, char* subKey);

void ExecuteWMIC();

/* List all SerialPort */
void EnumerateSerialPorts();

/* Start Send Data via SerialPort */
int CreateSerialPort(struct SerialPort *serialPort);
int OpenSerialPort(struct SerialPort *serialPort);
int SendReadyStr(serial::Serial *pPort, char *pName);
int ReceiveCommand(struct SerialPort *serialPort);
int SendCmdResult(SerialPort *serialPort, const char *cmd);
