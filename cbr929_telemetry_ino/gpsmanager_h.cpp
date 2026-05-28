#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

class GPSManager {
public:
    // Initialise le port série et envoie les commandes CASIC pour le 10Hz
    static void init();

    // À appeler en boucle (sur le Core 0 idéalement) pour vider le buffer série
    static void update();

    // --- Getters pour la télémétrie ---
    static bool isDataValid();      // Vrai si le GPS a "fixé" les satellites
    static float getSpeedKmh();     // Vitesse en km/h
    static double getLatitude();    // Latitude
    static double getLongitude();   // Longitude
    static int getSatellites();     // Nombre de satellites accrochés

private:
    static TinyGPSPlus gps;
    
    // Envoi d'une commande CASIC spécifique au module ATGM336H
    static void sendCASICCommand(const char* command);
};

#endif // GPS_MANAGER_H