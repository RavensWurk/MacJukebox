#pragma once

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

enum Codecs {
  CS43L22
};

struct Codec {
    I2C_HandleTypeDef* i2c;
    I2S_HandleTypeDef* i2s;
    uint16_t* data;
    uint16_t dataLen;
    void (*SetVolume)(struct Codec*, uint8_t);
    void (*Transmit)(struct Codec*, uint16_t* data, uint16_t len);
    void (*StopTransmit)(struct Codec*);
    void (*Stop)(struct Codec*);
    void (*DataCallback)(struct Codec*, uint16_t* data);
};

bool CodecInit (enum Codecs type, struct Codec* codec);
struct Codec* CodecGetActive (void);
