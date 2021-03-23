#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mp3player.h"
#include "FreeRTOS.h"

static struct MP3Player* currentPlayer;

static void PlayFile(struct MP3Player* player, const char* fileName);
static void PlayNextFrame(struct MP3Player* player);

static void MP3DataCallback(struct Codec* codec, uint16_t* data)
{
    memcpy (data, currentPlayer->codecBackBuffer, sizeof (currentPlayer->codecBackBuffer));
    osThreadFlagsSet(currentPlayer->task, 2);
}

static enum mad_flow MP3DataInput(void *data, struct mad_stream *stream)
{
    struct MP3Player* player = (struct MP3Player*)data;
    int keep; /* Number of bytes to keep from the previous buffer. */
    int len; /* Length of the new buffer. */
    int eof; /* Whether this is the last buffer that we can provide. */

    /* Figure out how much data we need to move from the end of the previous
       buffer into the start of the new buffer. */
    if (stream->error != MAD_ERROR_BUFLEN) {
        /* All data has been consumed, or this is the first call. */
        keep = 0;
    } else if (stream->next_frame != NULL) {
        /* The previous buffer was consumed partially. Move the unconsumed portion
           into the new buffer. */
        keep = stream->bufend - stream->next_frame;
    } else if ((stream->bufend - stream->buffer) < FILE_BUFFER_SIZE) {
        /* No data has been consumed at all, but our read buffer isn't full yet,
           so let's just read more data first. */
        keep = stream->bufend - stream->buffer;
    } else {
        /* No data has been consumed at all, and our read buffer is already full.
           Shift the buffer to make room for more data, in such a way that any
           possible frame position in the file is completely in the buffer at least
           once. */
        keep = FILE_BUFFER_SIZE - MP3_FRAME_SIZE;
    }

    /* Shift the end of the previous buffer to the start of the new buffer if we
       want to keep any bytes. */
    if (keep) {
        memmove(player->fileData, stream->bufend - keep, keep);
    }

    /* Append new data to the buffer. */
    unsigned int bytesRead = 0;
    FRESULT result = f_read(&player->currentFile, player->fileData + keep, FILE_BUFFER_SIZE - keep, &bytesRead);
    if (result != FR_OK) {
        /* Read error. */
        return MAD_FLOW_STOP;
    } else if (bytesRead == 0) {
        /* End of file. Append MAD_BUFFER_GUARD zero bytes to make sure that the
           last frame is properly decoded. */
        if (keep + MAD_BUFFER_GUARD <= FILE_BUFFER_SIZE) {
            /* Append all guard bytes and stop decoding after this buffer. */
            memset(player->fileData + keep, 0, MAD_BUFFER_GUARD);
            len = keep + MAD_BUFFER_GUARD;
            eof = 1;
        } else {
            /* The guard bytes don't all fit in our buffer, so we need to continue
               decoding and write all fo teh guard bytes in the next call to input(). */
            memset(player->fileData + keep, 0, FILE_BUFFER_SIZE - keep);
            len = FILE_BUFFER_SIZE;
            eof = 0;
        }
    } else {
        /* New buffer length is amount of bytes that we kept from the previous
           buffer plus the bytes that we read just now. */
        len = keep + bytesRead;
        eof = 0;
    }

    /* Pass the new buffer information to libmad. */
    mad_stream_buffer(stream, player->fileData, len);
    return eof ? MAD_FLOW_STOP : MAD_FLOW_CONTINUE;
}

static signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow MP3DataOutput(void *data, struct mad_header const *header,
		                    struct mad_pcm *pcm)
{
    struct MP3Player* player = (struct MP3Player*)data;

    int codecPos = 0;
    for (int i=0;i<pcm->length;i++)
    {
        player->codecBackBuffer[codecPos++] = scale(pcm->samples[0][i]);
        player->codecBackBuffer[codecPos++] = scale(pcm->samples[1][i]);
    }

    if (!player->startedPlaying)
    {
        player->codec->Transmit(player->codec, player->codecBuffer, CODEC_BUFFER_SIZE, header->samplerate);
        player->startedPlaying = true;
    }

    osThreadFlagsWait(2, osFlagsWaitAny, 60000);

