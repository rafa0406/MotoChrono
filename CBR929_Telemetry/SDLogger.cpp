#include "SDLogger.h"
#include "Config.h"
#include "GPSManager.h"
#include "IMUManager.h"
#include "FuelManager.h"
#include "SettingsManager.h"
#include <ArduinoJson.h>

bool SDLogger::isInitialized = false;
bool SDLogger::isRecording = false; // Initialisé à false
File SDLogger::logFile;
String SDLogger::currentFileName = "";
unsigned long SDLogger::writeCounter = 0;
SPIClass SDLogger::spi(FSPI); 

void SDLogger::init() {
    Serial.println(F("[SD] Initialisation de la carte MicroSD..."));
    spi.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SD_CS);

    if (!SD.begin(PIN_SD_CS, spi)) {
        Serial.println(F("[SD] ERREUR : Carte SD introuvable ou illisible !"));
        isInitialized = false;
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(F("[SD] ERREUR : Aucune carte SD détectée !"));
        isInitialized = false;
        return;
    }

    Serial.println(F("[SD] Carte SD détectée avec succès. Prêt à enregistrer."));
    isInitialized = true;
}

void SDLogger::toggleRecording() {
    if (!isInitialized) return;

    if (isRecording) {
        closeFile();
        isRecording = false;
        Serial.println(F("[SD] 🛑 Enregistrement ARRÊTÉ."));
    } else {
        createNewFile();
        isRecording = true;
        Serial.println(F("[SD] 🔴 Enregistrement DÉMARRÉ !"));
    }
}

bool SDLogger::isRecordingStatus() {
    return isRecording;
}

void SDLogger::createNewFile() {
    for (int i = 0; i < 1000; i++) {
        char filename[20];
        sprintf(filename, "/chrono_%03d.csv", i);
        
        if (!SD.exists(filename)) {
            currentFileName = String(filename);
            logFile = SD.open(currentFileName, FILE_WRITE);
            
            if (logFile) {
                Serial.print(F("[SD] Nouveau fichier créé : "));
                Serial.println(currentFileName);
                logFile.println(F("Time_ms,Lat,Lng,Speed_kmh,Satellites,Roll_deg,Pitch_deg,G_Total,G_Long,Fuel_L"));
                logFile.flush(); 
            } else {
                Serial.println(F("[SD] ERREUR lors de la création du fichier."));
                isRecording = false;
            }
            break; 
        }
    }
}

void SDLogger::logData() {
    // 1. Si on n'est pas en mode enregistrement ou SD absente, on annule.
    if (!isInitialized || !logFile || !isRecording) return;

    // 2. NOUVEAU : Filtre de vitesse minimale (On n'enregistre pas à l'arrêt)
    if (GPSManager::getSpeedKmh() < SettingsManager::minSpeedLogging) {
        return; 
    }

    // 3. NOUVEAU : Contrôle précis de la fréquence (Hz)
    static unsigned long lastLogTime = 0;
    unsigned long logInterval = 1000 / SettingsManager::logFrequencyHz; // Ex: 1000/20 = 50ms
    
    if (millis() - lastLogTime < logInterval) {
        return; // On attend le prochain cycle
    }
    lastLogTime = millis();

    // 4. Écriture des données
    logFile.print(millis());                      logFile.print(F(","));
    logFile.print(GPSManager::getLatitude(), 6);  logFile.print(F(","));
    logFile.print(GPSManager::getLongitude(), 6); logFile.print(F(","));
    logFile.print(GPSManager::getSpeedKmh(), 1);  logFile.print(F(","));
    logFile.print(GPSManager::getSatellites());   logFile.print(F(","));
    logFile.print(IMUManager::getRoll(), 1);      logFile.print(F(","));
    logFile.print(IMUManager::getPitch(), 1);     logFile.print(F(","));
    logFile.print(IMUManager::getGForceTotal(), 2);logFile.print(F(","));
    logFile.print(IMUManager::getGForceLong(), 2); logFile.print(F(","));
    logFile.println(FuelManager::getRemainingLiters(), 2); 

    writeCounter++;
    
    // 5. NOUVEAU : Flush intelligent (exactement 1 fois par seconde)
    // Si on est à 20Hz, ça fait un flush tous les 20 passages. Si 10Hz, tous les 10 passages.
    if (SettingsManager::logFrequencyHz > 0 && writeCounter % SettingsManager::logFrequencyHz == 0) {
        logFile.flush();
    }
}

void SDLogger::closeFile() {
    if (isInitialized && logFile) {
        logFile.flush(); 
        logFile.close();
        Serial.print(F("[SD] Fichier fermé proprement : "));
        Serial.println(currentFileName);
    }
}

// ==========================================
// == GESTION DU FICHIER TROPHEES.JSON     ==
// ==========================================

void SDLogger::saveTrophees(float speed, float leanL, float leanR, float brakeG, float accelG, float maxG, float b0100) {
    if (!isInitialized) return;

    // Suppression de l'ancien fichier pour réécrire proprement par-dessus
    if (SD.exists("/trophees.json")) {
        SD.remove("/trophees.json");
    }

    File file = SD.open("/trophees.json", FILE_WRITE);
    if (!file) {
        Serial.println(F("[SD] ERREUR : Impossible d'écrire trophees.json"));
        return;
    }

    // Création du document JSON (Syntaxe ArduinoJson v7)
    JsonDocument doc; 
    doc["maxSpeed"] = speed;
    doc["maxLeanLeft"] = leanL;
    doc["maxLeanRight"] = leanR;
    doc["maxBrakingG"] = brakeG;
    doc["maxAccelG"] = accelG;
    doc["maxGForce"] = maxG;
    doc["best0to100"] = b0100;

    if (serializeJson(doc, file) == 0) {
        Serial.println(F("[SD] ERREUR : Sérialisation JSON échouée."));
    } else {
        Serial.println(F("[SD] Trophées sauvegardés avec succès !"));
    }
    file.close();
}

bool SDLogger::loadTrophees(float &speed, float &leanL, float &leanR, float &brakeG, float &accelG, float &maxG, float &b0100) {
    if (!isInitialized || !SD.exists("/trophees.json")) {
        Serial.println(F("[SD] Fichier trophees.json introuvable. Nouveaux records vierges."));
        return false;
    }

    File file = SD.open("/trophees.json", FILE_READ);
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println(F("[SD] ERREUR : Lecture JSON corrompue."));
        return false;
    }

    // Récupération avec valeurs par défaut si la clé n'existe pas
    speed  = doc["maxSpeed"] | 0.0f;
    leanL  = doc["maxLeanLeft"] | 0.0f;
    leanR  = doc["maxLeanRight"] | 0.0f;
    brakeG = doc["maxBrakingG"] | 0.0f;
    accelG = doc["maxAccelG"] | 0.0f;
    maxG   = doc["maxGForce"] | 0.0f;
    b0100  = doc["best0to100"] | 99.99f;

    Serial.println(F("[SD] Anciens Trophées chargés depuis la SD avec succès."));
    return true;
}