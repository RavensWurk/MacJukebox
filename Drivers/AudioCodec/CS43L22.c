#include "CS43L22.h"

#define CODEC_ADDRESS                   0x94  /* b00100111 */

static void _WriteReg (struct Codec* codec, uint8_t reg, uint8_t value);

bool CS43L22_Init (struct Codec* codec)
{
    codec->SetVolume = CS43L22_SetVolume;
    codec->Transmit = CS43L22_Transmit;
    codec->StopTransmit = CS43L22_StopTransmit;

    HAL_GPIO_WritePin (Audio_RST_GPIO_Port, Audio_RST_Pin, GPIO_PIN_SET);
    _WriteReg (codec, 0x02, 0x01);
    _WriteReg (codec, 0x04, 0xAF);
    _WriteReg (codec, 0x05, 0x81);
    _WriteReg (codec, 0x06, 0x04);

    CS43L22_SetVolume (codec, 150);

    _WriteReg (codec, 0x02, 0x9E);
    _WriteReg (codec, 0x0A, 0x00);
    _WriteReg (codec, 0x0E, 0x04);
    _WriteReg (codec, 0x27, 0x00);
    _WriteReg (codec, 0x1F, 0x0F);
    _WriteReg (codec, 0x1A, 0x0A);
    _WriteReg (codec, 0x1B, 0x0A);

    return true;
}

void CS43L22_SetVolume (struct Codec* codec, uint8_t volume)
{
  if (volume > 0xE6)
  {
    _WriteReg(codec, 0x20, volume - 0xE7);
    _WriteReg(codec, 0x21, volume - 0xE7);
  }
  else
  {
    _WriteReg(codec, 0x20, volume + 0x19);
    _WriteReg(codec, 0x21, volume + 0x19);
  }
}

void CS43L22_Transmit (struct Codec* codec, uint16_t* data, uint16_t len)
{
    codec->data = data;
    codec->dataLen = len;
    HAL_I2S_Transmit_DMA(codec->i2s, codec->data, codec->dataLen);
}

void CS43L22_StopTransmit (struct Codec* codec)
{
    HAL_I2S_DMAStop(codec->i2s);
    codec->data = NULL;
    codec->dataLen = 0;
}

static void _WriteReg (struct Codec* codec, uint8_t reg, uint8_t value)
{
    HAL_I2C_Mem_Write (codec->i2c, CODEC_ADDRESS, reg, 1, &value, 1, 1000);
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    struct Codec* codec = CodecGetActive();

    if (codec != NULL && hi2s == codec->i2s)
    {
        codec->DataCallback (codec, codec->data);
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    struct Codec* codec = CodecGetActive();

    if (codec != NULL && hi2s == codec->i2s)
    {
        codec->DataCallback (codec, &codec->data[codec->dataLen / 2]);
    }
}
