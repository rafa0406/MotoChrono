#ifndef FUEL_MANAGER_H
#define FUEL_MANAGER_H

#include <Arduino.h>

class FuelManager {
public:
    static void init();
    static float getRemainingLiters();
    static void resetFuel();
    static bool isReserve();

    // Nécessaire pour les interruptions matérielles
    static void IRAM_ATTR onPulse();
    static void IRAM_ATTR onTimer();

private:
    static unsigned long lastPulseTime;
    static volatile unsigned int pulseCount;
    static hw_timer_t * timer;
};

#endif // FUEL_MANAGER_H