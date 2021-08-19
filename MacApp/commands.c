#define TYPE_BOOL true
#include <stdbool.h>
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "command.pb.h"
#include "filelist.pb.h"
#include "serial.h"
#include <string.h>

#define TICKS_TIMEOUT 300 // Max 5 second wait between response bytes

struct FileListCallback {
    void (*callback)(FileListing_File*, void* arg);
    void* arg;
};

static bool WriteCommandToSerial(pb_ostream_t *stream, const uint8_t *buf, size_t count) {
    writeToPort((IOParam*)stream->state, buf, count);
}

static void SendCommand(const Command *command, IOParam* param) {
    pb_ostream_t serialStream = {
        .callback = WriteCommandToSerial,
        .state = param,
        .max_size = SIZE_MAX,
        .bytes_written = 0
    };

    pb_encode(&serialStream, Command_fields, &command);
}

static bool FileListingCallback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    struct FileListCallback *callbackOpts = (struct FileListCallback*)arg;

    callbackOpts->callback((FileListing_File*)field, callbackOpts->arg);

    return true;
}

static bool ReadPBCallback(pb_istream_t *stream, pb_byte_t *buf, size_t count) {
    uint32_t ticks = 0;
    size_t remainingBytes = count;

    while (ticks < TICKS_TIMEOUT) {
        size_t read = readFromPort((IOParam*)stream->state, buf, remainingBytes);

        if (read != 0) {
            ticks = 0;
            remainingBytes -= read;
        }

        if (read == count) {
            break;
        }

        unsigned long afterTicks;
        Delay(1, &afterTicks);
        ticks++;
    }

    return remainingBytes == 0;
}

void PlayFile(const char* file, IOParam* param) {
    Command command = Command_init_zero;

    command.id = Command_ID_PLAY_FILE;

    if (strlen(file) > (sizeof command.data.bytes)-1) {
        return;
    }

    strcpy((char*)command.data.bytes, file);
    SendCommand(&command, param);
}

void Pause(IOParam* param) {
    Command command = { .id = Command_ID_PAUSE };
    SendCommand(&command, param);
}

void ListFiles(IOParam* param, void (*callback)(FileListing_File*, void*), void* arg) {
    Command command = { .id = Command_ID_LIST_FILES };
    SendCommand(&command, param);

    pb_istream_t serialInStream = {
        .state = param,
        .callback = ReadPBCallback,
    };

    struct FileListCallback callbackOpts = {
        .callback = callback,
        .arg = arg
    };
    FileListing fileList = FileListing_init_zero;
    fileList.files.arg = &callbackOpts;
    fileList.files.funcs.decode = FileListingCallback;
    pb_decode_ex(&serialInStream, FileListing_fields, &fileList, PB_DECODE_DELIMITED);
}
