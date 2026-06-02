#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

class SettingsManager {
public:
    static void init();
    static void save();

    // Variables de configuration dynamiques
    static float tankCapacity;
    static float reserveCapacity;
    static int logFrequencyHz;
    static float minSpeedLogging;
    static double gpsDefaultLat;
    static double gpsDefaultLng;
    static float gpsFakeSpeed;

private:
    static Preferences preferences;
};

#endif