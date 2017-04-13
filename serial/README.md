test_serial
=================

# Usage

```
//list all serial ports
> test_serial.exe -e
(COM1, ELTIMA Virtual Serial Port (COM1), EVSERIAL7)
(COM2, ELTIMA Virtual Serial Port (COM2), EVSERIAL7)

//open serial port and send data
> test_serial.exe com1 115200
Is the serial port open? Yes.
Timeout == 1000ms, asking for 1 more byte than written.
Iteration: 0, Bytes written: 8, Bytes read: 0, String read:
Iteration: 1, Bytes written: 8, Bytes read: 0, String read:
Iteration: 2, Bytes written: 8, Bytes read: 0, String read:
```