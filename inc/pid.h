#ifndef PID_H_
#define PID_H_

void pid_configura_constantes(double Kp_, double Ki_, double Kd_);
void pid_imprime_constantes();
void pid_atualiza_referencia(float referencia_);
float pid_retorna_referencia();
double pid_controle(double saida_medida);

#endif /* PID_H_ */
