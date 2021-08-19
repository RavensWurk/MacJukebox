#include <stddef.h>
#include <stdbool.h>
#include <Files.h>
#include "pb.h"
#include "filelist.pb.h"

void PlayFile(const char* file, IOParam* param);
void Pause(IOParam* param);
void ListFiles(IOParam* param, void (*callback)(FileListing_File*, void*), void* arg);
