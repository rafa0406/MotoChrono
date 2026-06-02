#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h" // Utilisé pour récupérer l'angle d'inclinaison
#include "SDLogger.h" // utiliser pour récupérer le status d'enregistrement
#include "SettingsManager.h"

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
    // ==========================================
    // 1. BARRE DE STATUT (Haut de l'écran)
    // ==========================================
    // GPS (Haut Gauche)
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("GPS", 10, 8, 2);
    spr->fillCircle(45, 8, 4, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

    // ENREGISTREMENT (Haut Droit)
    if (SDLogger::isRecordingStatus()) {
        if ((millis() / 500) % 2 == 0) {
            spr->fillCircle(230, 8, 4, TFT_RED);
        }
        spr->setTextDatum(MR_DATUM);
        spr->setTextColor(TFT_RED);
        spr->drawString("REC", 220, 8, 2);
    }

    // ==========================================
    // 2. VITESSE (Centre Haut)
    // ==========================================
    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(MC_DATUM);
    spr->drawString(String((int)GPSManager::getSpeedKmh()), 120, 60, 7); 
    spr->drawString("km/h", 120, 100, 2);

    // Ligne de séparation
    spr->drawFastHLine(0, 120, 240, TFT_DARKGREY);

    // ==========================================
    // 3. ESSENCE : VALEUR ET JAUGE VERTICALE (Bas)
    // ==========================================
    float remaining = FuelManager::getRemainingLiters();
    float tankCap = SettingsManager::tankCapacity;
    if (tankCap <= 0) tankCap = 18.0; // Sécurité anti-division par zéro
    
    // Calcul du pourcentage restant (entre 0.0 et 1.0)
    float fuelPct = constrain(remaining / tankCap, 0.0f, 1.0f);
    
    // Détermination de la couleur du texte (Rouge si réserve)
    bool inReserve = FuelManager::isReserve();
    uint16_t textColor = inReserve ? TFT_RED : TFT_WHITE;

    // --- Affichage Numérique (décalé à gauche) ---
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("ESSENCE", 90, 135, 2);

    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(textColor);
    spr->drawFloat(remaining, 1, 130, 180, 6); // Valeur numérique

    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawString("L", 135, 185, 4); // Unité

    // --- Affichage Jauge Verticale Dégradée (à droite) ---
    const int barX = 175;
    const int barY = 135;
    const int barW = 30; // Jauge assez large pour être vue avec les vibrations
    const int barH = 90;

    // Contour de la jauge
    spr->drawRect(barX, barY, barW, barH, TFT_DARKGREY);
    
    // Fond vide de la jauge (noir)
    spr->fillRect(barX + 1, barY + 1, barW - 2, barH - 2, TFT_BLACK);

    // Calcul de la hauteur de remplissage
    int fillH = fuelPct * (barH - 2);
    int fillY = (barY + barH - 1) - fillH; // Point de départ en Y (de bas en haut)

    // Tracé ligne par ligne pour créer le dégradé de couleur "Vert -> Jaune -> Rouge"
    for (int y = fillY; y <= barY + barH - 2; y++) {
        // Ratio de position de cette ligne par rapport à la jauge totale (0.0 = bas, 1.0 = haut)
        float posPct = (float)((barY + barH - 2) - y) / (barH - 2);
        
        // Calcul RGB : Rouge max en bas, Vert max en haut. 
        // Le *2 permet de saturer les canaux au milieu pour obtenir un vrai Jaune vif.
        uint8_t r = constrain((1.0f - posPct) * 255 * 2, 0, 255); 
        uint8_t g = constrain(posPct * 255 * 2, 0, 255);          
        
        uint16_t lineColor = tft.color565(r, g, 0); // Conversion en format TFT 16-bits
        
        spr->drawFastHLine(barX + 1, y, barW - 2, lineColor);
    }
    
    // Petit indicateur de réserve (trait rouge horizontal à côté de la jauge)
    float resPct = constrain(SettingsManager::reserveCapacity / tankCap, 0.0f, 1.0f);
    int resY = (barY + barH - 1) - (resPct * (barH - 2));
    spr->drawFastHLine(barX - 5, resY, 5, TFT_RED);
    spr->drawFastHLine(barX + barW, resY, 5, TFT_RED);
}

