#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "modbus.h"
#include "uart.h"
#include "crc.h"

int write_modbus(char subcodigo, void *buffer)
{
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;
    int tamanho = 7;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = 0x01;
    *p_tx_buffer++ = 0x16;
    *p_tx_buffer++ = subcodigo;

    if (subcodigo == 0xB1)
    {
        int *dado = buffer;
        memcpy(&tx_buffer[3], dado, 4);
    }
    else if (subcodigo == 0xB2)
    {
        float *dado = buffer;
        memcpy(&tx_buffer[3], dado, 4);
    }
    else
    {
        char *dado = buffer;
        int tam = strlen(dado);
        int i = 0;

        *p_tx_buffer++ = tam;
        for (i = 0; i < tam; i++)
        {
            *p_tx_buffer++ = dado[i];
        }
        tamanho = p_tx_buffer - &tx_buffer[0];
    }
    short crc = calcula_CRC(tx_buffer, tamanho);
    memcpy(&tx_buffer[tamanho], &crc, 2);

    init();
    uart_write(tx_buffer, tamanho + 2);
    close_uart();
    return 0;
}

void read_modbus(char subcodigo)
{
    unsigned char tx_buffer[20];
    unsigned char rx_buffer[255];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];
    *p_tx_buffer++ = 0x01;
    *p_tx_buffer++ = 0x23;
    *p_tx_buffer++ = subcodigo;
    short crc = calcula_CRC(tx_buffer, (p_tx_buffer - &tx_buffer[0]));

    memcpy(&tx_buffer[3], &crc, 2);

    init();
    uart_write(tx_buffer, 5);
    sleep(1);
    int tamanho = uart_read(rx_buffer);

    if (subcodigo == 0xA1)
    {
        int num;
        memcpy(&num, &rx_buffer[3], 4);
        printf("%i Bytes lidos : %d\n", tamanho, num);
    }
    else if (subcodigo == 0xA2)
    {
        float num;
        memcpy(&num, &rx_buffer[3], 4);
        printf("%i Bytes lidos : %f\n", tamanho, num);
    }
    else
    {
        char dado[50];
        memcpy(dado, &rx_buffer[4], rx_buffer[3]);
        printf("%i Bytes lidos : %s\n", tamanho, dado);
    }
    close_uart();
}