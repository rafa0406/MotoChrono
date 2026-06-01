#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h" // Utilisé pour récupérer l'angle d'inclinaison

// Variables statiques globales
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite* DisplayManager::spr = nullptr;

DisplayPage DisplayManager::currentPage = PAGE_PISTE;
unsigned long DisplayManager::lastButtonPressMs = 0;

// Variables pour l'inclinomètre
float DisplayManager::maxLeanLeft = 0.0f;
float DisplayManager::maxLeanRight = 0.0f;
float DisplayManager::gForceAtMaxLeanLeft = 0.0f;
float DisplayManager::gForceAtMaxLeanRight = 0.0f;

float DisplayManager::maxPitchUp = 0.0f;
float DisplayManager::maxPitchDown = 0.0f;
float DisplayManager::gForceAtMaxPitchUp = 0.0f;
float DisplayManager::gForceAtMaxPitchDown = 0.0f;
float DisplayManager::maxGForce = 0.0f;

void DisplayManager::init() {
    Serial.println(F("[DISPLAY] 1. Activation du rétroéclairage..."));
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); 

    Serial.println(F("[DISPLAY] 2. Init TFT Hardware..."));
    tft.init(); 
    tft.setRotation(3); 
    tft.fillScreen(TFT_BLACK);

    Serial.println(F("[DISPLAY] 3. Allocation Sprite en PSRAM..."));
    spr = new TFT_eSprite(&tft); 
    
    // Allocation du sprite (nécessite ~115ko de RAM/PSRAM)
    if (spr != nullptr && spr->createSprite(240, 240) != nullptr) {
        spr->setTextDatum(MC_DATUM); 
        spr->fillSprite(TFT_BLACK);
        Serial.println(F("[DISPLAY] Sprite alloué avec succès !"));
    } else {
        Serial.println(F("[DISPLAY] ERREUR ALLOCATION SPRITE !"));
    }
}

void DisplayManager::update(bool isButtonPressed) {
    if (spr == nullptr) return;

    // Gestion du bouton poussoir pour changer de page
    if (isButtonPressed && (millis() - lastButtonPressMs > 300)) { // Anti-rebond basique
        currentPage = (currentPage == PAGE_ROUTE) ? PAGE_PISTE : PAGE_ROUTE;
        lastButtonPressMs = millis();
    }

    spr->fillSprite(TFT_BLACK); // Nettoyage de la frame

    if (currentPage == PAGE_ROUTE) {
        drawPageRoute();
    } else if (currentPage == PAGE_PISTE) {
        drawPagePiste();
    }

    spr->pushSprite(0, 0); // Envoi du buffer à l'écran ST7789 sans flickering
}

void DisplayManager::showByeBye() {
    if (spr == nullptr) return;
    spr->fillSprite(TFT_BLACK);
    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(MC_DATUM);
    spr->drawString("Arrêt en cours...", 120, 120, 4);
    spr->pushSprite(0, 0);
}

void DisplayManager::drawPageRoute() {
    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(MC_DATUM);
    spr->drawString(String((int)GPSManager::getSpeedKmh()), 120, 50, 7);
    spr->drawString("km/h", 120, 90, 2);

    spr->drawFastHLine(0, 120, 240, TFT_DARKGREY);

    spr->setTextColor(TFT_WHITE);
    spr->drawString("Réservoir", 125, 140, 2);

    float remaining = FuelManager::getRemainingLiters();
    int color = (remaining > 5.0) ? TFT_GREEN : TFT_RED;
    
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(color);
    spr->drawFloat(remaining, 1, 160, 180, 6);

    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawString("L", 165, 185, 4);

    // Le point GPS sera bien rouge s'il ne capte pas
    spr->fillCircle(220, 20, 5, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);
}

