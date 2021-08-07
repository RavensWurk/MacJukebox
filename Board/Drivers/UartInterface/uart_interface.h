#include "stm32f4xx_hal.h"
#include "parsers/commands.h"

void UartInterface_Init(UART_HandleTypeDef *uart);
void UartInterface_Received(uint8_t data);
void UartInterface_OnCommand(void (*callback)(Command*));
void UartInterface_Write(UART_HandleTypeDef *uart, const void* data, size_t len);
