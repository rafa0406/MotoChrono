#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h" 
#include "SDLogger.h" 
#include "SettingsManager.h"

// Variables statiques globales
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite* DisplayManager::spr = nullptr;

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
DisplayPage DisplayManager::previousPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;
unsigned long DisplayManager::lastMotionTime = 0;

// Init de la V-Max
float DisplayManager::maxSpeed = 0.0f; 

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

// --- Trophées supplémentaires ---
float DisplayManager::maxBrakingG = 0.0f;
float DisplayManager::maxAccelG = 0.0f;
float DisplayManager::best0to100 = 99.99f; 
unsigned long DisplayManager::accelStartTime = 0;
bool DisplayManager::isTracking0to100 = false;

bool DisplayManager::tropheesLoaded = false;
bool DisplayManager::tropheesChanged = false;

void DisplayManager::init() {
    Serial.println(F("[DISPLAY] 1. Activation du rétroéclairage..."));
    pinMode(PIN_BACKLIGHT, OUTPUT); 
    digitalWrite(PIN_BACKLIGHT, HIGH); 

    Serial.println(F("[DISPLAY] 2. Init TFT Hardware..."));
    tft.init(); 
    tft.setRotation(3); 
    tft.fillScreen(TFT_BLACK);

    Serial.println(F("[DISPLAY] 3. Allocation Sprite en PSRAM..."));
    spr = new TFT_eSprite(&tft); 
    
    if (spr != nullptr && spr->createSprite(240, 240) != nullptr) {
        spr->setTextDatum(MC_DATUM); 
        spr->fillSprite(TFT_BLACK);
        Serial.println(F("[DISPLAY] Sprite alloué avec succès !"));
    } else {
        Serial.println(F("[DISPLAY] ERREUR ALLOCATION SPRITE !"));
    }
    
    lastMotionTime = millis(); // Initialisation du chronomètre d'immobilité
}