void DisplayManager::drawPagePiste() {
    float currentRoll = IMUManager::getRoll(); 
    float currentPitch = IMUManager::getPitch();
    float currentSpeed = GPSManager::getSpeedKmh();
    float currentGForce = IMUManager::getGForceTotal(); 

    // ---- 2. Mise à jour des extremums ET de la force G associée ----
    if (currentRoll < maxLeanLeft) {
        maxLeanLeft = currentRoll;
        gForceAtMaxLeanLeft = currentGForce;
    }
    if (currentRoll > maxLeanRight) {
        maxLeanRight = currentRoll;
        gForceAtMaxLeanRight = currentGForce;
    }
    if (currentPitch < maxPitchDown) {
        maxPitchDown = currentPitch;
        gForceAtMaxPitchDown = currentGForce;
    }
    if (currentPitch > maxPitchUp) {
        maxPitchUp = currentPitch;
        gForceAtMaxPitchUp = currentGForce;
    }
    if (currentGForce > maxGForce) maxGForce = currentGForce;

    // ---- 3. Statut GPS ----
    spr->fillCircle(220, 10, 4, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

    // ==========================================
    // BARRE DE ROLL (Horizontale en haut)
    // ==========================================
    const int rollW = 160;     
    const int rollX = 110;     
    const int rollY = 20;      
    const float maxRollScale = 90.0f;

    spr->drawRect(rollX - (rollW/2), rollY, rollW, 16, TFT_DARKGREY);
    spr->drawFastVLine(rollX, rollY, 16, TFT_WHITE); 

    int rx = rollX + (currentRoll / maxRollScale) * (rollW/2);
    rx = constrain(rx, rollX - (rollW/2), rollX + (rollW/2));
    spr->fillRect(rx - 4, rollY, 9, 16, TFT_CYAN); 

    int mrl = rollX + (maxLeanLeft / maxRollScale) * (rollW/2);
    int mrr = rollX + (maxLeanRight / maxRollScale) * (rollW/2);
    int draw_mrl = constrain(mrl, rollX - (rollW/2), rollX);
    int draw_mrr = constrain(mrr, rollX, rollX + (rollW/2));

    spr->drawFastVLine(draw_mrl, rollY - 4, 24, TFT_RED);
    spr->drawFastVLine(draw_mrr, rollY - 4, 24, TFT_RED);

    // Affichage Angle + G Force en dessous (Fond transparent)
    spr->setTextDatum(TC_DATUM); 
    spr->setTextColor(TFT_RED); 
    
    // Gauche
    String rollLeftStr = String((int)abs(maxLeanLeft)) + "  " + String(gForceAtMaxLeanLeft, 1) + "G";
    spr->drawString(rollLeftStr, draw_mrl, rollY + 14, 2); 

    // Droite
    String rollRightStr = String((int)abs(maxLeanRight)) + "  " + String(gForceAtMaxLeanRight, 1) + "G";
    spr->drawString(rollRightStr, draw_mrr, rollY + 14, 2);

    // Valeur actuelle au dessus
    spr->setTextDatum(BC_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawFloat(abs(currentRoll), 1, rollX, rollY - 3, 2);

    // ==========================================
    // BARRE DE PITCH (Verticale à droite)
    // ==========================================
    const int pitchH = 120;
    const int pitchX = 210; 
    const int pitchY = 140; // MODIFIÉ : Descendu à 140 (au lieu de 120) pour dégager le haut de l'écran
    const float maxPitchScale = 90.0f;

    spr->drawRect(pitchX, pitchY - (pitchH/2), 16, pitchH, TFT_DARKGREY);
    spr->drawFastHLine(pitchX, pitchY, 16, TFT_WHITE); 

    int py = pitchY - (currentPitch / maxPitchScale) * (pitchH/2);
    py = constrain(py, pitchY - (pitchH/2), pitchY + (pitchH/2));
    spr->fillRect(pitchX, py - 4, 16, 9, TFT_MAGENTA);

    int mpd = pitchY - (maxPitchDown / maxPitchScale) * (pitchH/2);
    int mpu = pitchY - (maxPitchUp / maxPitchScale) * (pitchH/2);
    int draw_mpd = constrain(mpd, pitchY, pitchY + (pitchH/2));
    int draw_mpu = constrain(mpu, pitchY - (pitchH/2), pitchY);

    spr->drawFastHLine(pitchX - 4, draw_mpd, 24, TFT_ORANGE);
    spr->drawFastHLine(pitchX - 4, draw_mpu, 24, TFT_YELLOW);

    // Affichage "G Force | Angle" à gauche de la barre
    spr->setTextDatum(MR_DATUM); 
    
    // Plongée (Orange)
    spr->setTextColor(TFT_ORANGE);
    String pitchDownStr = String(gForceAtMaxPitchDown, 1) + "G  " + String((int)abs(maxPitchDown));
    spr->drawString(pitchDownStr, pitchX - 8, draw_mpd, 2);
    
    // Cabrage (Jaune)
    spr->setTextColor(TFT_YELLOW);
    String pitchUpStr = String(gForceAtMaxPitchUp, 1) + "G  " + String((int)abs(maxPitchUp));
    spr->drawString(pitchUpStr, pitchX - 8, draw_mpu, 2);

    // Valeur actuelle de Pitch en direct au-dessus de la jauge (Totalement dégagée maintenant)
    spr->setTextDatum(BC_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawFloat(abs(currentPitch), 1, pitchX + 8, pitchY - (pitchH/2) - 4, 2);

    // ==========================================
    // FORCE G (Centre Gauche)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("G-FORCE", 10, 95, 2);
    
    spr->setTextColor(TFT_GREEN);
    spr->drawFloat(currentGForce, 2, 10, 120, 4);
    
    spr->setTextColor(TFT_RED);
    spr->drawString("MAX: " + String(maxGForce, 2), 10, 145, 2);

    // ==========================================
    // VITESSE (Centre Bas)
    // ==========================================
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString(String((int)currentSpeed), 115, 200, 7); 
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("km/h", 115, 235, 2);
}