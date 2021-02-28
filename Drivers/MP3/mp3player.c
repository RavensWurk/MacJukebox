#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mp3player.h"

static struct MP3Player* currentPlayer;

static void MP3DataCallback(struct Codec* codec, uint16_t* data)
{
    memcpy (data, currentPlayer->codecBackBuffer, sizeof (currentPlayer->codecBackBuffer));
    osThreadFlagsSet(currentPlayer->task, 2);
}

int MP3PlayerInit(struct MP3Player* player)
{
    player->decoder = MP3InitDecoder();

    if (player == NULL)
    {
        return -1;
    }

    currentPlayer = player;
    player->codec->DataCallback = MP3DataCallback;

    return 0;
}

static unsigned int _LoadFileBuffer (struct MP3Player* player)
{
    unsigned int bytesRead = 0;

    if (player->bytesLeft < 0)
    {
        player->bytesLeft = 0;
    }
    memmove(player->fileData, player->fileReadPtr, player->bytesLeft);
    f_read (&player->currentFile, player->fileData + player->bytesLeft, FILE_BUFFER_SIZE - player->bytesLeft, &bytesRead);

    player->bytesLeft += bytesRead;
    player->fileReadPtr = player->fileData;

    return bytesRead;
}

static bool _DecodeNextFrame (struct MP3Player* player)
{
    bool decoded = false;

    do {
        player->position = MP3FindSyncWord(player->fileReadPtr, player->bytesLeft);

        if (player->position < 0)
        {
            if (_LoadFileBuffer(player) > 0)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        player->bytesLeft -= player->position;
        player->fileReadPtr += player->position;

        int err = MP3Decode(player->decoder, &player->fileReadPtr, &player->bytesLeft, (short*)player->outBuffer, 0);

        if (!err)
        {
            decoded = true;
        }
    } while (!decoded);

    return decoded;
}

int MP3PlayerPlayFile(struct MP3Player* player, const char* fileName)
{
    if (f_open(&player->currentFile, fileName, FA_READ) != FR_OK)
    {
        return -1;
    }

    unsigned int bytesRead = 0;
    if (f_read(&player->currentFile, player->fileData, FILE_BUFFER_SIZE, &bytesRead) != FR_OK)
    {
        return -2;
    }

    player->position = MP3FindSyncWord(player->fileData, bytesRead);
    player->bytesLeft = bytesRead - player->position;
    player->fileReadPtr = &player->fileData[player->position];

    if (player->position < 0)
    {
        return -3;
    }

    for (int i=0;i<CODEC_BUFFER_SIZE / MP3_BUFFER_SIZE; i++)
    {
        int err = MP3Decode(player->decoder, &player->fileReadPtr, &player->bytesLeft, (short*)player->outBuffer, 0);

        if (err)
        {
            return -4;
        }

        memcpy (&player->codecBuffer[i * MP3_BUFFER_SIZE], player->outBuffer, MP3_BUFFER_SIZE);
    }

    player->codec->Transmit (player->codec, player->codecBuffer, CODEC_BUFFER_SIZE);

    return 0;
}

void MP3PlayerTick (struct MP3Player* player)
{
    _DecodeNextFrame(player);
    memcpy(player->codecBackBuffer, player->outBuffer, sizeof (player->outBuffer));
}
