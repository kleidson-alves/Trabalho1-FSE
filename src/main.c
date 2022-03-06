#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "modbus.h"
#include "pid.h"
#include "wiringPi.h"
#include "softPwm.h"
#include "bme280.h"
#include "bme_userspace.h"
#include "lcd.h"

#define POTENCIOMETRO 0
#define CURVA 1
#define ARQUIVO 1
#define TERMINAL 2
#define RESISTOR 4
#define VENTOINHA 5

struct Dado_Arquivo {
    int tempo;
    float temp;
}Dado_Arquivo;

struct Dado_Arquivo dados[50];
int cont_tempo = -1;
int pos = 0;

float temp_int = 0;

void config_param() {
    double kp, ki, kd;
    printf("Defina o Kp: ");
    scanf("%lf", &kp);
    printf("Defina o Ki: ");
    scanf("%lf", &ki);
    printf("Defina o Kd: ");
    scanf("%lf", &kd);

    pid_configura_constantes(kp, ki, kd);
}

void atualizar_parametros() {
    char escolha, lixo;
    printf("Parâmetros atuais:\n");
    pid_imprime_constantes();
    printf("\n------------------------------------\n\n");
    printf("Deseja configurar novos parâmetros? (s/n)\n");
    scanf("%c", &lixo);
    while (1) {

        scanf("%c", &escolha);
        if (escolha == 's' || escolha == 'S') {
            config_param();
            printf("Parametros atualizados com sucesso\n");
            break;
        }
        else if (escolha == 'n' || escolha == 'N')
            break;
        else
            printf("Entrada invalida\n");
    }
}

void config_temp() {
    float referencia;
    printf("Temperatura de referencia: ");
    scanf("%f", &referencia);

    pid_atualiza_referencia(referencia);
    write_modbus(0xD2, &referencia);
}

void apresentar_temperaturas(char modo) {
    ClrLcd();
    if (modo == TERMINAL)
        typeln("TERMINAL");
    else
        typeln("UART");

    typeln(" Ti:");
    typeFloat(temp_int);
    lcdLoc(LINE2);
    typeln("Te:");
    typeFloat(get_temperatura());
    typeln(" Tr:");
    typeFloat(pid_retorna_referencia());
}

void carrega_arquivo() {
    FILE* fp = fopen("curva_reflow.csv", "r");
    if (!fp) {
        printf("nao foi possivel abrir o arquivo csv\n");
    }

    char buffer[1024];
    int row = 0;
    int column = 0;
    while (fgets(buffer, 1024, fp)) {
        column = 0;
        row++;

        if (row == 1)
            continue;

        char* value = strtok(buffer, ", ");
        while (value) {
            if (column == 0) {
                dados[row - 2].tempo = strtof(value, NULL);
            }

            if (column == 1) {
                dados[row - 2].temp = strtof(value, NULL);
            }

            value = strtok(NULL, ", ");
            column++;
        }
    }
    fclose(fp);
}

void inicia_arquivo(char* nome_arquivo) {
    FILE* fpt;
    fpt = fopen(nome_arquivo, "w+");
    fprintf(fpt, "Data, Hora, Temperatura Interna, Temperatura Externa, Temperatura Usuario, Valor Acionamento\n");
    fclose(fpt);
}

void salvar_dados_arquivo(char modo, int valor_acionamento) {
    FILE* fpt;
    struct tm* data_hora;
    time_t segundos;

    time(&segundos);
    data_hora = localtime(&segundos);

    if (modo == TERMINAL)
        fpt = fopen("terminal.csv", "a");
    else if (modo == CURVA)
        fpt = fopen("curva.csv", "a");
    else
        fpt = fopen("potenciometro.csv", "a");

    fprintf(fpt, "%d/%d/%d, %d:%d:%d,", data_hora->tm_mday, data_hora->tm_mon + 1, data_hora->tm_year + 1900, data_hora->tm_hour, data_hora->tm_min, data_hora->tm_sec);
    fprintf(fpt, "%.2lf, ", temp_int);

    double temp_ext = get_temperatura();
    double temp_ref = pid_retorna_referencia();

    fprintf(fpt, "%.2lf, %.2lf, %d\n", temp_ext, temp_ref, valor_acionamento);

    fclose(fpt);
}

void controla_ambiente(int* dispositivo) {
    float dado;
    if (read_modbus(0xC1, &dado) != -1) {
        temp_int = dado;
        int sinal_controle = pid_controle(temp_int);
        write_modbus(0xD1, &sinal_controle);
        if (sinal_controle >= 0) {
            // if (*dispositivo < 0) {
            //     printf("Resistor acionado\n");
            // }

            softPwmWrite(RESISTOR, sinal_controle);
        }
        else {
            // if (*dispositivo > 0) {
            //     printf("Ventoinha acionada\n");
            // }
            if (sinal_controle > -40) {
                sinal_controle = -40;
            }

            softPwmWrite(VENTOINHA, sinal_controle * -1);
            softPwmWrite(RESISTOR, 0);
        }
        *dispositivo = sinal_controle;
    }
}

