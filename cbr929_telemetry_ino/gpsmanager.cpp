#include "GPSManager.h"
#include "Config.h"

// Instanciation de l'objet TinyGPS++
TinyGPSPlus GPSManager::gps;

// Utilisation de l'UART 1 de l'ESP32-S3 (l'UART 0 sert pour l'USB/Console)
HardwareSerial GPS_Serial(1); 

void GPSManager::init() {
    Serial.println(F("[GPS] Initialisation du module ATGM336H..."));

    // 1. Démarrage à la vitesse par défaut de l'ATGM336H (souvent 9600 bauds)
    GPS_Serial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(500);

    // 2. Passage à 115200 bauds. 
    // Pourquoi ? À 9600 bauds, le port série sature si on demande 10 phrases NMEA par seconde.
    // La commande $PCAS01,5 configure le baudrate. 19 est le checksum calculé (XOR).
    Serial.println(F("[GPS] Configuration Baudrate à 115200..."));
    sendCASICCommand("$PCAS01,5*19");
    
    // On laisse le temps au GPS de digérer la commande
    delay(200); 

    // On redémarre notre propre port série à la nouvelle vitesse
    GPS_Serial.updateBaudRate(115200);
    delay(200);

    // 3. Passage en 10 Hz (100 ms par rafraîchissement)
    // La commande $PCAS02,100 configure le taux. 1E est le checksum.
    Serial.println(F("[GPS] Configuration Rafraîchissement à 10Hz..."));
    sendCASICCommand("$PCAS02,100*1E");
    
    delay(200);
    Serial.println(F("[GPS] Initialisation terminée. En attente des satellites..."));
}

void GPSManager::update() {
    // On vide le buffer matériel de l'ESP32 et on donne les caractères à TinyGPS++
    // Cette boucle doit s'exécuter très fréquemment pour ne pas rater de données
    while (GPS_Serial.available() > 0) {
        char c = GPS_Serial.read();
        gps.encode(c);
        
        // Débogage brut : décommenter la ligne ci-dessous si le GPS semble muet
        // Serial.print(c); 
    }
}

// ==========================================
// == FONCTIONS UTILITAIRES ET GETTERS     ==
// ==========================================

void GPSManager::sendCASICCommand(const char* command) {
    GPS_Serial.print(command);
    GPS_Serial.print("\r\n"); // Fin de ligne obligatoire pour le protocole NMEA/CASIC
}

bool GPSManager::isDataValid() {
    // On considère la donnée valide si la position est fixée et récente
    return gps.location.isValid() && gps.location.isUpdated();
}

float GPSManager::getSpeedKmh() {
    if (gps.speed.isValid()) {
        return gps.speed.kmph();
    }
    return 0.0f;
}

double GPSManager::getLatitude() {
    if (gps.location.isValid()) {
        return gps.location.lat();
    }
    return 0.0;
}

double GPSManager::getLongitude() {
    if (gps.location.isValid()) {
        return gps.location.lng();
    }
    return 0.0;
}

int GPSManager::getSatellites() {
    if (gps.satellites.isValid()) {
        return gps.satellites.value();
    }
    return 0;
}