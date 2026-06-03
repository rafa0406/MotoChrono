#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum DisplayPage {
    PAGE_ROUTE,
    PAGE_PISTE,
    PAGE_CALIBRATION,
    PAGE_GPS,
    PAGE_VEILLE
};

class DisplayManager {
public:
    static void init();
    static void update(bool isButtonPressed);
    static void showByeBye();

private:
    static void drawPageRoute();
    static void drawPagePiste();
    static void drawPageCalibration();
    static void drawPageGPS();
    static void drawPageVeille();

    // Seul le Sprite (l'image lourde) reste dynamique !
    static TFT_eSprite* spr; 

    static DisplayPage currentPage;
    static DisplayPage previousPage;
    static unsigned long lastButtonPressMs;
    static unsigned long lastMotionTime;

    // Historique Vitesse
    static float maxSpeed;

    // Historique Inclinaison (Roll)
    static float maxLeanLeft;
    static float maxLeanRight;
    static float gForceAtMaxLeanLeft;  // G Force sur l'angle max gauche
    static float gForceAtMaxLeanRight; // G Force sur l'angle max droit

    // Historique Assiette (Pitch)
    static float maxPitchUp;   // Cabrage / Accélération max
    static float maxPitchDown; // Plongée / Gros freinage max
    static float maxGForce;    // Force G
    static float gForceAtMaxPitchUp;   // G Force au cabrage max
    static float gForceAtMaxPitchDown; // G Force à la plongée max
};

#endif // DISPLAYMANAGER_H