void curva_reflow(int modo, int* dispositivo) {

    if (modo == ARQUIVO) {
        if (cont_tempo == dados[pos].tempo) {
            printf("Atualizando temperatura. . .\n");
            double temp_ref = dados[pos].temp;
            printf("Atualizada para --> %.2lf\n", temp_ref);
            pos++;
            printf("Nova temperatura no segundo %d\n", dados[pos].tempo);
            pid_atualiza_referencia(temp_ref);
            write_modbus(0xD2, &temp_ref);
        }
    }
    controla_ambiente(dispositivo);
}

void potenciometro(int* dispositivo) {
    float temp_potenciometro;
    if (read_modbus(0xC2, &temp_potenciometro) != -1) {
        pid_atualiza_referencia(temp_potenciometro);
        write_modbus(0xD2, &temp_potenciometro);
        controla_ambiente(dispositivo);
    }
}

void controle(char modo) {
    char modo_controle = modo;
    int dispositivo = -1;
    write_modbus(0xD4, &modo_controle);
    bme_init();

    if (modo == TERMINAL) {
        printf("Controle pela referência dada pelo terminal\n");
        modo_controle = 1;
        write_modbus(0xD4, &modo_controle);
    }
    else if (modo == CURVA)
        printf("Controle por Curva Reflow\n");
    else
        printf("Controle por potenciometro\n");

    if (wiringPiSetup() != -1) {
        pinMode(VENTOINHA, OUTPUT);
        softPwmCreate(VENTOINHA, 40, 100);

        pinMode(RESISTOR, OUTPUT);
        softPwmCreate(RESISTOR, 1, 100);

        while (1) {
            int dado;
            if (read_modbus(0xC3, &dado) != -1) {
                if (dado == 0x02) {
                    printf("O sistema está sendo desligado...\n");
                    softPwmStop(RESISTOR);
                    softPwmStop(VENTOINHA);
                    break;
                }
                else if (dado == 0x03) {
                    modo = POTENCIOMETRO;
                    write_modbus(0xD4, &modo);
                    cont_tempo = -1;
                    pos = 0;
                    printf("Controle alterado para potenciometro\n");

                }
                else if (dado == 0x04) {
                    modo = CURVA;
                    carrega_arquivo();
                    write_modbus(0xD4, &modo);
                    printf("Controle alterado para Curva Reflow\n");
                }
            }

            if (modo == POTENCIOMETRO) {
                potenciometro(&dispositivo);
            }
            else if (modo == CURVA) {
                cont_tempo++;
                curva_reflow(ARQUIVO, &dispositivo);
            }
            else if (modo == TERMINAL) {
                curva_reflow(TERMINAL, &dispositivo);
            }

            salvar_dados_arquivo(modo, dispositivo);
            apresentar_temperaturas(modo);
        }
    }
    else {
        printf("Nao foi possivel incializar a wiringPi");
    }
}

void menu() {
    int escolha, dado;
    while (1) {
        ClrLcd();
        typeln("Aguardando ...");
        printf("1 - Parametros\n2 - Definir temperatura de referencia\n3 - Potenciometro/Curva Reflow\n");
        scanf("%d", &escolha);

        if (escolha == 1)
            atualizar_parametros();
        else if (escolha == 2) {
            config_temp();
            controle(TERMINAL);
            break;
        }
        else if (escolha == 3) {
            controle(POTENCIOMETRO);
            break;
        }

        if (read_modbus(0xC3, &dado) != -1 && dado == 0x02) {
            printf("O sistema está sendo desligado...\n");
            break;
        }

        sleep(1);
    }
}

void trata_sinal(int sig) {
    softPwmStop(RESISTOR);
    softPwmStop(VENTOINHA);
    bme_stop();
    ClrLcd();
    typeln("Desligado");
    lcd_stop();
    char estado_sistema = 0;
    write_modbus(0xD3, &estado_sistema);
    exit(0);
}

void inicializacao() {
    printf("Para inciar, defina os parametros para o calculo do PID\n");
    config_param();

    printf("Aguardando o sistema ser ligado...\n");

    inicia_arquivo("terminal.csv");
    inicia_arquivo("curva.csv");
    inicia_arquivo("potenciometro.csv");
    lcd_setup();
    ClrLcd();
    typeln("Desligado");
    bme_init();
}

int main(int argc, const char* argv[]) {

    inicializacao();
    signal(SIGINT, trata_sinal);

    while (1) {
        int dado;
        if (read_modbus(0xC3, &dado) != -1) {
            if (dado == 0x01) {
                printf("Ligando o sistema...\n");
                char estado_sistema = 1;
                write_modbus(0xD3, &estado_sistema);
                menu();
                estado_sistema = 0;
                write_modbus(0xD3, &estado_sistema);
                ClrLcd();
                typeln("Desligado");
                printf("Aguardando o sistema ser ligado...\n");
            }
        }

        sleep(1);
    }
    return 0;
}