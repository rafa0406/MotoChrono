#ifndef FUEL_MANAGER_H
#define FUEL_MANAGER_H

#include <Arduino.h>

class FuelManager {
public:
    static void init();
    static float getRemainingLiters();
    static void resetFuel();
    static bool isReserve();

    // Getters pour la calibration
    static unsigned int getPulseCount();
    static unsigned long getTotalOpenTimeMicros(); // NOUVEAU : Temps total d'ouverture
    static float getConsumedLiters();

    static void IRAM_ATTR onPulse();
    static void IRAM_ATTR onTimer();

private:
    static volatile unsigned int pulseCount;
    static volatile unsigned long totalOpenTimeMicros; // Cumul du temps d'ouverture
    static volatile unsigned long openStartMicros;     // Timestamp de l'ouverture
    static volatile bool isInjectorOpen;               // État actuel de l'injecteur
    
    static hw_timer_t * timer;
};

#endif // FUEL_MANAGER_H