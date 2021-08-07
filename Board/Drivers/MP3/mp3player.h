#include <stdint.h>
#include "ff.h"
#include "codec.h"
#include "cmsis_os.h"
#include "mad.h"

#define FILE_BUFFER_SIZE (1024*8)
#define MP3_BUFFER_SIZE (2*1152)
#define MP3_FRAME_SIZE 2881
#define CODEC_BUFFER_SIZE (MP3_BUFFER_SIZE*2)

enum MP3CommandType {
    MP3_PLAY_FILE,
    MP3_PAUSE
};

struct MP3Command {
    enum MP3CommandType type;
    uint8_t data[100];
};

struct MP3FilePlayCommand {
    char fileName[50];
};

struct MP3Player {
    FIL currentFile;
    struct mad_decoder decoder;
    struct Codec *codec;
    uint8_t fileData[FILE_BUFFER_SIZE];
    uint16_t codecBuffer[CODEC_BUFFER_SIZE];
    uint16_t codecBackBuffer[CODEC_BUFFER_SIZE / 2];
    osThreadId_t task;
    StaticTask_t taskBuffer;
    uint32_t taskStack[2048];
    osMessageQueueId_t commandQueue;
    StaticQueue_t commandQueueBuffer;
    uint8_t commandQueueData[sizeof (struct MP3Command) * 3];
    bool startedPlaying;
    bool isPlaying;
};

int MP3PlayerInit(struct MP3Player* player);
int MP3PlayerPlayFile(struct MP3Player* player, const char* fileName);
void MP3PlayerTick (struct MP3Player* player);
void MP3PlayerPause(struct MP3Player* player);