    return MAD_FLOW_CONTINUE;
}

static enum mad_flow MP3DataError(void *data, struct mad_stream *stream,
		                   struct mad_frame *frame)
{
    return MAD_FLOW_CONTINUE;
}

static void MP3PlayerTask(void* arg)
{
    struct MP3Player *player = (struct MP3Player*)arg;
    while (true)
    {
        struct MP3Command command = {0};

        osStatus_t stat = osMessageQueueGet(player->commandQueue, &command, NULL, 0);

        if (stat == osOK)
        {
            switch (command.type)
            {
                case MP3_PLAY_FILE:
                    {
                        struct MP3FilePlayCommand *playCommand = (struct MP3FilePlayCommand*)&command.data;
                        PlayFile(player, playCommand->fileName);
                    }
                    break;
                case MP3_PAUSE:
                    player->isPlaying = false;
                    break;
                default:
                    break;
            }
        }

        if (player->isPlaying)
        {
            PlayNextFrame(player);
        }
    }
}

int MP3PlayerInit(struct MP3Player* player)
{
    currentPlayer = player;
    player->codec->DataCallback = MP3DataCallback;

    mad_decoder_init(&player->decoder, player, MP3DataInput, 0, 0,
                     MP3DataOutput, MP3DataError, 0);

    const osMessageQueueAttr_t queueAttrs = {
        .name = "MP3CommandQueue",
        .cb_mem = &player->commandQueueBuffer,
        .cb_size = sizeof(player->commandQueueBuffer),
        .mq_mem = player->commandQueueData,
        .mq_size = sizeof(player->commandQueueData)
    };
    player->commandQueue = osMessageQueueNew(3, sizeof (struct MP3Command), &queueAttrs);

    const osThreadAttr_t taskAttrs = {
        .name = "MP3Player",
        .stack_mem = player->taskStack,
        .stack_size = sizeof(player->taskStack),
        .cb_mem = &player->taskBuffer,
        .cb_size = sizeof(player->taskBuffer),
        .priority = osPriorityNormal
    };
    player->task = osThreadNew(MP3PlayerTask, player, &taskAttrs);

    return 0;
}

static void PlayNextFrame(struct MP3Player* player)
{
    struct mad_stream *stream = &player->decoder.sync->stream;
    struct mad_frame *frame = &player->decoder.sync->frame;
    struct mad_synth *synth = &player->decoder.sync->synth;

    if (!MAD_RECOVERABLE(stream->error) && player->decoder.input_func(player->decoder.cb_data, stream) == MAD_FLOW_STOP)
    {
        player->isPlaying = false;
        return;
    }

    if (mad_frame_decode(frame, stream) == -1)
    {
        // bad don't output
        return;
    }

    mad_synth_frame(synth, frame);
    player->decoder.output_func(player->decoder.cb_data, &frame->header, &synth->pcm);
}

static void PlayFile(struct MP3Player* player, const char* fileName)
{
    if (f_open(&player->currentFile, fileName, FA_READ) != FR_OK)
    {
        return;
    }

    if (player->decoder.sync != NULL)
    {
        mad_synth_finish(&player->decoder.sync->stream);
        mad_frame_finish(&player->decoder.sync->frame);
        mad_stream_finish(&player->decoder.sync->stream);
        free (player->decoder.sync);
        player->decoder.sync = NULL;
    }
    player->decoder.sync = malloc(sizeof(*player->decoder.sync));

    struct mad_stream *stream = &player->decoder.sync->stream;
    struct mad_frame *frame = &player->decoder.sync->frame;
    struct mad_synth *synth = &player->decoder.sync->synth;

    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_options(stream, player->decoder.options);

    player->isPlaying = true;

    return;
}

int MP3PlayerPlayFile(struct MP3Player* player, const char* fileName)
{
    struct MP3Command command = {
        .type = MP3_PLAY_FILE
    };
    struct MP3FilePlayCommand* playCommand = (struct MP3FilePlayCommand*)&command.data;

    snprintf(playCommand->fileName, sizeof(playCommand->fileName), "%s", fileName);

    osMessageQueuePut(player->commandQueue, &command, 0, 0);

    return 1;
}
