#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum DisplayPage {
    PAGE_ROUTE,
    PAGE_PISTE,
    PAGE_TROPHEES,
    PAGE_CALIBRATION,
    PAGE_GPS,
    PAGE_WIFI,
    PAGE_VEILLE
};

class DisplayManager {
public:
    static void init();
    static void update(bool isButtonPressed);
    static void showByeBye();
    static DisplayPage getCurrentPage() { return currentPage; }

private:
    static void drawPageRoute();
    static void drawPagePiste();
    static void drawPageTrophees(); 
    static void drawPageCalibration();
    static void drawPageGPS();
    static void drawPageWIFI();
    static void drawPageVeille();

    static TFT_eSprite* spr; 

    static DisplayPage currentPage;
    static DisplayPage previousPage;
    
    static unsigned long lastButtonPressMs;
    static unsigned long lastMotionTime;

    // --- TABLEAU DES RECORDS ET AFFICHAGE PISTE ---
    static float maxSpeed;
    static float maxGForce;

    // Historique Inclinaison (Roll)
    static float maxLeanLeft;
    static float maxLeanRight;
    static float gForceAtMaxLeanLeft;  
    static float gForceAtMaxLeanRight; 

    // Historique Assiette (Pitch)
    static float maxPitchUp;   
    static float maxPitchDown; 
    static float gForceAtMaxPitchUp;   
    static float gForceAtMaxPitchDown; 
    
    // Nouveaux records calculés (Page Trophées)
    static float maxBrakingG; // Freinage le plus violent
    static float maxAccelG;   // Accélération la plus violente
    static float best0to100;  // Meilleur temps 0-100 km/h (en secondes)

    // Variables internes pour le calcul du 0-100 km/h
    static unsigned long accelStartTime;
    static bool isTracking0to100;

    // Drapeaux de gestion de sauvegarde ---
    static bool tropheesLoaded;
    static bool tropheesChanged;
};

#endif // DISPLAYMANAGER_H