#ifndef _UART_
#define _UART_

int init();
int uart_write(unsigned char *mensagem, int tamanho);
int uart_read(unsigned char *mensagem);
void close_uart();

#endif