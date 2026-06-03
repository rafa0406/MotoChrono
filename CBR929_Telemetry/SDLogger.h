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
    
    static void toggleRecording();
    static bool isRecordingStatus();

    // Gestion des Trophées JSON ---
    static bool getIsInitialized() { return isInitialized; }
    static void saveTrophees(float speed, float leanL, float leanR, float brakeG, float accelG, float maxG, float b0100);
    static bool loadTrophees(float &speed, float &leanL, float &leanR, float &brakeG, float &accelG, float &maxG, float &b0100);

private:
    static bool isInitialized;
    static bool isRecording; 
    static File logFile;
    static String currentFileName;
    static unsigned long writeCounter;
    static SPIClass spi; 
    static void createNewFile();
};

#endif // SD_LOGGER_H