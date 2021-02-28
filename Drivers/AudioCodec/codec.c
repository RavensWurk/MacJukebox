#include "codec.h"
#include "CS43L22.h"
#include "main.h"
#include <stdint.h>

struct Codec* activeCodec = NULL;

bool CodecInit (enum Codecs type, struct Codec* codec)
{
    if (type == CS43L22)
    {
        activeCodec = codec;
        return CS43L22_Init(codec);
    }
    return false;
}

struct Codec* CodecGetActive (void)
{
    return activeCodec;
}
