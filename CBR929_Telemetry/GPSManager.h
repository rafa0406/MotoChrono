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

private:
    static HardwareSerial GPS_Serial;
    static TinyGPSPlus gps;
    static void sendCASICCommand(const char* command);
};

#endif // GPS_MANAGER_H