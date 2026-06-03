#include "GPSManager.h"
#include "Config.h"
#include "SettingsManager.h"

HardwareSerial GPSManager::GPS_Serial(1);
TinyGPSPlus GPSManager::gps;

void GPSManager::init() {
    Serial.println(F("========================================="));
    Serial.println(F("[GPS] Lancement du Diagnostic UART..."));
    
    // 1. On démarre à la vitesse d'usine présumée (9600)
    GPS_Serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(500);
    
    // On force la commande de passage à 115200 (au cas où il est bien à 9600)
    sendCASICCommand("PCAS04,5"); 
    delay(100);
    
    // 2. On bascule l'ESP32 sur la vitesse cible (115200)
    GPS_Serial.end();
    delay(10);
    GPS_Serial.begin(115200, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(500);

    // ==========================================
    // ETAPE DIAGNOSTIC : LE "SNIFFER" UART
    // ==========================================
    Serial.println(F("[GPS-DIAG] Ecoute brute de la broche RX (Pin 4) pendant 3 secondes..."));
    Serial.println(F("--- DEBUT DE LA TRAME BRUTE ---"));
    
    unsigned long startTime = millis();
    int bytesReceived = 0;
    
    while (millis() - startTime < 3000) {
        if (GPS_Serial.available()) {
            char c = GPS_Serial.read();
            Serial.write(c); // On pousse directement le caractère vers le PC
            bytesReceived++;
        }
    }
    
    Serial.println(F("\n--- FIN DE LA TRAME BRUTE ---"));
    Serial.printf("[GPS-DIAG] Total des octets reçus : %d\n", bytesReceived);
    
    if (bytesReceived == 0) {
        Serial.println(F("[ERREUR FATALE] 0 octet reçu. L'ESP32 est totalement aveugle."));
        Serial.println(F(" -> Verifiez au multimètre la continuité du fil entre le TX du GPS et la Pin 4."));
    } else if (bytesReceived > 0 && bytesReceived < 50) {
        Serial.println(F("[ERREUR VITESSE] Quelques octets reçus (probablement des caractères bizarres)."));
        Serial.println(F(" -> Le GPS n'est pas à 115200 bauds."));
    } else {
        Serial.println(F("[SUCCES] Communication matérielle validée !"));
    }
    Serial.println(F("=========================================\n"));

    // 3. Configuration finale pour la piste
    sendCASICCommand("PCAS02,100"); // 10 Hz
    delay(100);
    sendCASICCommand("PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0"); // GGA + RMC uniquement
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