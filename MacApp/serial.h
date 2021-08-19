#include <MacTypes.h>
#include <Files.h>

void writeToPort(IOParam *port, const void* data, size_t len);
size_t readFromPort(IOParam *port, const void* data, size_t len);
OSErr SerialInit(ConstStr255Param port, IOParam* param);
