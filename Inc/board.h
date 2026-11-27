/**
 * @file    board.h
 * @brief   Constantes matérielles communes (horloge, PWM, sujets MQTT).
 *
 * Le microcontrôleur tourne sur l'oscillateur interne HSI à 16 MHz
 * (configuration au reset, aucune PLL) : tous les calculs de BRR,
 * PSC et ARR ci-dessous en dépendent.
 */
#ifndef BOARD_H
#define BOARD_H

#include "stm32f4xx.h"
#include <stdint.h>

#define SYSCLK_HZ            16000000UL

/* ---------- Moteurs ---------- */
#define PWM_MAX              999      /* ARR de TIM3 : rapport cyclique 0..999 */
#define MOTOR_SPEED_DEFAULT  800
#define TURN_INNER_PERCENT   40       /* vitesse du côté intérieur en virage (%) */

/* ---------- MQTT ---------- */
#define MQTT_LINK_ID         0
#define MQTT_TOPIC_POT1      "stm32/adc/pot1"
#define MQTT_TOPIC_POT2      "stm32/adc/pot2"
#define MQTT_TOPIC_POT3      "stm32/adc/pot3"
#define MQTT_TOPIC_TEMP      "stm32/temp"

#endif /* BOARD_H */
