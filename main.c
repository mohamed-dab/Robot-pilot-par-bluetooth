/*
written by: 		D.Mohamed

*/


#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>
/* ===== Variables ===== */
char Buffer_RX[50];
int i = 0;
uint16_t pot1, pot2, pot3;
char msg[50];
float temperature;
/* ===================================================== */
/* ================= GPIO CONFIG ======================= */
/* ===================================================== */

/* --- GPIO pour USART3 : PB10 TX / PB11 RX --- */
void config_GPIO_USART3(void)
{
    RCC->AHB1ENR |= (1 << 1);              // Enable GPIOB

    GPIOB->MODER &= ~((3<<20)|(3<<22));    // Clear mode
    GPIOB->MODER |=  (2<<20)|(2<<22);      // Alternate function

    GPIOB->AFR[1] &= ~((0xF<<8)|(0xF<<12));
    GPIOB->AFR[1] |=  (7<<8)|(7<<12);      // AF7 USART3
}
void config_GPIOD()
{
	RCC->AHB1ENR |= 0x00000008;
	GPIOD->MODER |= 0x55<<24;
}

/* --- GPIO pour TIM3 PWM : PB0 PB1 PB4 PB5 --- */
void config_GPIO_TIM3(void)
{
    RCC->AHB1ENR |= (1 << 1);              // Enable GPIOB

    GPIOB->MODER &= ~((3<<0)|(3<<2)|(3<<8)|(3<<10));
    GPIOB->MODER |=  (2<<0)|(2<<2)|(2<<8)|(2<<10);

    GPIOB->AFR[0] &= ~((0xF<<0)|(0xF<<4)|(0xF<<16)|(0xF<<20));
    GPIOB->AFR[0] |=  (2<<0)|(2<<4)|(2<<16)|(2<<20); // AF2 TIM3
}

/* ===================================================== */
/* ================= PERIPHERALS ======================= */
/* ===================================================== */

/* --- USART3 --- */
void config_USART3(void)
{   
	 
    RCC->APB1ENR |= (1 << 18);             // USART3 clock

    USART3->BRR = 0x683;                   // 9600 baud
    USART3->CR1 |= (1<<2)|(1<<3);          // RX + TX
    USART3->CR1 |= (1<<5)|(1<<4);          // RXNE + IDLE interrupt
	  
    NVIC_EnableIRQ(USART3_IRQn);
	  USART3->CR1 |= (1<<13);                // USART enable
}

/* --- TIM3 PWM --- */
void config_TIM3_PWM(void)
{
    RCC->APB1ENR |= (1 << 1);              // TIM3 clock

    TIM3->PSC = 15;
    TIM3->ARR = 1000;

    TIM3->CCMR1 |= (0x60<<0)|(0x60<<8);
    TIM3->CCMR2 |= (0x60<<0)|(0x60<<8);

    TIM3->CCER |= (1<<0)|(1<<4)|(1<<8)|(1<<12);
  	TIM3->CCMR1 |= (1<<3) | (1<<11);  // OC1PE OC2PE
    TIM3->CCMR2 |= (1<<3) | (1<<11);  // OC3PE OC4PE


    TIM3->CR1 |= 1;
	  GPIOD->ODR = 0x4000;       // led rouge 
}
void config_GPIO_ADC(void)
{
    RCC->AHB1ENR |= (1<<0); // GPIOA clock

    // PA0 PA1 PA2 en mode analogique
    GPIOA->MODER |= (3<<0) | (3<<2) | (3<<4);
}
void config_ADC1(void)
{
    RCC->APB2ENR |= (1<<8); // ADC1 clock

    ADC->CCR = 0; // prescaler /2

    ADC1->CR2 = (1<<0); // ADC ON

    // Séquence de conversion = 3 canaux
    ADC1->SQR1 = (2<<20); // 3 conversions

    ADC1->SQR3 =
        (0<<0) |   // Channel 0 ? PA0
        (1<<5) |   // Channel 1 ? PA1
        (2<<10);   // Channel 2 ? PA2
}
uint16_t ADC_Read(void)
{
    ADC1->CR2 |= (1<<30);          // Start conversion
    while(!(ADC1->SR & (1<<1)));  // EOC
    return ADC1->DR;
}
void USART3_SendChar(char c)
{
    while(!(USART3->SR & (1<<7)));
    USART3->DR = c;
}

void USART3_SendString(char *str)
{
    while(*str)
    {
        USART3_SendChar(*str++);
    }
}

