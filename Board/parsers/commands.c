#include "parsers/commands.h"
#include "command.pb.h"
#include "pb_decode.h"
#include <string.h>

bool Command_search(void* data, size_t len, Command *command)
{
    pb_istream_t stream = pb_istream_from_buffer(data, len);

    return pb_decode(&stream, Command_fields, command);
}
