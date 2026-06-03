#include "GPSManager.h"
#include "Config.h"
#include "SettingsManager.h"

HardwareSerial GPSManager::GPS_Serial(1);
TinyGPSPlus GPSManager::gps;

void GPSManager::init() {
    Serial.println(F("[GPS] Initialisation module ATGM336H..."));
    
    // 1. On démarre à la vitesse d'usine du GPS (9600)
    GPS_Serial.begin(GPS_DEFAULT_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(500);
    
    // 2. On demande au GPS de passer à 115200 bauds (CASIC 04)
    sendCASICCommand("PCAS04,5"); 
    delay(100);
    
    // 3. On redémarre physiquement le port UART de l'ESP32 à la nouvelle vitesse
    GPS_Serial.end();
    delay(10);
    GPS_Serial.begin(GPS_SETING_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(100);

    // 4. On configure la fréquence de rafraîchissement à 10Hz (100ms) (CASIC 02)
    sendCASICCommand("PCAS02,100");
    delay(100);

    // 5. OPTIMISATION PISTE : On active UNIQUEMENT les trames GGA (index 1) et RMC (index 5).
    // Format : GGA,GLL,GSA,GSV,RMC,VTG,ZDA,ANT,VDV,vel,clk
    sendCASICCommand("PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0");
    
    Serial.println(F("[GPS] Module configuré à 10Hz (115200 bauds, GGA+RMC uniquement)."));
}

void GPSManager::update() {
    while (GPS_Serial.available() > 0) {
        gps.encode(GPS_Serial.read());
    }
}

// ==========================================
// == FONCTION D'ENVOI INTELLIGENTE        ==
// ==========================================
void GPSManager::sendCASICCommand(const char* command) {
    // Calcul du XOR Checksum (standard NMEA)
    byte checksum = 0;
    for (int i = 0; command[i] != '\0'; i++) {
        checksum ^= command[i];
    }
    
    // Construction et envoi de la trame ($ + Commande + * + Checksum + \r\n)
    GPS_Serial.print("$");
    GPS_Serial.print(command);
    GPS_Serial.print("*");
    
    // Si le checksum est < 16, on ajoute le 0 manquant pour formater sur 2 caractères (ex: 0F)
    if (checksum < 0x10) GPS_Serial.print("0");
    
    GPS_Serial.print(checksum, HEX);
    GPS_Serial.print("\r\n"); 
}

bool GPSManager::isDataValid() {
    return gps.location.isValid() && gps.location.isUpdated();
}

uint32_t GPSManager::getCharsProcessed() {
    return gps.charsProcessed();
}

float GPSManager::getSpeedKmh() {
    if (gps.speed.isValid()) {
        return gps.speed.kmph();
    }
    return SettingsManager::gpsFakeSpeed; 
}

double GPSManager::getLatitude() {
    if (gps.location.isValid() && gps.location.lat() != 0.0) {
        return gps.location.lat();
    }
    return SettingsManager::gpsDefaultLat; 
}

double GPSManager::getLongitude() {
    if (gps.location.isValid() && gps.location.lng() != 0.0) {
        return gps.location.lng();
    }
    return SettingsManager::gpsDefaultLng; 
}

int GPSManager::getSatellites() {
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    return 0;
}