void DisplayManager::update(bool isButtonPressed) {
    if (spr == nullptr) return;

    // ==========================================
    // 0. CHARGEMENT UNIQUE DES RECORDS AU DÉMARRAGE
    // ==========================================
    // On attend 2 secondes pour être sûr que la tâche Core0 a bien initialisé la carte SD
    if (!tropheesLoaded && millis() > 2000) {
        if (SDLogger::getIsInitialized()) {
            SDLogger::loadTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
            tropheesLoaded = true; // Fait une seule fois !
        }
    }

    float currentSpeed = GPSManager::getSpeedKmh();
    float currentRoll = IMUManager::getRoll(); 
    float currentPitch = IMUManager::getPitch();
    float currentGForce = IMUManager::getGForceTotal(); 
    unsigned long currentMillis = millis();

    // ==========================================
    // 2. CENTRALISATION DES RECORDS ET TROPHÉES
    // ==========================================
    bool recordBroken = false; // Flag local pour ce cycle
    
    if (currentRoll < maxLeanLeft) { maxLeanLeft = currentRoll; gForceAtMaxLeanLeft = currentGForce; recordBroken = true; }
    if (currentRoll > maxLeanRight) { maxLeanRight = currentRoll; gForceAtMaxLeanRight = currentGForce; recordBroken = true; }
    if (currentPitch < maxPitchDown) { maxPitchDown = currentPitch; gForceAtMaxPitchDown = currentGForce; }
    if (currentPitch > maxPitchUp) { maxPitchUp = currentPitch; gForceAtMaxPitchUp = currentGForce; }
    
    if (currentGForce > maxGForce) { maxGForce = currentGForce; recordBroken = true; }
    if (currentSpeed > maxSpeed) { maxSpeed = currentSpeed; recordBroken = true; }

    if (currentPitch < 0.0f) { 
        if (currentGForce > maxBrakingG) { maxBrakingG = currentGForce; recordBroken = true; }
    } else if (currentPitch > 0.0f) { 
        if (currentGForce > maxAccelG) { maxAccelG = currentGForce; recordBroken = true; }
    }

    if (currentSpeed < 1.0f) {
        isTracking0to100 = true;
        accelStartTime = currentMillis;
    } else if (isTracking0to100) {
        if (currentSpeed >= 100.0f) {
            float runTime = (currentMillis - accelStartTime) / 1000.0f;
            if (runTime < best0to100 && runTime > 1.5f) { 
                best0to100 = runTime;
                recordBroken = true;
            }
            isTracking0to100 = false;
        } else if (currentSpeed < 5.0f && (currentMillis - accelStartTime > 5000)) {
            accelStartTime = currentMillis; 
        }
    }

    // Si on a battu un record dans cette boucle de 50ms, on lève le drapeau de sauvegarde pending
    if (recordBroken) {
        tropheesChanged = true;
    }

    // ==========================================
    // 3 & 4. GESTION DU REVEIL ET DE LA VEILLE
    // ==========================================
    if (currentSpeed >= STANDBY_SPEED_THRESH) {
        lastMotionTime = currentMillis; 
        if (currentPage == PAGE_VEILLE) {
            currentPage = previousPage; 
            digitalWrite(PIN_BACKLIGHT, HIGH); 
        }
    }

    if (currentPage != PAGE_VEILLE && (currentMillis - lastMotionTime > STANDBY_TIMEOUT_MS)) {
        previousPage = currentPage; 
        currentPage = PAGE_VEILLE;
        digitalWrite(PIN_BACKLIGHT, LOW); 
        Serial.println(F("[DISPLAY] Mise en veille."));

        // ---> SAUVEGARDE STRATÉGIQUE DES RECORDS AUX STANDS <---
        if (tropheesChanged) {
            SDLogger::saveTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
            tropheesChanged = false; // On abaisse le drapeau pour ne pas réécrire pour rien
        }
    }

    // ==========================================
    // 5. GESTION DU BOUTON (Réveil MANUEL / Pages)
    // ==========================================
    if (isButtonPressed && (currentMillis - lastButtonPressMs > 300)) { 
        lastButtonPressMs = currentMillis;
        lastMotionTime = currentMillis; 

        if (currentPage == PAGE_VEILLE) {
            currentPage = previousPage;
            digitalWrite(PIN_BACKLIGHT, HIGH);
            Serial.println(F("[DISPLAY] Réveil manuel via BTN1."));
        } else {
            // Cycle mis à jour avec la page Trophées
            if (currentPage == PAGE_ROUTE) currentPage = PAGE_PISTE;
            else if (currentPage == PAGE_PISTE) currentPage = PAGE_TROPHEES;
            else if (currentPage == PAGE_TROPHEES) currentPage = PAGE_CALIBRATION;
            else if (currentPage == PAGE_CALIBRATION) currentPage = PAGE_GPS;
            else currentPage = PAGE_ROUTE;
        }
    }

    // ==========================================
    // 6. RENDU VISUEL (Uniquement si allumé)
    // ==========================================
    if (currentPage != PAGE_VEILLE) {
        spr->fillSprite(TFT_BLACK); 

        if (currentPage == PAGE_ROUTE) drawPageRoute();
        else if (currentPage == PAGE_PISTE) drawPagePiste();
        else if (currentPage == PAGE_TROPHEES) drawPageTrophees();
        else if (currentPage == PAGE_CALIBRATION) drawPageCalibration();
        else if (currentPage == PAGE_GPS) drawPageGPS();

        spr->pushSprite(0, 0); 
    }
}

void DisplayManager::drawPageVeille() {
    // Écran physiquement éteint, pas de calcul CPU
}

void DisplayManager::showByeBye() {
    if (spr == nullptr) return;

    // ---> SAUVEGARDE D'URGENCE (Coupure Contact 12V) <---
    // Si tu rentres aux stands et coupes le contact avant le délai de veille de 60s
    if (tropheesChanged) {
        SDLogger::saveTrophees(maxSpeed, maxLeanLeft, maxLeanRight, maxBrakingG, maxAccelG, maxGForce, best0to100);
        tropheesChanged = false;
    }

    spr->fillSprite(TFT_BLACK);
    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(MC_DATUM);
    spr->drawString("Arret en cours...", 120, 120, 4);
    spr->pushSprite(0, 0);
}

