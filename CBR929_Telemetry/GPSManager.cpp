#include "GPSManager.h"
#include "Config.h"
#include "SettingsManager.h" // NOUVEAU : Inclusion du gestionnaire

HardwareSerial GPSManager::GPS_Serial(1);
TinyGPSPlus GPSManager::gps;

void GPSManager::init() {
    Serial.println(F("[GPS] Initialisation module ATGM336H..."));
    GPS_Serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    
    delay(500);
    
    sendCASICCommand("$PCAS03,0,0,0,0,0,0,0,0,0,0,,,0,0*02"); 
    delay(100);
    sendCASICCommand("$PCAS04,5*1C"); 
    delay(100);
    
    GPS_Serial.updateBaudRate(115200);
    Serial.println(F("[GPS] Module configuré à 10Hz (115200 bauds)."));
}

void GPSManager::update() {
    while (GPS_Serial.available() > 0) {
        gps.encode(GPS_Serial.read());
    }
}

void GPSManager::sendCASICCommand(const char* command) {
    GPS_Serial.print(command);
    GPS_Serial.print("\r\n"); 
}

bool GPSManager::isDataValid() {
    return gps.location.isValid() && gps.location.isUpdated();
}

float GPSManager::getSpeedKmh() {
    if (gps.speed.isValid()) {
        return gps.speed.kmph();
    }
    // NOUVEAU : Vitesse factice depuis la configuration
    return SettingsManager::gpsFakeSpeed; 
}

double GPSManager::getLatitude() {
    if (gps.location.isValid() && gps.location.lat() != 0.0) {
        return gps.location.lat();
    }
    // NOUVEAU : Position de secours depuis la configuration
    return SettingsManager::gpsDefaultLat; 
}

double GPSManager::getLongitude() {
    if (gps.location.isValid() && gps.location.lng() != 0.0) {
        return gps.location.lng();
    }
    // NOUVEAU : Position de secours depuis la configuration
    return SettingsManager::gpsDefaultLng; 
}

int GPSManager::getSatellites() {
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    return 0;
}