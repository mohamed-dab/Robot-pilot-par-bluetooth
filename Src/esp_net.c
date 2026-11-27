/**
 * @file    esp_net.c
 * @brief   ESP32 en commandes AT sur USART2 : Wi-Fi, ThingSpeak et MQTT.
 *
 *   USART2 : PA2 = TX, PA3 = RX (AF7), BRR = 139 -> ~115200 bauds @ 16 MHz
 */
#include "board.h"
#include "config.h"
#include "esp_net.h"
#include <stdio.h>
#include <string.h>

static char tx_buf[256];

/* ====================================================================== */
/*                                USART2                                  */
/* ====================================================================== */
void USART2_Init(void)
{
    RCC->AHB1ENR |= (1 << 0);                  /* GPIOA  */
    RCC->APB1ENR |= (1 << 17);                 /* USART2 */

    GPIOA->MODER  &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER  |=  ((2 << 4) | (2 << 6));
    GPIOA->AFR[0] &= ~((0xF << 8) | (0xF << 12));
    GPIOA->AFR[0] |=  ((7 << 8) | (7 << 12));

    USART2->BRR = 139;
    USART2->CR1 = (1 << 13) | (1 << 3) | (1 << 2);   /* UE | TE | RE */
}

void USART2_SendChar(char c)
{
    while (!(USART2->SR & (1 << 7)));          /* TXE */
    USART2->DR = c;
}

void USART2_SendString(const char *s)
{
    while (*s)
        USART2_SendChar(*s++);
}

/* Temporisation approximative (boucle calibrée pour 16 MHz). */
void Delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 1600; i++)
        __NOP();
}

/* ====================================================================== */
/*                                ESP32                                   */
/* ====================================================================== */
void ESP_SendCmd(const char *cmd)
{
    USART2_SendString(cmd);
    USART2_SendString("\r\n");
    Delay_ms(500);
}

void ESP_Init(void)
{
    Delay_ms(2000);
    ESP_SendCmd("AT+RST");
    Delay_ms(3000);
    ESP_SendCmd("AT");
    ESP_SendCmd("AT+CWMODE=1");                /* mode station */
    Delay_ms(1000);
}

void ESP_ConnectWiFi(const char *ssid, const char *pass)
{
    snprintf(tx_buf, sizeof(tx_buf), "AT+CWJAP=\"%s\",\"%s\"", ssid, pass);
    USART2_SendString(tx_buf);
    USART2_SendString("\r\n");
    Delay_ms(8000);
}

/* ====================================================================== */
/*                         ThingSpeak (HTTP GET)                          */
/* ====================================================================== */
void ThingSpeak_Send(uint16_t ch1, uint16_t ch2, uint16_t ch3, int8_t temp)
{
    char get_req[192];
    int  len;

    ESP_SendCmd("AT+CIPSTART=\"TCP\",\"" THINGSPEAK_IP "\",80");
    Delay_ms(2000);

    len = snprintf(get_req, sizeof(get_req),
                   "GET /update?api_key=%s&field1=%u&field2=%u&field3=%u&field4=%d\r\n",
                   THINGSPEAK_API_KEY, ch1, ch2, ch3, (int)temp);

    snprintf(tx_buf, sizeof(tx_buf), "AT+CIPSEND=%d\r\n", len);
    USART2_SendString(tx_buf);
    Delay_ms(1000);

    USART2_SendString(get_req);
    Delay_ms(2000);

    ESP_SendCmd("AT+CIPCLOSE");
}

/* ====================================================================== */
/*                                 MQTT                                   */
/* ====================================================================== */
void MQTT_Init(void)
{
    snprintf(tx_buf, sizeof(tx_buf),
             "AT+MQTTUSERCFG=%d,1,\"%s\",\"\",\"\",0,0,\"\"",
             MQTT_LINK_ID, MQTT_CLIENT_ID);
    ESP_SendCmd(tx_buf);

    snprintf(tx_buf, sizeof(tx_buf),
             "AT+MQTTCONN=%d,\"%s\",%d,0",
             MQTT_LINK_ID, MQTT_BROKER_IP, MQTT_PORT);
    USART2_SendString(tx_buf);
    USART2_SendString("\r\n");
    Delay_ms(3000);
}

void MQTT_Publish(const char *topic, int32_t value)
{
    snprintf(tx_buf, sizeof(tx_buf), "AT+MQTTPUB=%d,\"%s\",\"%d\",0,0",
             MQTT_LINK_ID, topic, (int)value);
    ESP_SendCmd(tx_buf);
}

void MQTT_Send_Data(uint16_t pot1, uint16_t pot2, uint16_t pot3, int8_t temp)
{
    MQTT_Publish(MQTT_TOPIC_POT1, pot1);
    MQTT_Publish(MQTT_TOPIC_POT2, pot2);
    MQTT_Publish(MQTT_TOPIC_POT3, pot3);
    MQTT_Publish(MQTT_TOPIC_TEMP, temp);
}
