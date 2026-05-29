#include "SDLogger.h"
#include "Config.h"
#include "GpsManager.h"
#include "ImuManager.h"
#include "FuelManager.h"

bool SDLogger::isInitialized = false;
File SDLogger::logFile;
String SDLogger::currentFileName = "";
unsigned long SDLogger::writeCounter = 0;

// Utilisation du bus SPI FSPI (standard) sur l'ESP32-S3
SPIClass SDLogger::spi(FSPI); 

void SDLogger::init() {
    Serial.println(F("[SD] Initialisation de la carte MicroSD..."));

    // Configuration personnalisée des broches SPI selon Config.h
    spi.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SD_CS);

    // Démarrage de la carte SD
    if (!SD.begin(PIN_SD_CS, spi)) {
        Serial.println(F("[SD] ERREUR : Carte SD introuvable ou illisible !"));
        isInitialized = false;
        return; // On abandonne l'init, mais le reste de la moto fonctionnera
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(F("[SD] ERREUR : Aucune carte SD détectée !"));
        isInitialized = false;
        return;
    }

    Serial.println(F("[SD] Carte SD détectée avec succès."));
    isInitialized = true;

    // Création du fichier de session
    createNewFile();
}

void SDLogger::createNewFile() {
    // On cherche un nom de fichier libre (de chrono_000.csv à chrono_999.csv)
    for (int i = 0; i < 1000; i++) {
        char filename[20];
        sprintf(filename, "/chrono_%03d.csv", i);
        
        if (!SD.exists(filename)) {
            currentFileName = String(filename);
            logFile = SD.open(currentFileName, FILE_WRITE);
            
            if (logFile) {
                Serial.print(F("[SD] Nouveau fichier créé : "));
                Serial.println(currentFileName);
                
                // --- Écriture de l'en-tête (Header) du CSV ---
                // Ces colonnes seront lues par MotoChrono.html
                logFile.println(F("Time_ms,Lat,Lng,Speed_kmh,Satellites,Roll_deg,Pitch_deg,G_Total,G_Long,Fuel_L"));
                logFile.flush(); // On force l'écriture physique de l'en-tête
            } else {
                Serial.println(F("[SD] ERREUR lors de la création du fichier."));
                isInitialized = false;
            }
            break; // On sort de la boucle dès qu'on a trouvé et créé notre fichier
        }
    }
}

void SDLogger::logData() {
    // Sécurité pragmatique : si pas de SD, on sort tout de suite
    if (!isInitialized || !logFile) return;

    // --- Formatage pragmatique ---
    // On utilise logFile.print() consécutivement au lieu de créer une énorme String.
    // C'est beaucoup plus rapide et ça évite de fragmenter la mémoire RAM.

    logFile.print(millis());               logFile.print(F(","));
    logFile.print(GPSManager::getLatitude(), 6);  logFile.print(F(",")); // 6 décimales pour la précision GPS
    logFile.print(GPSManager::getLongitude(), 6); logFile.print(F(","));
    logFile.print(GPSManager::getSpeedKmh(), 1);  logFile.print(F(","));
    logFile.print(GPSManager::getSatellites());   logFile.print(F(","));
    
    logFile.print(IMUManager::getRoll(), 1);      logFile.print(F(","));
    logFile.print(IMUManager::getPitch(), 1);     logFile.print(F(","));
    logFile.print(IMUManager::getGForceTotal(), 2);logFile.print(F(","));
    logFile.print(IMUManager::getGForceLong(), 2); logFile.print(F(","));
    
    logFile.println(FuelManager::getRemainingLiters(), 2); // println pour la fin de ligne

    writeCounter++;

    // --- Flush stratégique ---
    // Ne JAMAIS faire flush() à chaque ligne, ça tue la carte SD et ralentit le processeur.
    // On force l'écriture physique toutes les 20 lignes (toutes les 2 secondes à 10Hz)
    if (writeCounter % 20 == 0) {
        logFile.flush();
    }
}

void SDLogger::closeFile() {
    if (isInitialized && logFile) {
        logFile.flush(); // On sauve les dernières lignes en attente
        logFile.close();
        Serial.print(F("[SD] Fichier fermé proprement : "));
        Serial.println(currentFileName);
        isInitialized = false;
    }
}