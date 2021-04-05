#include "stm32f4xx_hal.h"
#include "parsers/commands.h"

void UartInterface_Init(UART_HandleTypeDef *uart);
void UartInterface_Received(uint8_t data);
void UartInterface_OnCommand(void (*callback)(struct Command*));
