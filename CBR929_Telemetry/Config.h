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
#define PIN_BUTTON        3   // IO3 : Bouton poussoir (Mise à la masse)

// --- UART EXTERNE (GPS ATGM336H) ---
#define PIN_GPS_TX        4   // IO4 : À connecter au RX du module GPS
#define PIN_GPS_RX        5   // IO5 : À connecter au TX du module GPS
#define GPS_BAUDRATE      9600 

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
#define TANK_CAPACITY_LITERS  18.0f

// COEFFICIENT À CALIBRER (Litres consommés par milliseconde d'injection)
// Valeur initiale très faible, à ajuster après le premier plein.
#define INJECTOR_FLOW_RATE_L_MS  0.0000005f 

// --- Paramètres Télémétrie ---
#define LOG_FREQUENCY_HZ      10  // Fréquence d'écriture sur la SD (10 fois par seconde)
#define MIN_SPEED_LOGGING     10  // Vitesse min (km/h) pour lancer le chrono et le log

#endif // CONFIG_H