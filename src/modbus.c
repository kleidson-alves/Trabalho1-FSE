#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "modbus.h"
#include "uart.h"
#include "crc.h"

int write_modbus(char subcodigo, void* buffer) {
    unsigned char tx_buffer[20];
    unsigned char* p_tx_buffer;
    int tamanho;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = 0x01;
    *p_tx_buffer++ = 0x16;
    *p_tx_buffer++ = subcodigo;
    *p_tx_buffer++ = 3;
    *p_tx_buffer++ = 8;
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 1;

    if (subcodigo == 0xD3 || subcodigo == 0xD4) {
        memcpy(&tx_buffer[7], buffer, sizeof(char));
        tamanho = p_tx_buffer - &tx_buffer[0] + sizeof(char);
    }
    else {
        memcpy(&tx_buffer[7], buffer, sizeof(float));
        tamanho = p_tx_buffer - &tx_buffer[0] + sizeof(float);
    }

    short crc = calcula_CRC(tx_buffer, tamanho);
    memcpy(&tx_buffer[tamanho], &crc, sizeof(short));

    init();
    uart_write(tx_buffer, tamanho + sizeof(short));
    close_uart();
    return 0;
}

int read_modbus(char subcodigo, void* buffer) {
    unsigned char tx_buffer[20];
    unsigned char rx_buffer[255];
    unsigned char* p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = 0x01;
    *p_tx_buffer++ = 0x23;
    *p_tx_buffer++ = subcodigo;
    *p_tx_buffer++ = 3;
    *p_tx_buffer++ = 8;
    *p_tx_buffer++ = 6;
    *p_tx_buffer++ = 1;

    short crc = calcula_CRC(tx_buffer, (p_tx_buffer - &tx_buffer[0]));
    memcpy(&tx_buffer[7], &crc, sizeof(short));

    init();
    uart_write(tx_buffer, p_tx_buffer - &tx_buffer[0] + sizeof(short));
    sleep(1);

    int i, cond = 0, tamanho;
    for (i = 0; i < 5; i++) {
        short rx_crc;
        tamanho = uart_read(rx_buffer);
        memcpy(&rx_crc, &rx_buffer[tamanho - 2], sizeof(short));

        if (calcula_CRC(rx_buffer, tamanho - 2) == rx_crc) {
            cond = 1;
            break;
        }
    }
    if (cond == 0)
        return -1;

    int* dado = buffer;

    memcpy(dado, &rx_buffer[3], sizeof(int));

    close_uart();
    return tamanho;
}