void DisplayManager::drawPageRoute() {
    float currentSpeed = GPSManager::getSpeedKmh();

    // ==========================================
    // 1. BARRE DE STATUT (Haut de l'écran)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("GPS", 10, 8, 2);
    spr->fillCircle(45, 8, 4, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

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
    spr->drawString(String((int)currentSpeed), 80, 60, 7); 
    spr->drawString("km/h", 80, 100, 2);

    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString("Max Speed ", 210, 50, 2);
    spr->drawString(String((int)maxSpeed), 200, 80, 4);
    spr->drawString("km/h", 200, 100, 2);

    spr->drawFastHLine(0, 120, 240, TFT_DARKGREY);

    // ==========================================
    // 3. ESSENCE : VALEUR ET JAUGE VERTICALE (Bas)
    // ==========================================
    float remaining = FuelManager::getRemainingLiters();
    float tankCap = SettingsManager::tankCapacity;
    if (tankCap <= 0) tankCap = 18.0; 
    
    float fuelPct = constrain(remaining / tankCap, 0.0f, 1.0f);
    bool inReserve = FuelManager::isReserve();
    uint16_t textColor = inReserve ? TFT_RED : TFT_WHITE;

    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("ESSENCE", 90, 135, 2);

    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(textColor);
    spr->drawFloat(remaining, 1, 130, 180, 6); 

    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawString("L", 135, 185, 4); 

    const int barX = 175;
    const int barY = 135;
    const int barW = 30; 
    const int barH = 90;

    spr->drawRect(barX, barY, barW, barH, TFT_DARKGREY);
    spr->fillRect(barX + 1, barY + 1, barW - 2, barH - 2, TFT_BLACK);

    int fillH = fuelPct * (barH - 2);
    int fillY = (barY + barH - 1) - fillH; 

    for (int y = fillY; y <= barY + barH - 2; y++) {
        float posPct = (float)((barY + barH - 2) - y) / (barH - 2);
        uint8_t r = constrain((1.0f - posPct) * 255 * 2, 0, 255); 
        uint8_t g = constrain(posPct * 255 * 2, 0, 255);          
        uint16_t lineColor = tft.color565(r, g, 0); 
        spr->drawFastHLine(barX + 1, y, barW - 2, lineColor);
    }
    
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

    // ==========================================
    // 1. BARRE DE STATUT (Haut de l'écran)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("GPS", 10, 8, 2);
    spr->fillCircle(45, 8, 4, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

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
    const int rollY = 24; 
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
    spr->drawString(String((int)currentSpeed), 80, 200, 7); 
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("km/h", 80, 235, 2);

    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_CYAN);
    spr->drawString(String((int)maxSpeed), 180, 220, 4);
    spr->drawString("Max km/h", 180, 235, 2);
}

void DisplayManager::drawPageTrophees() {
    float displayLeft = abs(maxLeanLeft);
    float displayRight = abs(maxLeanRight);

    // ==========================================
    // EN-TÊTE PAGE
    // ==========================================
    spr->setTextDatum(TC_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString("RECORD BOOK", 120, 8, 2);
    spr->drawFastHLine(0, 26, 240, TFT_DARKGREY);

    // Ligne 1 : VITESSE MAX
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Vitesse Max :", 15, 45, 2);
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_WHITE);
    spr->drawString(String((int)maxSpeed) + " km/h", 225, 45, 4);

    // Ligne 2 : INCLINAISON MAX (Gauche / Droite)
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Angles Max :", 15, 80, 2);
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_CYAN);
    spr->drawString("G " + String((int)displayLeft) + "` | " + String((int)displayRight) + "` D", 225, 80, 2);

    // Ligne 3 : ACCÉLÉRATION VS FREINAGE (Forces G Latérales)
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Accel / Frein :", 15, 115, 2);
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_ORANGE);
    spr->drawString("+" + String(maxAccelG, 2) + "G | -" + String(maxBrakingG, 2) + "G", 225, 115, 2);

    // Ligne 4 : FORCE G ABSOLUE (Virages + bosses)
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Force G Max :", 15, 150, 2);
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_GREEN);
    spr->drawString(String(maxGForce, 2) + " G", 225, 150, 4);

    spr->drawFastHLine(0, 175, 240, TFT_DARKGREY);

    // ==========================================
    // ZONE CHRONO EN GROS (Bas de l'écran)
    // ==========================================
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("MEILLEUR 0 - 100 km/h", 120, 192, 2);

    spr->setTextColor(TFT_SKYBLUE);
    if (best0to100 > 90.0f) {
        spr->drawString("--.-- s", 120, 220, 4); 
    } else {
        spr->drawString(String(best0to100, 2) + " s", 120, 220, 4);
    }
}

