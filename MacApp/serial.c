#include <Devices.h>
#include <Serial.h>
#include "serial.h"

void writeToPort(IOParam *port, const void* data, size_t len) {
    port->ioBuffer = (void*)data;
    port->ioReqCount = len;
    PBWrite((ParmBlkPtr)port, false);
}

size_t readFromPort(IOParam *port, const void* data, size_t len) {
    port->ioBuffer = (void*)data;
    port->ioReqCount = len;
    PBRead((ParmBlkPtr)port, false);

    return port->ioActCount;
}

OSErr SerialInit(ConstStr255Param port, IOParam* param) {
    OpenDriver(port, &param->ioRefNum);

    CntrlParam cb;
    cb.ioCRefNum = param->ioRefNum;
    cb.csCode = 8;
    cb.csParam[0] = stop10 | noParity | data8 | baud9600;
    return PBControl ((ParmBlkPtr)&cb, 0);
}
