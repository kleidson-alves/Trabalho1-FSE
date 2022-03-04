#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "modbus.h"
#include "pid.h"
#include "wiringPi.h"
#include "softPwm.h"

void menu()
{
    printf("(1) Temperatura de Referência pelo teclado");
}

void config_param()
{
    double kp, ki, kd;
    printf("Defina o Kp: ");
    scanf("%lf", &kp);
    printf("Defina o Ki: ");
    scanf("%lf", &ki);
    printf("Defina o Kd: ");
    scanf("%lf", &kd);

    pid_configura_constantes(kp, ki, kd);
}

void potenciometro()
{
    float temp_interna, temp_potenciometro;
    int PWM_ventoinha = 5;
    int PWM_resistor = 4;

    if (wiringPiSetup() != -1)
    {
        pinMode(PWM_ventoinha, OUTPUT);
        softPwmCreate(PWM_ventoinha, 40, 100);

        pinMode(PWM_resistor, OUTPUT);
        softPwmCreate(PWM_resistor, 1, 100);
        while (1)
        {
            int dado;
            if (read_modbus(0xC3, &dado) != -1 && dado == 0x02)
            {
                char estado_sistema = 0;
                write_modbus(0xD3, &estado_sistema);
                printf("O sistema está sendo desligado...");
                softPwmStop(PWM_resistor);
                softPwmStop(PWM_ventoinha);
                break;
            }

            if (read_modbus(0xC2, &temp_potenciometro) != -1)
            {
                pid_atualiza_referencia(temp_potenciometro);
                write_modbus(0xD2, &temp_potenciometro);
            }

            if (read_modbus(0xC1, &temp_interna) != -1)
            {
                int sinal_controle = pid_controle(temp_interna);

                write_modbus(0xD1, &sinal_controle);

                if (sinal_controle >= 0)
                {
                    printf("Acionando o resistor\n");

                    softPwmWrite(PWM_resistor, sinal_controle);
                }
                else
                {
                    sinal_controle *= -1;
                    if (sinal_controle < 40)
                    {
                        sinal_controle = 40;
                    }
                    printf("Acionando a ventoinha\n");

                    softPwmWrite(PWM_ventoinha, sinal_controle);
                }
            }

            sleep(1);
        }
    }
    else
    {
        printf("Nao foi possivel incializar a wiringPi");
    }
}

int main(int argc, const char *argv[])
{
    config_param();

    // int opcao;
    // scanf("%d", &opcao);

    // if (opcao == 1)
    // {
    //     float referencia;
    //     scanf("%f", &referencia);
    //     pid_atualiza_referencia(referencia);
    // }

    while (1)
    {
        int dado;
        if (read_modbus(0xC3, &dado) == -1)
            break;

        if (dado == 0x01)
        {
            printf("Ligando o sistema...\n");
            char estado_sistema = 1;
            write_modbus(0xD3, &estado_sistema);
            dado = 0x03;
        }

        else if (dado == 0x02)
        {
            printf("O sistema está sendo desligado...");
            char estado_sistema = 0;
            write_modbus(0xD3, &estado_sistema);
        }

        if (dado == 0x03)
        {
            printf("Modo de controle pelo potenciometro");
            char modo_controle = 0;
            write_modbus(0xD4, &modo_controle);
            potenciometro();
        }
        else if (dado == 0x04)
        {
            printf("Modo de controle pela curva reflow");
            char modo_controle = 1;
            write_modbus(0xD4, &modo_controle);
        }
        sleep(1);
    }

    return 0;
}
