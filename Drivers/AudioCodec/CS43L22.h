#pragma once

#include "codec.h"

bool CS43L22_Init (struct Codec* codec);
void CS43L22_SetVolume (struct Codec* codec, uint8_t volume);
void CS43L22_Transmit (struct Codec* codec, uint16_t* data, uint16_t len);
