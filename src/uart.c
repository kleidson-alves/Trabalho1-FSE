#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "uart.h"

static int uart0_filestream = -1;

int init() {

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart0_filestream == -1) {
        // printf("Erro - Não foi possível iniciar a UART.\n");
        return -1;
    }
    else {
        // printf("UART inicializada!\n");
    }
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    return 0;
}

int uart_write(unsigned char* mensagem, int tamanho) {
    if (uart0_filestream != -1) {
        // printf("Escrevendo caracteres na UART ...");
        int count = write(uart0_filestream, &mensagem[0], tamanho);
        if (count < 0) {
            // printf("UART TX error\n");
            return -1;
        }
        else {
            // printf("escrito.\n");
        }
    }
    return 0;
}

int uart_read(unsigned char* mensagem) {
    int tamanho;
    if (uart0_filestream != -1) {
        tamanho = read(uart0_filestream, (void*)mensagem, 255);
        if (tamanho < 0) {
            // printf("Erro na leitura.\n");
            return -1;
        }
        else if (tamanho == 0) {
            // printf("Nenhum dado disponível.\n");
            return -1;
        }
    }
    return tamanho;
}

void close_uart() {
    close(uart0_filestream);
}