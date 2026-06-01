#ifndef SD_LOGGER_H
#define SD_LOGGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDLogger {
public:
    static void init();
    static void logData();
    static void closeFile();
    
    // Nouvelles méthodes pour gérer l'enregistrement manuel
    static void toggleRecording();
    static bool isRecordingStatus();

private:
    static bool isInitialized;
    static bool isRecording; // NOUVEAU
    static File logFile;
    static String currentFileName;
    static unsigned long writeCounter;
    static SPIClass spi; 
    static void createNewFile();
};

#endif // SD_LOGGER_H