void config_I2C1(void) {
    RCC->AHB1ENR |= (1 << 1);      // Activer horloge GPIOB
    RCC->APB1ENR |= (1 << 21);     // Activer horloge I2C1

    // --- Configuration des broches PB8 (SCL) et PB9 (SDA) ---
    
    // 1. Mode Alternate Function (10 sur les bits 16-17 et 18-19)
    GPIOB->MODER &= ~((3 << 16) | (3 << 18));
    GPIOB->MODER |=  ((2 << 16) | (2 << 18));

    // 2. Sortie Open-Drain (indispensable pour I2C)
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);

    // 3. Vitesse élevée (optionnel mais conseillé)
    GPIOB->OSPEEDR |= (3 << 16) | (3 << 18);

    // 4. ACTIVATION PULL-UP INTERNE (C'est ici que ça se joue)
    // On met "01" dans le registre PUPDR pour PB8 et PB9
    GPIOB->PUPDR &= ~((3 << 16) | (3 << 18));
    GPIOB->PUPDR |=  ((1 << 16) | (1 << 18));

    // 5. Affectation AF4 (I2C1) dans le registre AFR High
    GPIOB->AFR[1] &= ~((0xF << 0) | (0xF << 4));
    GPIOB->AFR[1] |=  ((4 << 0) | (4 << 4));

    // --- Configuration du périphérique I2C ---
    I2C1->CR1 = (1 << 15);         // Reset logiciel
    I2C1->CR1 &= ~(1 << 15);
    I2C1->CR2 = 42;                // Fréquence APB1 (42MHz sur F407)
    I2C1->CCR = 210;               // 100kHz
    I2C1->TRISE = 43;
    I2C1->CR1 |= (1 << 0);         // Enable I2C
}
void DS1621_Init(void) {
    // Start condition
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));
    
    // Adresse (0x48 << 1) + Write (0)
    I2C1->DR = 0x90;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;

    // Commande Start Convert (0xEE)
    I2C1->DR = 0xEE;
    while (!(I2C1->SR1 & (1 << 7)));
    
    // Stop condition
    I2C1->CR1 |= (1 << 9);
}

float DS1621_ReadTemp(void) {
    int8_t temp_msb;
    
    // Start condition
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));

    // Adresse + Write
    I2C1->DR = 0x90;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;

    // Commande Read Temp (0xAA)
    I2C1->DR = 0xAA;
    while (!(I2C1->SR1 & (1 << 7)));

    // Repeated Start pour lecture
    I2C1->CR1 |= (1 << 8);
    while (!(I2C1->SR1 & (1 << 0)));

    // Adresse + Read (1)
    I2C1->DR = 0x91;
    while (!(I2C1->SR1 & (1 << 1)));
    (void)I2C1->SR2;

    // Lecture MSB
    while (!(I2C1->SR1 & (1 << 6)));
    temp_msb = I2C1->DR;

    // Stop condition
    I2C1->CR1 |= (1 << 9);

    return (float)temp_msb;
}

/* ===================================================== */
/* ================= MOTEURS ============================ */
/* ===================================================== */

void STOP(void){
    TIM3->CCR1 = 0; TIM3->CCR2 = 0;
    TIM3->CCR3 = 0; TIM3->CCR4 = 0;
}

void AVANCE(void){
    TIM3->CCR1 = 0; TIM3->CCR2 = 0;
    TIM3->CCR3 = 0; TIM3->CCR4 = 800;
}

void ARRIERE(void){
    TIM3->CCR1 = 0; TIM3->CCR2 = 0;
    TIM3->CCR3 = 800; TIM3->CCR4 =0;
}

void AGAUCHE(void){
    TIM3->CCR1 = 800; TIM3->CCR2 = 0;
    TIM3->CCR3 = 0; TIM3->CCR4 = 800;
}

void ADROITE(void){
    TIM3->CCR1 = 0; TIM3->CCR2 =800;
    TIM3->CCR3 = 0; TIM3->CCR4 =800;

}
void RDROITE(void){
    TIM3->CCR1 = 0; TIM3->CCR2 = 800;
    TIM3->CCR3 = 800; TIM3->CCR4 = 0;
}

void RGAUCHE(void){
    TIM3->CCR1 = 800; TIM3->CCR2 = 0;
    TIM3->CCR3 = 800; TIM3->CCR4 =0;
}


/* ===================================================== */
/* ================= MAIN =============================== */
/* ===================================================== */



int main(void) {
    // Vos configurations existantes
    config_GPIOD();
    config_GPIO_USART3();
    config_GPIO_TIM3();
    config_GPIO_ADC();
    
    // Nouvelles configurations
    config_I2C1();
    DS1621_Init();

    config_USART3();
    config_TIM3_PWM();
    config_ADC1();

    while(1) {
        pot1 = ADC_Read();
        pot2 = ADC_Read();
        pot3 = ADC_Read();
        temperature = DS1621_ReadTemp(); // Lecture capteur

        // Envoi des données incluant la température
        sprintf(msg,"P1:%d P2:%d P3:%d T:%.1f C\r\n", pot1, pot2, pot3, temperature);
        USART3_SendString(msg);

        for(volatile int i=0; i<1000000; i++); 
    }
}
/* ===================================================== */
/* ================= USART3 IRQ ========================= */
/* ===================================================== */

void USART3_IRQHandler(void)
{
    if (USART3->SR & (1 << 5)) // RXNE
    {
        Buffer_RX[i++] = USART3->DR; 
        if (i >= 49) i = 49;
    }

    if (USART3->SR & (1 << 4)) // IDLE
    {
        volatile int tmp = USART3->DR;
        Buffer_RX[i] = '\0';

        if      (strstr(Buffer_RX,"AVANCE"))  AVANCE();
        else if (strstr(Buffer_RX,"ARRIERE")) ARRIERE();
        else if (strstr(Buffer_RX,"STOP"))    STOP();
        else if (strstr(Buffer_RX,"ADROITE"))  ADROITE();
        else if (strstr(Buffer_RX,"AGAUCHE"))  AGAUCHE();
			  else if (strstr(Buffer_RX,"RDROITE"))  RDROITE();
        else if (strstr(Buffer_RX,"RGAUCHE"))  RGAUCHE();
        GPIOD->ODR = 0x1000; //led vert
        memset(Buffer_RX,0,sizeof(Buffer_RX));
        i = 0;
    }
}
