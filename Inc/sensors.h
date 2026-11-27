/**
 * @file    sensors.h
 * @brief   Acquisition des capteurs :
 *          - 3 voies analogiques (PC0, PC1, PC2) par ADC1 + DMA2, déclenchées par TIM2 à 1 Hz ;
 *          - température par un DS1621 sur I2C1 (PB6 = SCL, PB7 = SDA).
 */
#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

extern volatile uint16_t adc_value[3];
extern int8_t ds1621_temperature;

void DMA2_Stream0_ADC1_Init(void);
void ADC1_DMA_Init(void);
void TIM2_TRGO_Init(void);

/** @return 1 si une séquence ADC complète est disponible (flag remis à zéro). */
uint8_t sensors_adc_ready(void);

void config_I2C1(void);
void I2C_write_DS1621(uint8_t data);
void I2C_config_DS1621(void);
void I2C_read_DS1621(void);

#endif /* SENSORS_H */
