#ifndef UART_COMM_H
#define UART_COMM_H

#include <stdbool.h>
#include <stddef.h>

extern volatile bool is_running;

void uart_comm_send_flow(float flow_rate);
int uart_comm_receive_command(char *buffer, size_t max_len);
void uart_comm_init(void);

#endif // UART_COMM_H
