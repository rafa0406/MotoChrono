#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>

class GPSManager {
public:
    static void init();
    static void update();
    static bool isDataValid();
    static float getSpeedKmh();
    static double getLatitude();
    static double getLongitude();
    static int getSatellites();    
    static uint32_t getCharsProcessed(); // Optionnel, pour ta page de Diagnostic (octets reçus)

private:
    static HardwareSerial GPS_Serial;
    static TinyGPSPlus gps;
    static void sendCASICCommand(const char* command); // Méthode intelligente qui calcule le Checksum
};

#endif // GPS_MANAGER_H