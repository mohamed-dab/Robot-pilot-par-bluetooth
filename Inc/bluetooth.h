/**
 * @file    bluetooth.h
 * @brief   Réception des commandes Bluetooth (HC-05/HC-06) sur USART3.
 *
 * Les octets sont accumulés sous interruption RXNE ; la fin de trame est
 * détectée par l'interruption IDLE (ligne inactive), ce qui permet de traiter
 * une commande dès qu'elle est complète, sans attente active.
 */
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>

void USART3_Init(void);

/**
 * @brief  Récupère la dernière commande reçue.
 * @param  cmd  premier caractère de la trame reçue
 * @return 1 si une nouvelle commande est disponible, 0 sinon
 */
uint8_t bluetooth_get_command(char *cmd);

#endif /* BLUETOOTH_H */
