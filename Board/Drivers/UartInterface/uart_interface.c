#include "uart_interface.h"

#define MAX_CALLBACKS 10

void (*callbacks[MAX_CALLBACKS])(struct Command*);

// Temporary storage for incoming commands
char uart_data[256];
size_t current_len;

void UartInterface_Received(uint8_t data)
{
    uart_data[current_len++] = data;

    struct Command command = {0};

    if (current_len > COMMAND_HEADER_LEN &&
        Command_search(uart_data, current_len, &command))
    {
        for (int i=0;i<MAX_CALLBACKS;i++)
        {
            if (callbacks[i] != NULL)
            {
                callbacks[i](&command);
            }
        }
        current_len = 0;

    }
    else if (current_len == sizeof (uart_data) - 1)
    {
        current_len = 0;
    }
}

void UartInterface_Init(UART_HandleTypeDef *uart)
{
    __HAL_UART_ENABLE_IT(uart, UART_IT_RXNE);
}

void UartInterface_OnCommand(void (*callback)(struct Command*))
{
    for (int i=0;i<MAX_CALLBACKS;i++)
    {
        if (callbacks[i] == NULL)
        {
            callbacks[i] = callback;
            return;
        }
    }
}