void DisplayManager::drawPageCalibration() {
    // ==========================================
    // EN-TÊTE
    // ==========================================
    spr->setTextDatum(TC_DATUM);
    spr->setTextColor(TFT_ORANGE);
    spr->drawString("CALIBRATION  INJ.", 120, 10, 4);
    spr->drawFastHLine(0, 40, 240, TFT_DARKGREY);

    unsigned int pulses = FuelManager::getPulseCount();
    unsigned long openTimeMicros = FuelManager::getTotalOpenTimeMicros();
    float consumed = FuelManager::getConsumedLiters();
    float openTimeMs = openTimeMicros / 1000.0f; 

    // ==========================================
    // DONNÉES INJECTEUR
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Ouverture totale (ms):", 10, 60, 2);
    
    spr->setTextColor(TFT_WHITE);
    spr->drawFloat(openTimeMs, 1, 10, 94, 6); 

    spr->setTextColor(TFT_WHITE);
    spr->drawString("Impulsions : " + String(pulses), 10, 125, 2);

    // ==========================================
    // CONSOMMATION
    // ==========================================
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Conso. calculee :", 10, 140, 2);
    
    spr->setTextColor(TFT_CYAN);
    spr->drawFloat(consumed, 3, 10, 175, 6); 
    spr->drawString("L", 160, 180, 4);

    spr->drawFastHLine(0, 210, 240, TFT_DARKGREY);
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString("Coeff: " + String(SettingsManager::injectorCoeff, 9), 120, 225, 2);
}

void DisplayManager::drawPageGPS() {
    // ==========================================
    // EN-TÊTE
    // ==========================================
    spr->setTextDatum(TC_DATUM);
    spr->setTextColor(TFT_SKYBLUE);
    spr->drawString("DIAGNOSTIC  GPS", 120, 10, 4);
    spr->drawFastHLine(0, 40, 240, TFT_DARKGREY);

    bool fixValid = GPSManager::isDataValid();
    int satellites = GPSManager::getSatellites();
    double lat = GPSManager::getLatitude();
    double lng = GPSManager::getLongitude();

    // ==========================================
    // ÉTAT DU SIGNAL
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Statut Fix :", 10, 65, 2);

    spr->setTextDatum(MR_DATUM);
    if (fixValid) {
        spr->setTextColor(TFT_GREEN);
        spr->drawString("FIX OK", 230, 65, 4);
    } else {
        uint16_t searchColor = ((millis() / 500) % 2 == 0) ? TFT_ORANGE : TFT_DARKGREY;
        spr->setTextColor(searchColor);
        spr->drawString("RECHERCHE...", 230, 65, 2);
    }

    // ==========================================
    // SATELLITES
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Satellites :", 10, 115, 2);

    spr->setTextDatum(MR_DATUM);
    uint16_t satColor = TFT_RED;
    if (satellites >= 6) satColor = TFT_GREEN;
    else if (satellites > 0) satColor = TFT_ORANGE;

    spr->setTextColor(satColor);
    spr->drawString(String(satellites), 195, 115, 6); 
    
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("Sats", 205, 120, 2);

    spr->drawFastHLine(0, 155, 240, TFT_DARKGREY);

    // ==========================================
    // COORDONNÉES
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Position actuelle :", 10, 175, 2);

    spr->setTextColor(TFT_WHITE);
    spr->drawString("Lat: " + String(lat, 6), 15, 200, 2);
    spr->drawString("Lng: " + String(lng, 6), 15, 220, 2);

    if (lat == SettingsManager::gpsDefaultLat && lng == SettingsManager::gpsDefaultLng && !fixValid) {
        spr->setTextDatum(MR_DATUM);
        spr->setTextColor(TFT_YELLOW);
        spr->drawString("SIMULÉ", 230, 210, 2);
    }
}