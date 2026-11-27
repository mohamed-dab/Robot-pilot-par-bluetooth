/**
 * @file    esp_net.h
 * @brief   Passerelle réseau : ESP32 piloté en commandes AT sur USART2
 *          (PA2 = TX, PA3 = RX, 115200 bauds), envoi vers ThingSpeak (HTTP)
 *          et vers un broker MQTT.
 */
#ifndef ESP_NET_H
#define ESP_NET_H

#include <stdint.h>

void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(const char *s);

void Delay_ms(uint32_t ms);

void ESP_SendCmd(const char *cmd);
void ESP_Init(void);
void ESP_ConnectWiFi(const char *ssid, const char *pass);

void ThingSpeak_Send(uint16_t ch1, uint16_t ch2, uint16_t ch3, int8_t temp);

void MQTT_Init(void);
void MQTT_Publish(const char *topic, int32_t value);
void MQTT_Send_Data(uint16_t pot1, uint16_t pot2, uint16_t pot3, int8_t temp);

#endif /* ESP_NET_H */
