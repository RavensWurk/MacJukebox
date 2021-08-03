/*
     Copyright 2014 Wolfgang Thaller.

     This file is part of Retro68.

     Retro68 is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     Retro68 is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with Retro68.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <Devices.h>
#include <Serial.h>
#include <parsers/commands.h>

void writeToPort(IOParam *port, const void* data, size_t len) {
    port->ioBuffer = (void*)data;
    port->ioReqCount = len;
    PBWrite((ParmBlkPtr)port, 0);
}

void writeCommand(IOParam *port, const struct Command* command) {
    uint8_t startByte = COMMAND_START;

    writeToPort(port, &startByte, 1);
    writeToPort(port, &command->id, 1);
    writeToPort(port, &command->len, 4);
    writeToPort(port, command->data, command->len);
}

int main(int argc, char** argv) {
    IOParam outPort;
    OpenDriver(".A", &outPort.ioRefNum);

    CntrlParam cb;
    cb.ioCRefNum = outPort.ioRefNum;
    cb.csCode = 8;
    cb.csParam[0] = stop10 | noParity | data8 | baud19200;
    OSErr err = PBControl ((ParmBlkPtr) & cb, 0);

    const char* playFile = "song.mp3";
    struct Command playCmd = {
        .id = COMMAND_PLAY_FILE,
        .len = strlen(playFile),
        .data = playFile
    };

    writeCommand(&outPort, &playCmd);

    CloseDriver(outPort.ioRefNum);

    return 0;
}
