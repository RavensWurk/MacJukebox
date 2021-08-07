#include "parsers/commands.h"
#include "command.pb-c.h"
#include <string.h>

bool Command_search(void* data, size_t len, struct Command *command)
{
    for (int i=0;i<len;i++)
    {
        if (*(uint8_t*)data+i == COMMAND_START)
        {
            return Command_parse(data+i, len - i, command);
        }
    }

    return false;
}

bool Command_parse(void* data, size_t len, struct Command *command)
{
    uint8_t *commData = (uint8_t*)data;

    if (commData[0] != COMMAND_START)
    {
        return false;
    }

    enum CommandID commId = commData[1];
    size_t commLen;
    memcpy(&commLen, &commData[2], 4);

    if (commLen != 0 && commLen > (len - COMMAND_HEADER_LEN))
    {
        return false;
    }

    command->id = commId;
    command->len = commLen;

    if (command->len != 0)
    {
        command->data = data + COMMAND_HEADER_LEN;
    }

    return true;
}
