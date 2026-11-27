/**
 * @file    sensors.c
 * @brief   ADC1 + DMA2 Stream0 déclenché par TIM2, et capteur DS1621 sur I2C1.
 */
#include "board.h"
#include "sensors.h"

volatile uint16_t adc_value[3];
int8_t ds1621_temperature = 0;
static volatile uint8_t adc_ready = 0;

/* ====================================================================== */
/*                     DMA2 Stream0, canal 0 = ADC1                       */
/* ====================================================================== */
void DMA2_Stream0_ADC1_Init(void)
{
    RCC->AHB1ENR |= (1 << 22);                 /* DMA2 */

    DMA2_Stream0->CR &= ~(1 << 0);
    while (DMA2_Stream0->CR & (1 << 0));
    DMA2->LIFCR = 0x3D;                        /* efface les flags du stream 0 */

    DMA2_Stream0->PAR  = (uint32_t)&ADC1->DR;
    DMA2_Stream0->M0AR = (uint32_t)adc_value;
    DMA2_Stream0->NDTR = 3;

    DMA2_Stream0->CR  = 0;
    DMA2_Stream0->CR |= (1 << 10);             /* MINC          */
    DMA2_Stream0->CR |= (1 << 8);              /* CIRC          */
    DMA2_Stream0->CR |= (1 << 4);              /* TCIE          */
    DMA2_Stream0->CR |= (1 << 11);             /* PSIZE 16 bits */
    DMA2_Stream0->CR |= (1 << 13);             /* MSIZE 16 bits */

    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    DMA2_Stream0->CR |= (1 << 0);              /* EN */
}

void DMA2_Stream0_IRQHandler(void)
{
    if (DMA2->LISR & (1 << 5))                 /* TCIF0 */
    {
        DMA2->LIFCR = (1 << 5);
        adc_ready = 1;
    }
}

uint8_t sensors_adc_ready(void)
{
    if (!adc_ready)
        return 0;
    adc_ready = 0;
    return 1;
}

/* ====================================================================== */
/*          ADC1 : 3 voies (CH10 = PC0, CH11 = PC1, CH12 = PC2)           */
/* ====================================================================== */
void ADC1_DMA_Init(void)
{
    RCC->AHB1ENR |= (1 << 2);                  /* GPIOC */
    RCC->APB2ENR |= (1 << 8);                  /* ADC1  */

    GPIOC->MODER |= (3 << 0) | (3 << 2) | (3 << 4);   /* PC0..PC2 analogiques */

    ADC->CCR &= ~(3 << 16);
    ADC->CCR |=  (3 << 16);                    /* ADCPRE : PCLK2 / 8 */

    ADC1->CR1 = 0;
    ADC1->CR2 = 0;
    ADC1->CR1 |= (1 << 8);                     /* SCAN   */
    ADC1->CR1 |= (1 << 11);                    /* DISCEN */

    ADC1->SQR1 &= ~(0xF << 20);
    ADC1->SQR1 |=  (2 << 20);                  /* 3 conversions */
    ADC1->SQR3  = (10 << 0) | (11 << 5) | (12 << 10);
    ADC1->SMPR1 |= (5 << 0) | (5 << 3) | (5 << 6);    /* 112 cycles */

    ADC1->CR2 |= (6 << 24);                    /* EXTSEL = TIM2_TRGO */
    ADC1->CR2 |= (1 << 28);                    /* EXTEN  = front montant */
    ADC1->CR2 |= (1 << 8);                     /* DMA    */
    ADC1->CR2 |= (1 << 9);                     /* DDS    */
    ADC1->CR2 |= (1 << 10);                    /* EOCS   */
    ADC1->CR2 |= (1 << 0);                     /* ADON   */
}

/* ====================================================================== */
/*         TIM2 : TRGO sur événement de mise à jour, 16 MHz / 16000 / 1000 = 1 Hz */
/* ====================================================================== */
void TIM2_TRGO_Init(void)
{
    RCC->APB1ENR |= (1 << 0);
    TIM2->PSC = 15999;
    TIM2->ARR = 999;
    TIM2->CR2 &= ~(7 << 4);
    TIM2->CR2 |=  (2 << 4);                    /* MMS = Update -> TRGO */
    TIM2->CR1 |= (1 << 0);
}

/* ====================================================================== */
/*                 I2C1 + DS1621 (adresse 0x48 -> 0x90 / 0x91)            */
/* ====================================================================== */
void config_I2C1(void)
{
    RCC->AHB1ENR |= (1 << 1);                  /* GPIOB */
    RCC->APB1ENR |= (1 << 21);                 /* I2C1  */
    GPIOB->MODER  |= (2 << 12) | (2 << 14);    /* PB6, PB7 en AF */
    GPIOB->OTYPER |= (1 << 6) | (1 << 7);      /* drain ouvert */
    GPIOB->AFR[0] |= (4 << 24) | (4 << 28);    /* AF4 = I2C1 */

    I2C1->CR1   = (1 << 15);                   /* reset logiciel */
    I2C1->CR1   = 0;
    I2C1->CR2   = 16;                          /* PCLK1 = 16 MHz */
    I2C1->CCR   = 80;                          /* 100 kHz, mode standard */
    I2C1->TRISE = 17;
    I2C1->CR1  |= (1 << 0);                    /* PE */
}

void I2C_write_DS1621(uint8_t data)
{
    I2C1->CR1 |= (1 << 8);                     /* START */
    while (!(I2C1->SR1 & (1 << 0)));
    I2C1->DR = 0x90;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;
    I2C1->DR = data;
    while (!(I2C1->SR1 & (1 << 7)));
    I2C1->CR1 |= (1 << 9);                     /* STOP */
}

/* Registre de configuration (0xAC) = 0x00 : conversion continue */
void I2C_config_DS1621(void)
{
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));
    I2C1->DR = 0x90;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;
    I2C1->DR = 0xAC;
    while (!(I2C1->SR1 & (1 << 7)));
    I2C1->DR = 0x00;
    while (!(I2C1->SR1 & (1 << 7)));
    I2C1->CR1 |= (1 << 9);
}

/* Lecture de l'octet de poids fort de la température (commande 0xAA) */
void I2C_read_DS1621(void)
{
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));
    I2C1->DR = 0x90;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;
    I2C1->DR = 0xAA;
    while (!(I2C1->SR1 & (1 << 7)));

    I2C1->CR1 |= (1 << 8);                     /* START répété */
    while (!(I2C1->SR1 & (1 << 0)));
    I2C1->DR = 0x91;
    while (!(I2C1->SR1 & (1 << 1)));
    I2C1->CR1 &= ~(1 << 10);                   /* NACK : un seul octet lu */
    (void)I2C1->SR2;
    I2C1->CR1 |= (1 << 9);
    while (!(I2C1->SR1 & (1 << 6)));
    ds1621_temperature = (int8_t)I2C1->DR;
}
