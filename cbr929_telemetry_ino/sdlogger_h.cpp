#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDLogger {
public:
    // Initialise le bus SPI, la carte SD et crée un nouveau fichier
    static void init();

    // Enregistre une ligne de télémétrie (à appeler à 10Hz depuis la boucle principale)
    static void logData();

    // Ferme proprement le fichier (crucial lors de la coupure du contact)
    static void closeFile();

private:
    static bool isInitialized;
    static File logFile;
    static String currentFileName;
    static unsigned long writeCounter;

    // Déclaration d'un objet SPI spécifique pour l'ESP32-S3
    static SPIClass spi; 

    // Trouve le prochain nom de fichier disponible (ex: chrono_005.csv)
    static void createNewFile();
};

#endif // SD_LOGGER_H