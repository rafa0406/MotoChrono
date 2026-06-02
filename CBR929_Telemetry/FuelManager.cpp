#include "FuelManager.h"
#include "Config.h"
#include "SettingsManager.h" 

// Déclaration des variables statiques
unsigned long FuelManager::lastPulseTime = 0;
volatile unsigned int FuelManager::pulseCount = 0;
hw_timer_t * FuelManager::timer = NULL;

void IRAM_ATTR FuelManager::onPulse() {
    pulseCount++;
}

void IRAM_ATTR FuelManager::onTimer() {
    // Logique de calcul de consommation (vide pour le moment)
}

void FuelManager::init() {
    Serial.println(F("[FUEL] Initialisation du gestionnaire d'essence..."));
    
    pinMode(PIN_INJECTOR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_INJECTOR), onPulse, FALLING);

    // NOUVELLE API TIMER ESP32 (Core V3)
    timer = timerBegin(1000000); // Fréquence de 1MHz
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 1000000, true, 0); // Déclenchement toutes les 1 sec, boucle infinie (0)
}

float FuelManager::getRemainingLiters() {
    // On calcule l'essence consommée selon les impulsions physiques
    float consumed = (pulseCount * 0.00005); 
    
    // Le restant est calculé DYNAMIQUEMENT par rapport aux paramètres actuels
    float remaining = SettingsManager::tankCapacity - consumed;
    
    return (remaining > 0) ? remaining : 0;
}

void FuelManager::resetFuel() {
    // Faire le plein revient simplement à remettre la consommation à zéro
    pulseCount = 0;
    Serial.println(F("[FUEL] Plein fait ! Réinitialisation de la consommation à 0."));
}

bool FuelManager::isReserve() {
    // isReserve utilisait déjà SettingsManager de façon dynamique, c'est parfait.
    return (getRemainingLiters() <= SettingsManager::reserveCapacity);
}