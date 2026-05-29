#include "FuelManager.h"
#include "Config.h" // On inclut la config pour récupérer les PINS et constantes

// --- Initialisation des variables statiques ---
volatile float FuelManager::totalConsumedLiters = 0.0f;
float FuelManager::remainingLiters = TANK_CAPACITY_LITERS;
Preferences FuelManager::preferences;

volatile unsigned long FuelManager::lastOpenTimeMicros = 0;
portMUX_TYPE FuelManager::mux = portMUX_INITIALIZER_UNLOCKED;

// Variable interne pour éviter de sauvegarder en NVS en permanence
static unsigned long lastSaveTimeMs = 0;
static const unsigned long SAVE_INTERVAL_MS = 30000; // Sauvegarde auto toutes les 30 sec
static float lastSavedConsumedLiters = 0.0f;


void FuelManager::init() {
    Serial.println(F("[FUEL] Initialisation..."));
    
    // 1. Charger l'ancienne valeur depuis la mémoire flash (NVS)
    loadFromNVS();

    // 2. Configurer la broche de l'injecteur
    pinMode(PIN_INJECTOR, INPUT_PULLUP);

    // 3. Attacher l'interruption matérielle
    attachInterrupt(digitalPinToInterrupt(PIN_INJECTOR), injectorISR, CHANGE);
    
    Serial.print(F("[FUEL] Niveau actuel : "));
    Serial.print(remainingLiters);
    Serial.println(F(" L"));
}

void FuelManager::update(bool isContactOff, bool isResetPressed) {
    // --- 1. Gestion du bouton de Reset (Plein à la pompe) ---
    if (isResetPressed) {
        resetFuel();
    }

    // --- 2. Calcul du volume restant (sécurisé par Mutex) ---
    portENTER_CRITICAL(&mux);
    float currentConsumed = totalConsumedLiters;
    portEXIT_CRITICAL(&mux);

    remainingLiters = TANK_CAPACITY_LITERS - currentConsumed;
    if (remainingLiters < 0) remainingLiters = 0; // Sécurité

    // --- 3. Gestion de la sauvegarde auto (NVS) ---
    unsigned long now = millis();
    if (!isContactOff) {
        if ((now - lastSaveTimeMs > SAVE_INTERVAL_MS) && (currentConsumed - lastSavedConsumedLiters > 0.05f)) {
            forceSave();
        }
    }
}

// ==========================================
// == FONCTION D'INTERRUPTION (CRITIQUE) ==
// ==========================================
void IRAM_ATTR FuelManager::injectorISR() {
    unsigned long currentTime = micros();
    // État LOW : L'ECU met l'injecteur à la masse -> L'injecteur S'OUVRE
    bool isInjectorOpen = (digitalRead(PIN_INJECTOR) == LOW); 

    if (isInjectorOpen) {
        // Début de l'injection
        lastOpenTimeMicros = currentTime;
    } 
    else {
        // Fin de l'injection
        if (lastOpenTimeMicros > 0 && currentTime > lastOpenTimeMicros) {
            unsigned long durationMicros = currentTime - lastOpenTimeMicros;
            float durationMs = (float)durationMicros / 1000.0f;
            float volumeInjected = durationMs * INJECTOR_FLOW_RATE_L_MS;

            portENTER_CRITICAL_ISR(&mux);
            totalConsumedLiters += volumeInjected;
            portEXIT_CRITICAL_ISR(&mux);
        }
        lastOpenTimeMicros = 0; 
    }
}

// ==========================================
// == FONCTIONS UTILITAIRES                ==
// ==========================================

void FuelManager::loadFromNVS() {
    preferences.begin("fuel_data", false); 
    totalConsumedLiters = preferences.getFloat("consumed", 0.0f);
    lastSavedConsumedLiters = totalConsumedLiters;
    preferences.end();
}

void FuelManager::forceSave() {
    portENTER_CRITICAL(&mux);
    float currentConsumed = totalConsumedLiters;
    portEXIT_CRITICAL(&mux);

    preferences.begin("fuel_data", false);
    preferences.putFloat("consumed", currentConsumed);
    preferences.end();

    lastSavedConsumedLiters = currentConsumed;
    lastSaveTimeMs = millis();
    Serial.println(F("[FUEL] Sauvegarde NVS effectuée."));
}

void FuelManager::resetFuel() {
    Serial.println(F("[FUEL] PLEIN EFFECTUÉ ! Réinitialisation..."));
    portENTER_CRITICAL(&mux);
    totalConsumedLiters = 0.0f;
    portEXIT_CRITICAL(&mux);
    remainingLiters = TANK_CAPACITY_LITERS;
    forceSave();
}

float FuelManager::getRemainingLiters() {
    return remainingLiters;
}

float FuelManager::getConsumedLiters() {
    portENTER_CRITICAL(&mux);
    float currentConsumed = totalConsumedLiters;
    portEXIT_CRITICAL(&mux);
    return currentConsumed;
}