#ifndef FUEL_MANAGER_H
#define FUEL_MANAGER_H

#include <Arduino.h>
#include <Preferences.h> // Librairie standard ESP32 pour la NVS

class FuelManager {
public:
    // Initialise le module (attache l'interruption, charge la NVS)
    static void init();

    // Met à jour la logique lente (sauvegarde NVS, détection de reset)
    // À appeler dans la boucle principale du Core 1
    static void update(bool isContactOff, bool isResetPressed);

    // Retourne le volume restant (en Litres)
    static float getRemainingLiters();

    // Retourne le volume total consommé (en Litres) depuis le dernier plein
    static float getConsumedLiters();

    // Déclenche une sauvegarde immédiate en NVS
    static void forceSave();

private:
    // --- Variables d'état ---
    static volatile float totalConsumedLiters; // volatile car modifiée par l'interruption
    static float remainingLiters;
    static Preferences preferences;

    // --- Variables pour l'interruption ---
    static volatile unsigned long lastOpenTimeMicros;
    static portMUX_TYPE mux; // Mutex pour protéger les variables partagées entre l'ISR et la boucle

    // --- Fonction d'interruption (ISR) ---
    // Doit être en RAM (IRAM_ATTR) pour être ultra-rapide
    static void IRAM_ATTR injectorISR();
    
    // --- Fonctions internes ---
    static void loadFromNVS();
    static void resetFuel();
};

#endif // FUEL_MANAGER_H