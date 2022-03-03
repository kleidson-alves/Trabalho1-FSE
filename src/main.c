#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "modbus.h"

int main(int argc, const char *argv[])
{
    double kp, ki, kd;
    printf("Defina o Kp: ");
    scanf("%lf", &kp);
    printf("Defina o Ki: ");
    scanf("%lf", &ki);
    printf("Defina o Kd: ");
    scanf("%lf", &kd);

    while (1)
    {
        int dado;
        if (read_modbus(0xC3, &dado) == -1)
            break;

        if (dado == 0x01)
        {
            char estado_sistema = 1;
            write_modbus(0xD3, &estado_sistema);
        }

        else if (dado == 0x02)
        {
            char estado_sistema = 0;
            write_modbus(0xD3, &estado_sistema);
        }

        else if (dado == 0x03)
        {
        }
        else if (dado == 0x04)
        {
        }
        printf("%d\n", dado);
        sleep(1);
    }

    return 0;
}
