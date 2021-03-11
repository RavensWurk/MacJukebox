#include <stdint.h>
#include "ff.h"
#include "codec.h"
#include "cmsis_os.h"
#include "mad.h"

#define FILE_BUFFER_SIZE (1024*8)
#define MP3_BUFFER_SIZE (2*1152)
#define MP3_FRAME_SIZE 2881
#define CODEC_BUFFER_SIZE (MP3_BUFFER_SIZE*2)

struct MP3Player {
    FIL currentFile;
    struct mad_decoder decoder;
    int32_t position;
    struct Codec *codec;
    uint8_t fileData[FILE_BUFFER_SIZE];
    uint8_t* fileReadPtr;
    int bytesLeft;
    uint16_t outBuffer[MP3_BUFFER_SIZE];
    uint16_t codecBuffer[CODEC_BUFFER_SIZE];
    uint16_t codecBackBuffer[CODEC_BUFFER_SIZE / 2];
    osThreadId_t task;
};

int MP3PlayerInit(struct MP3Player* player);
int MP3PlayerPlayFile(struct MP3Player* player, const char* fileName);
void MP3PlayerTick (struct MP3Player* player);