void DisplayManager::drawPagePiste() {
    float currentRoll = IMUManager::getRoll(); 
    float currentPitch = IMUManager::getPitch();
    float currentSpeed = GPSManager::getSpeedKmh();
    float currentGForce = IMUManager::getGForceTotal(); 

    // ---- Mise à jour des extremums ----
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

    // ==========================================
    // 1. BARRE DE STATUT (Haut de l'écran)
    // ==========================================
    // GPS (Haut Gauche)
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("GPS", 10, 8, 2);
    spr->fillCircle(45, 8, 4, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

    // ENREGISTREMENT (Haut Droit)
    if (SDLogger::isRecordingStatus()) {
        if ((millis() / 500) % 2 == 0) {
            spr->fillCircle(230, 8, 4, TFT_RED);
        }
        spr->setTextDatum(MR_DATUM);
        spr->setTextColor(TFT_RED);
        spr->drawString("REC", 220, 8, 2);
    }

    // ==========================================
    // 2. BARRE DE ROLL (Horizontale en haut)
    // ==========================================
    const int rollW = 160;     
    const int rollX = 110;     
    const int rollY = 24; // Légèrement descendu pour la lisibilité sous le GPS  
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

    spr->setTextDatum(TC_DATUM); 
    spr->setTextColor(TFT_CYAN); 
    
    String rollLeftStr = String((int)abs(maxLeanLeft))+"`" + " " + String(gForceAtMaxLeanLeft, 1) + "G";
    spr->drawString(rollLeftStr, draw_mrl, rollY + 18, 2); 

    String rollRightStr = String((int)abs(maxLeanRight))+"`" + " " + String(gForceAtMaxLeanRight, 1) + "G";
    spr->drawString(rollRightStr, draw_mrr, rollY + 18, 2);

    spr->setTextDatum(BC_DATUM);
    spr->setTextColor(TFT_CYAN);
    spr->drawString(String((int)abs(currentRoll)) + "`", rollX, rollY - 3, 2);

    // ==========================================
    // 3. BARRE DE PITCH (Verticale à droite)
    // ==========================================
    const int pitchH = 120;
    const int pitchX = 210; 
    const int pitchY = 140; 
    const float maxPitchScale = 90.0f;

    spr->drawRect(pitchX, pitchY - (pitchH/2), 16, pitchH, TFT_DARKGREY);
    spr->drawFastHLine(pitchX, pitchY, 16, TFT_WHITE); 

    int py = pitchY - (currentPitch / maxPitchScale) * (pitchH/2);
    py = constrain(py, pitchY - (pitchH/2), pitchY + (pitchH/2));
    spr->fillRect(pitchX, py - 4, 16, 9, TFT_YELLOW);

    int mpd = pitchY - (maxPitchDown / maxPitchScale) * (pitchH/2);
    int mpu = pitchY - (maxPitchUp / maxPitchScale) * (pitchH/2);
    int draw_mpd = constrain(mpd, pitchY, pitchY + (pitchH/2));
    int draw_mpu = constrain(mpu, pitchY - (pitchH/2), pitchY);

    spr->drawFastHLine(pitchX - 4, draw_mpd, 24, TFT_YELLOW);
    spr->drawFastHLine(pitchX - 4, draw_mpu, 24, TFT_YELLOW);

    spr->setTextDatum(MR_DATUM); 
    
    spr->setTextColor(TFT_YELLOW);
    String pitchDownStr = String(gForceAtMaxPitchDown, 1) + "G " + String((int)abs(maxPitchDown))+"`";
    spr->drawString(pitchDownStr, pitchX - 8, draw_mpd, 2);
    
    spr->setTextColor(TFT_YELLOW);
    String pitchUpStr = String(gForceAtMaxPitchUp, 1) + "G " + String((int)abs(maxPitchUp))+"`";
    spr->drawString(pitchUpStr, pitchX - 8, draw_mpu, 2);

    spr->setTextDatum(BC_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString(String((int)abs(currentPitch)) + "`", pitchX + 8, pitchY - (pitchH/2) - 4, 2);

    // ==========================================
    // 4. FORCE G (Centre Gauche)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("G-FORCE", 10, 95, 2);
    
    spr->setTextColor(TFT_GREEN);
    spr->drawFloat(currentGForce, 2, 10, 120, 4);
    
    spr->setTextColor(TFT_GREEN);
    spr->drawString("MAX: " + String(maxGForce, 2), 10, 145, 2);

    // ==========================================
    // 5. VITESSE (Centre Bas)
    // ==========================================
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString(String((int)currentSpeed), 100, 200, 7); 
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("km/h", 100, 235, 2);
}