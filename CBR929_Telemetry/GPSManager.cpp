#include "GPSManager.h"
#include "Config.h"
#include "SettingsManager.h"

HardwareSerial GPSManager::GPS_Serial(1);
TinyGPSPlus GPSManager::gps;

void GPSManager::init() {
    Serial.println(F("========================================="));
    Serial.println(F("[GPS] Initialisation module ATGM336H..."));
    
    // 1. PAUSE VITALE : L'ESP32 attend que le GPS ait fini de s'allumer
    Serial.println(F("[GPS] Attente du boot du GPS (2.5 secondes)..."));
    delay(2500); 
    
    // 2. Démarrage de l'ESP32 à la vitesse d'usine (9600)
    GPS_Serial.begin(GPS_DEFAULT_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(100);
    
    // 3. Ordre de passage à 115200 (On tire avec les deux protocoles !)
    sendCASICCommand("PCAS01,5");                 // LA VRAIE COMMANDE BAUDRATE ATGM
    GPS_Serial.print("$PMTK251,115200*1F\r\n");   // Commande de secours pour clone Mediatek
    delay(200);
    
    // 4. Bascule matérielle de l'ESP32 sur la nouvelle vitesse
    GPS_Serial.end();
    delay(100);
    GPS_Serial.begin(GPS_SETING_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(500); // On laisse le port série se stabiliser

    // 5. SÉCURITÉ : On renvoie les commandes à 115200
    sendCASICCommand("PCAS01,5");
    GPS_Serial.print("$PMTK251,115200*1F\r\n");
    delay(200);

    // 6. Configuration Piste (Fréquence 10 Hz)
    sendCASICCommand("PCAS02,100"); 
    delay(100);
    
    // BONUS : On force l'utilisation combinée des satellites GPS + BeiDou (Plus de précision !)
    sendCASICCommand("PCAS04,3"); 
    delay(100);

    // 7. Optimisation : GGA + RMC uniquement
    sendCASICCommand("PCAS03,1,0,0,0,1,0,0,0,0,0,,,0,0");
    
    Serial.println(F("[GPS] Module configuré et prêt pour la piste (10Hz, 115200 bauds)."));
    Serial.println(F("========================================="));
}

void GPSManager::update() {
    while (GPS_Serial.available() > 0) {
        char c = GPS_Serial.read(); // On lit 1 caractère venant du GPS
        
        // Serial.print(c); // ECHO : On l'affiche sur l'écran du PC
        
        gps.encode(c);   // On le donne au décodeur pour la télémétrie
    }
}

// ==========================================
// == FONCTION D'ENVOI INTELLIGENTE        ==
// ==========================================
void GPSManager::sendCASICCommand(const char* command) {
    // 1. Calcul du Checksum (XOR de tous les caractères de la commande)
    byte checksum = 0;
    for (int i = 0; command[i] != '\0'; i++) {
        checksum ^= command[i];
    }
    
    // 2. Construction de la trame valide : $ + commande + * + checksum + \r\n
    GPS_Serial.print("$");
    GPS_Serial.print(command);
    GPS_Serial.print("*");
    
    // Formatage du checksum sur 2 caractères hexa (ex: 0F au lieu de F)
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
    // SÉCURITÉ : On vérifie que la donnée est valide ET qu'elle est ultra-fraîche (< 200ms)
    // Si age() est supérieur à 200ms, c'est que le GPS a loupé une trame 10Hz.
    if (gps.speed.isValid() && gps.speed.age() < 200) {
        float rawSpeed = gps.speed.kmph();

        // ZONE MORTE (Anti-dérive dans les stands ou sur béquille)
        // Sous les 3 km/h, on force l'affichage à 0.
        if (rawSpeed < 3.0f) {
            return 0.0f;
        }

        // AUCUN FILTRE LOGICIEL. ZÉRO LATENCE.
        // On passe directement la valeur brute du GPS à l'écran. 
        // Les accélérations et gros freinages s'afficheront instantanément.
        return rawSpeed;
    }
    
    // Si perte de signal (ex: sous un pont) ou module non branché
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