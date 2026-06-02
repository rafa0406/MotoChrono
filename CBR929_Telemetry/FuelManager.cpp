#include "FuelManager.h"
#include "Config.h"
#include "SettingsManager.h"

volatile unsigned int FuelManager::pulseCount = 0;
volatile unsigned long FuelManager::totalOpenTimeMicros = 0;
volatile unsigned long FuelManager::openStartMicros = 0;
volatile bool FuelManager::isInjectorOpen = false;
hw_timer_t * FuelManager::timer = NULL;

void IRAM_ATTR FuelManager::onPulse() {
    unsigned long currentMicros = micros();
    // L'optocoupleur tire la broche vers GND (LOW) quand l'injecteur s'ouvre
    bool isPinLow = (digitalRead(PIN_INJECTOR) == LOW); 

    if (isPinLow && !isInjectorOpen) {
        // --- OUVERTURE DE L'INJECTEUR ---
        openStartMicros = currentMicros;
        isInjectorOpen = true;
        pulseCount++; 
    } 
    else if (!isPinLow && isInjectorOpen) {
        // --- FERMETURE DE L'INJECTEUR ---
        // Le calcul "current - start" gère automatiquement le dépassement 
        // de capacité (overflow) de micros() qui arrive toutes les ~70 minutes.
        unsigned long duration = currentMicros - openStartMicros;
        totalOpenTimeMicros += duration;
        isInjectorOpen = false;
    }
}

void IRAM_ATTR FuelManager::onTimer() {
    // Logique de timer inchangée
}

void FuelManager::init() {
    Serial.println(F("[FUEL] Initialisation du gestionnaire d'essence (Mode Pulse Width)..."));
    
    pinMode(PIN_INJECTOR, INPUT_PULLUP);
    
    // IMPORTANT : On passe en CHANGE pour capter l'ouverture ET la fermeture
    attachInterrupt(digitalPinToInterrupt(PIN_INJECTOR), onPulse, CHANGE);

    timer = timerBegin(1000000); 
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 1000000, true, 0); 
}

unsigned int FuelManager::getPulseCount() {
    noInterrupts();
    unsigned int currentPulses = pulseCount;
    interrupts();
    return currentPulses;
}

unsigned long FuelManager::getTotalOpenTimeMicros() {
    // Désactiver les interruptions est critique ici pour ne pas lire 
    // une variable 32-bits pendant que l'interruption la modifie.
    noInterrupts();
    unsigned long totalTime = totalOpenTimeMicros;
    interrupts();
    return totalTime;
}

float FuelManager::getConsumedLiters() {
    // On utilise désormais la valeur dynamique configurée via la NVS
    return (getTotalOpenTimeMicros() * SettingsManager::injectorCoeff);
}

float FuelManager::getRemainingLiters() {
    float consumed = getConsumedLiters(); 
    float remaining = SettingsManager::tankCapacity - consumed;
    return (remaining > 0) ? remaining : 0;
}

void FuelManager::resetFuel() {
    noInterrupts();
    pulseCount = 0;
    totalOpenTimeMicros = 0; // Remise à zéro stricte des temps
    interrupts();
    Serial.println(F("[FUEL] Plein fait ! Réinitialisation de la consommation."));
}

bool FuelManager::isReserve() {
    return (getRemainingLiters() <= SettingsManager::reserveCapacity);
}