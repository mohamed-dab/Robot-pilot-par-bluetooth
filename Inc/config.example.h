/**
 * @file    config.example.h
 * @brief   Modèle de configuration réseau.
 *
 * Copier ce fichier en "config.h" puis renseigner vos identifiants.
 * "config.h" est ignoré par git : vos mots de passe et clés ne sont jamais publiés.
 */
#ifndef CONFIG_H
#define CONFIG_H

/* ---------- Wi-Fi ---------- */
#define WIFI_SSID            "VOTRE_SSID"
#define WIFI_PASSWORD        "VOTRE_MOT_DE_PASSE"

/* ---------- ThingSpeak ---------- */
#define THINGSPEAK_IP        "184.106.153.149"     /* api.thingspeak.com */
#define THINGSPEAK_API_KEY   "VOTRE_WRITE_API_KEY"

/* ---------- MQTT ---------- */
#define MQTT_BROKER_IP       "192.168.1.10"        /* broker Mosquitto local */
#define MQTT_PORT            1883
#define MQTT_CLIENT_ID       "STM32_Robot"

#endif /* CONFIG_H */
