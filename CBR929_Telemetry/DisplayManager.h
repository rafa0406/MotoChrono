#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

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

    // --- NOUVEAUX GETTERS (Encapsulation propre pour l'UI EEZ) ---
    static float getMaxSpeed() { return maxSpeed; }
    static float getMaxGForce() { return maxGForce; }
    static float getMaxLeanLeft() { return maxLeanLeft; }
    static float getMaxLeanRight() { return maxLeanRight; }
    static float getMaxBrakingG() { return maxBrakingG; }
    static float getMaxAccelG() { return maxAccelG; }
    static float getBest0to100() { return best0to100; }

private:
    // Le "Bridge" (Pont) obligatoire pour que LVGL puisse dessiner
    static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    
    // Buffers d'affichage alloués dynamiquement en PSRAM
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *buf1;

    // --- VARIABLES DE GESTION DES PAGES ET BOUTONS ---
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

    // Drapeaux de gestion de sauvegarde
    static bool tropheesLoaded;
    static bool tropheesChanged;
};

#endif // DISPLAYMANAGER_H