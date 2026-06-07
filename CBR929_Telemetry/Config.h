#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// == CONFIGURATION DES BROCHES ESP32-S3   ==
// == Carte : Waveshare ESP32-S3-LCD-1.3   ==
// ==========================================

// --- Acquisition Moto (Regroupé à gauche) ---
#define PIN_INJECTOR      1   // IO1 : Entrée PC817_A (Signal Injecteur)
#define PIN_IGNITION      2   // IO2 : Entrée PC817_B (12V Après Contact)

// --- Interface Utilisateur ---
#define PIN_BUTTON1        4   // IO4 : Bouton poussoir 1 (Page / Calib)
#define PIN_BUTTON2        5   // IO5 : Bouton poussoir 2 (Start/Stop Rec)

// --- Rétroéclairage Écran (Confirmé par schéma constructeur) ---
#define PIN_BACKLIGHT     20  // IO20 : BL_PWM (Contrôle matériel du rétroéclairage)

// --- UART EXTERNE (GPS ATGM336H) ---
#define PIN_GPS_RX        10   // connecter au TX du module GPS
#define PIN_GPS_TX        9    // connecter au RX du module GPS
#define GPS_DEFAULT_BAUDRATE     9600 
#define GPS_SETING_BAUDRATE      115200

// --- SPI EXTERNE (Lecteur Carte MicroSD selon doc Waveshare) ---
#define PIN_SPI_SCK       21  // SD_CLK
#define PIN_SPI_MOSI      18  // SD_MOSI
#define PIN_SPI_MISO      16  // SD_MISO
#define PIN_SD_CS         17  // SD_CS

// Broches I2C internes de ta carte Waveshare
#define I2C_SDA_INTERNAL 47
#define I2C_SCL_INTERNAL 48

// ==========================================
// == CONSTANTES & PARAMÈTRES MÉTIER       ==
// ==========================================

// --- Paramètres Carburant ---
#define TANK_CAPACITY_LITERS  10.0f

// COEFFICIENT À CALIBRER (Litres consommés par milliseconde d'injection)
// Valeur initiale très faible, à ajuster après le premier plein.
#define INJECTOR_FLOW_RATE_L_MS  0.0000005f 

// --- Paramètres Télémétrie ---
#define LOG_FREQUENCY_HZ      10  // Fréquence d'écriture sur la SD (10 fois par seconde)
#define MIN_SPEED_LOGGING     10  // Vitesse min (km/h) pour lancer le chrono et le log.

// --- Paramètres d'Affichage (Veille) ---
#define STANDBY_TIMEOUT_MS    60000 // 1 minute sans bouger = extinction écran
#define STANDBY_SPEED_THRESH  2.0f  // Vitesse min pour considérer la moto en mouvement (filtre la dérive GPS)

// --- Paramètres de secours GPS (Atelier / Garage) ---
#define GPS_DEFAULT_LAT       43.913500  // Circuit d'Albi (exemple)
#define GPS_DEFAULT_LNG       2.118000   // Circuit d'Albi (exemple)
#define GPS_FAKE_SPEED_KMH    0.0f       // Vitesse à l'arrêt dans le garage

// --- Wifi ---
#define WIFI_SSID "MotoChrono"
#define WIFI_MDP "piste929"
#define WIFI_ALIAS_IP "motochrono"

#define WIFI_IP_1 10
#define WIFI_IP_2 0
#define WIFI_IP_3 0
#define WIFI_IP_4 1

#define WIFI_IP_MASK_1 255
#define WIFI_IP_MASK_2 255
#define WIFI_IP_MASK_3 255
#define WIFI_IP_MASK_4 0


#endif // CONFIG_H