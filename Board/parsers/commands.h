/**
 * This file comprises the Mac Jukebox command parser used for passing
 * commands to this device, and from the device back to the Macintosh.
 *
 * A Mac to Device/Device to Mac command is in the following format:
 *
 * * 1 start byte 0x55
 * * 1 command ID byte
 * * 4 length bytes
 * * X data bytes, specified in the length byte
 *
 * Either side may initiate communication as needed.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define COMMAND_START 0x55
#define COMMAND_HEADER_LEN 6

enum CommandID {
    COMMAND_PLAY_FILE   = 0x00,
    COMMAND_PAUSE       = 0x01,
    COMMAND_STATUS      = 0x02,
    COMMAND_STATUS_RESP = 0x03,
    COMMAND_LIST_FILES  = 0x04
};

struct Command {
    enum CommandID id;
    size_t len;
    const void* data;
};

/**
 * Checks if a valid command can be found in the data provided. If it can,
 * returns true, and optionally sets the command pointer if it's not NULL.
 */
bool Command_search(void* data, size_t len, struct Command *command);
bool Command_parse(void* data, size_t len, struct Command *command);
