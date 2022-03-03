#include <stdio.h>
#include <string.h>
#include "modbus.h"

void menu()
{
    printf("(1) Solicitar int\n");
    printf("(2) Solicitar float\n");
}

int main(int argc, const char *argv[])
{
    double kp, ki, kd;
    printf("Defina o Kp: ");
    scanf("%lf", &kp);
    printf("Defina o Ki: ");
    scanf("%lf", &ki);
    printf("Defina o Kd: ");
    scanf("%lf", &kd);

    return 0;
}
