#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Preferences.h> // Librairie NVS pour la sauvegarde de la calibration

class IMUManager {
public:
    // Initialise le bus I2C, le capteur QMI8658 et charge la calibration
    static void init();

    // Met à jour les calculs de Madgwick (à appeler souvent sur Core 0)
    static void update();

    // Calibre le point zéro (moto droite) et le sauvegarde en NVS
    static void calibrateZero();

    // Accesseurs (Getters) pour la télémétrie
    static float getRoll();
    static float getPitch();
    static float getGForceTotal();
    static float getGForceLong();

private:
    static Madgwick filter;
    static Preferences preferences;
    
    static unsigned long lastUpdateMicros;

    // Variables pour stocker les données calculées
    static float currentRoll;
    static float currentPitch;
    static float currentGTotal;
    static float currentGLong;

    // Décalages pour la calibration (Zero)
    static float rollOffset;
    static float pitchOffset;
}; // <--- Le fameux point-virgule obligatoire en C++ pour clore une classe !

#endif // IMU_MANAGER_H