#include "SettingsManager.h"

Preferences SettingsManager::preferences;

// Valeurs par défaut (si la puce est vierge)
float SettingsManager::tankCapacity = 18.0;
float SettingsManager::reserveCapacity = 3.5;
int SettingsManager::logFrequencyHz = 20;
float SettingsManager::minSpeedLogging = 5.0;
double SettingsManager::gpsDefaultLat = 43.913500;
double SettingsManager::gpsDefaultLng = 2.118000;
float SettingsManager::gpsFakeSpeed = 0.0;

void SettingsManager::init() {
    // On ouvre l'espace "motochrono" en mode lecture/écriture (false)
    preferences.begin("motochrono", false);

    // On charge les valeurs, avec un fallback sur les valeurs par défaut
    tankCapacity = preferences.getFloat("tankCap", 18.0);
    reserveCapacity = preferences.getFloat("resCap", 3.5);
    logFrequencyHz = preferences.getInt("logFreq", 20);
    minSpeedLogging = preferences.getFloat("minSpd", 5.0);
    gpsDefaultLat = preferences.getDouble("gpsLat", 43.913500);
    gpsDefaultLng = preferences.getDouble("gpsLng", 2.118000);
    gpsFakeSpeed = preferences.getFloat("gpsSpd", 0.0);

    Serial.println(F("[SETTINGS] Paramètres chargés depuis le NVS au démarrage."));
}

void SettingsManager::save() {
    // 1. Ordre d'écriture dans le NVS
    preferences.putFloat("tankCap", tankCapacity);
    preferences.putFloat("resCap", reserveCapacity);
    preferences.putInt("logFreq", logFrequencyHz);
    preferences.putFloat("minSpd", minSpeedLogging);
    preferences.putDouble("gpsLat", gpsDefaultLat);
    preferences.putDouble("gpsLng", gpsDefaultLng);
    preferences.putFloat("gpsSpd", gpsFakeSpeed);

    // 2. Log de ce que l'on vient de demander à écrire (Les variables en RAM)
    Serial.println(F("\n============================================="));
    Serial.println(F("[SETTINGS] NOUVELLES VALEURS (EN RAM) :"));
    Serial.printf(" - Reservoir    : %.1f L\n", tankCapacity);
    Serial.printf(" - Reserve      : %.1f L\n", reserveCapacity);
    Serial.printf(" - Freq. Log    : %d Hz\n", logFrequencyHz);
    Serial.printf(" - Vitesse Min  : %.1f km/h\n", minSpeedLogging);
    Serial.printf(" - GPS Lat      : %.6f\n", gpsDefaultLat);
    Serial.printf(" - GPS Lng      : %.6f\n", gpsDefaultLng);
    Serial.printf(" - Vitesse Fake : %.1f km/h\n", gpsFakeSpeed);

    // 3. Re-lecture immédiate depuis la mémoire Flash (NVS)
    // On met des valeurs par défaut aberrantes (ex: -1.0) pour être sûr de capter une erreur de lecture
    float checkTankCap = preferences.getFloat("tankCap", -1.0);
    float checkResCap = preferences.getFloat("resCap", -1.0);
    int checkLogFreq = preferences.getInt("logFreq", -1);
    float checkMinSpd = preferences.getFloat("minSpd", -1.0);
    double checkGpsLat = preferences.getDouble("gpsLat", -1.0);
    double checkGpsLng = preferences.getDouble("gpsLng", -1.0);
    float checkGpsSpd = preferences.getFloat("gpsSpd", -1.0);

    // 4. Log de confirmation de la puce physique
    Serial.println(F("---------------------------------------------"));
    Serial.println(F("[SETTINGS] VERIFICATION LECTURE NVS (FLASH) :"));
    Serial.printf(" - Reservoir    : %.1f L\n", checkTankCap);
    Serial.printf(" - Reserve      : %.1f L\n", checkResCap);
    Serial.printf(" - Freq. Log    : %d Hz\n", checkLogFreq);
    Serial.printf(" - Vitesse Min  : %.1f km/h\n", checkMinSpd);
    Serial.printf(" - GPS Lat      : %.6f\n", checkGpsLat);
    Serial.printf(" - GPS Lng      : %.6f\n", checkGpsLng);
    Serial.printf(" - Vitesse Fake : %.1f km/h\n", checkGpsSpd);
    Serial.println(F("=============================================\n"));
}