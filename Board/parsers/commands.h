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
#include "command.pb.h"

/**
 * Checks if a valid command can be found in the data provided. If it can,
 * returns true, and optionally sets the command pointer if it's not NULL.
 */
bool Command_search(void* data, size_t len, Command *command);
