/**
 * @file    system.c
 * @brief   Initialisation minimale appelée par le code de démarrage.
 *
 * Aucune PLL n'est configurée : le cœur reste sur le HSI à 16 MHz,
 * fréquence sur laquelle sont calculés tous les diviseurs du projet.
 */
#include "board.h"

uint32_t SystemCoreClock = SYSCLK_HZ;

void SystemInit(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= (3UL << 20) | (3UL << 22);   /* accès complet aux CP10/CP11 (FPU) */
#endif
}
