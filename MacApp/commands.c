#define TYPE_BOOL true
#include <stdbool.h>
#include "pb.h"
#include "pb_encode.h"
#include "command.pb.h"
#include "serial.h"
#include <string.h>

static bool WriteComamndToSerial(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
    writeToPort((IOParam*)stream->state, buf, count);
}

void PlayFile(const char* file, IOParam* param) {
    Command command = Command_init_zero;

    command.id = Command_ID_PLAY_FILE;

    if (strlen(file) > (sizeof command.data.bytes)-1) {
        return;
    }

    strcpy((char*)command.data.bytes, file);

    pb_ostream_t serialStream = {
        .callback = WriteComamndToSerial,
        .state = param,
        .max_size = SIZE_MAX,
        .bytes_written = 0
    };

    pb_encode(&serialStream, Command_fields, &command);
}

void Pause(IOParam* param) {
    Command command = Command_init_zero;

    command.id = Command_ID_PAUSE;

    pb_ostream_t serialStream = {
        .callback = WriteComamndToSerial,
        .state = param,
        .max_size = SIZE_MAX,
        .bytes_written = 0
    };

    pb_encode(&serialStream, Command_fields, &command);
}
