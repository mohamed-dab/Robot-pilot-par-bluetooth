/**
 * @file    bluetooth.c
 * @brief   USART3 (PB10 = TX, PB11 = RX, AF7), 9600 bauds @ 16 MHz.
 *
 *   BRR = 16 MHz / 9600 = 1666,7  ->  1667
 *   CR1 : UE(13) | RXNEIE(5) | IDLEIE(4) | TE(3) | RE(2)
 */
#include "board.h"
#include "bluetooth.h"

static char             rx_buf[8];
static volatile uint8_t rx_idx      = 0;
static volatile uint8_t bt_received = 0;
static volatile char    bt_cmd      = 'S';

void USART3_Init(void)
{
    RCC->AHB1ENR |= (1 << 1);    /* GPIOB  */
    RCC->APB1ENR |= (1 << 18);   /* USART3 */

    GPIOB->MODER  &= ~((3 << 20) | (3 << 22));
    GPIOB->MODER  |=  ((2 << 20) | (2 << 22));          /* mode AF */
    GPIOB->AFR[1] &= ~((0xF << 8) | (0xF << 12));
    GPIOB->AFR[1] |=  ((7 << 8) | (7 << 12));           /* AF7 = USART3 */

    USART3->BRR = 1667;
    USART3->CR1 = (1 << 13) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);

    NVIC_EnableIRQ(USART3_IRQn);
}

void USART3_IRQHandler(void)
{
    /* RXNE : un octet est arrivé, on l'accumule */
    if (USART3->SR & (1 << 5))
    {
        char c = (char)(USART3->DR & 0xFF);   /* la lecture de DR efface RXNE */
        if (rx_idx < sizeof(rx_buf) - 1)
            rx_buf[rx_idx++] = c;
    }

    /* IDLE : fin de trame. Effacement par lecture de SR puis de DR. */
    if (USART3->SR & (1 << 4))
    {
        (void)USART3->SR;
        (void)USART3->DR;

        if (rx_idx > 0)
        {
            bt_cmd      = rx_buf[0];
            bt_received = 1;
            rx_idx      = 0;
        }
    }
}

uint8_t bluetooth_get_command(char *cmd)
{
    if (!bt_received)
        return 0;
    bt_received = 0;
    *cmd = bt_cmd;
    return 1;
}
