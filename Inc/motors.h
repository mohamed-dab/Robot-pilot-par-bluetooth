/**
 * @file    motors.h
 * @brief   Commande des 4 moteurs par PWM (TIM3, 4 canaux, 1 kHz).
 */
#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>

typedef enum {
    ETAT_STOP,
    ETAT_AVANT,
    ETAT_ARRIERE,
    ETAT_GAUCHE,
    ETAT_DROITE
} EtatMoteur;

void TIM3_PWM_Init(void);

void motor_forward(void);
void motor_backward(void);
void motor_left(void);
void motor_right(void);
void motor_stop(void);

/** Règle la vitesse (0..PWM_MAX) et l'applique à l'état courant. */
void motor_set_speed(uint16_t speed);

/** Interprète une commande reçue en Bluetooth (F, B, L, R, S, 0..9, q). */
void cmd_ROBOT(char cmd);

#endif /* MOTORS_H */
