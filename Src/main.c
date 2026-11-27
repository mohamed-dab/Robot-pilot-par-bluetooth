/**
 * @file    main.c
 * @brief   Robot mobile IoT sur STM32F4, programmé au niveau registres (sans HAL).
 *
 *  - Pilotage Bluetooth (HC-05/HC-06) sur USART3, traité sous interruption.
 *  - 4 moteurs commandés en PWM (TIM3, 1 kHz).
 *  - 3 mesures analogiques (ADC1 + DMA2, déclenchement TIM2 à 1 Hz)
 *    et température I2C (DS1621).
 *  - Publication des mesures sur ThingSpeak et en MQTT via un ESP32 (commandes AT).
 *
 * @author  Mohamed DAB — ENSIT, génie électrique
 */
#include "board.h"
#include "config.h"
#include "bluetooth.h"
#include "motors.h"
#include "sensors.h"
#include "esp_net.h"

int main(void)
{
    /* --- Capteurs --- */
    USART2_Init();
    config_I2C1();
    I2C_config_DS1621();
    I2C_write_DS1621(0xEE);          /* Start Convert T */
    Delay_ms(750);

    /* --- Acquisition ADC par DMA, déclenchée à 1 Hz --- */
    DMA2_Stream0_ADC1_Init();
    ADC1_DMA_Init();
    TIM2_TRGO_Init();

    /* --- Robot --- */
    USART3_Init();                   /* Bluetooth */
    TIM3_PWM_Init();                 /* PWM 4 moteurs */
    motor_stop();

    Delay_ms(2000);

    /* --- Réseau --- */
    ESP_Init();
    ESP_ConnectWiFi(WIFI_SSID, WIFI_PASSWORD);
    MQTT_Init();

    while (1)
    {
        char cmd;

        /* 1. Commande Bluetooth (trame complète détectée par IDLE) */
        if (bluetooth_get_command(&cmd))
            cmd_ROBOT(cmd);

        /* 2. Nouvelle séquence ADC disponible : lecture température et envoi */
        if (sensors_adc_ready())
        {
            I2C_read_DS1621();

            ThingSpeak_Send(adc_value[0], adc_value[1], adc_value[2], ds1621_temperature);
            Delay_ms(1000);          /* laisse l'ESP terminer CIPCLOSE */
            MQTT_Send_Data(adc_value[0], adc_value[1], adc_value[2], ds1621_temperature);
        }
    }
}
