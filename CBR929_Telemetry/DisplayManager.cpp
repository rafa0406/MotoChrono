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

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
DisplayPage DisplayManager::previousPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;
unsigned long DisplayManager::lastMotionTime = 0;

// Init de la V-Max
float DisplayManager::maxSpeed = 0.0f; //

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
    pinMode(PIN_BACKLIGHT, OUTPUT); // <-- Utilise la macro corrigée
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

    float currentSpeed = GPSManager::getSpeedKmh();
    unsigned long currentMillis = millis();

    // ==========================================
    // 1. REVEIL AUTOMATIQUE ET RESET TIMER
    // ==========================================
    // Si la vitesse dépasse le filtre antibruit GPS (ex: 2km/h), la moto roule
    if (currentSpeed >= STANDBY_SPEED_THRESH) {
        lastMotionTime = currentMillis; 
        
        if (currentPage == PAGE_VEILLE) {
            currentPage = previousPage; // On restaure la dernière vue (Route ou Piste)
            digitalWrite(PIN_BACKLIGHT, HIGH); // Rallumage matériel de la dalle IPS
            Serial.println(F("[DISPLAY] Réveil automatique : Mouvement détecté !"));
        }
    }

    // ==========================================
    // 2. PASSAGE EN VEILLE (Immobilité prolongée)
    // ==========================================
    if (currentPage != PAGE_VEILLE && (currentMillis - lastMotionTime > STANDBY_TIMEOUT_MS)) {
        previousPage = currentPage; // Mémorise la page avant l'extinction
        currentPage = PAGE_VEILLE;
        digitalWrite(PIN_BACKLIGHT, LOW); // Extinction complète pour sauver l'énergie
        Serial.println(F("[DISPLAY] Mise en veille : Immobilité de 60s."));
    }

    // ==========================================
    // 3. GESTION DU BOUTON (Réveil MANUEL / Pages)
    // ==========================================
    if (isButtonPressed && (currentMillis - lastButtonPressMs > 300)) { 
        lastButtonPressMs = currentMillis;
        lastMotionTime = currentMillis; // L'appui sur le bouton prouve qu'on est actif

        if (currentPage == PAGE_VEILLE) {
            // L'utilisateur réveille le tableau de bord à l'arrêt
            currentPage = previousPage;
            digitalWrite(PIN_BACKLIGHT, HIGH);
            Serial.println(F("[DISPLAY] Réveil manuel via BTN1."));
        } else {
            // Cycle classique des pages si on n'était pas en veille
            if (currentPage == PAGE_ROUTE) currentPage = PAGE_PISTE;
            else if (currentPage == PAGE_PISTE) currentPage = PAGE_CALIBRATION;
            else if (currentPage == PAGE_CALIBRATION) currentPage = PAGE_GPS;
            else currentPage = PAGE_ROUTE;
        }
    }

    // ==========================================
    // 4. RENDU VISUEL (Uniquement si allumé)
    // ==========================================
    // Optimisation Dual Core : si l'écran est en veille, l'ESP32 ne fait aucun calcul d'affichage.
    if (currentPage != PAGE_VEILLE) {
        spr->fillSprite(TFT_BLACK); 

        if (currentPage == PAGE_ROUTE) drawPageRoute();
        else if (currentPage == PAGE_PISTE) drawPagePiste();
        else if (currentPage == PAGE_CALIBRATION) drawPageCalibration();
        else if (currentPage == PAGE_GPS) drawPageGPS();

        spr->pushSprite(0, 0); 
    }
}

void DisplayManager::drawPageVeille() {
    // Cette fonction n'a rien de spécifique à faire puisque l'update évite 
    // l'appel au pushSprite() en veille pour sauver du temps CPU.
    // L'extinction matérielle de l'écran garantit un noir parfait et moins de conso.
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

    // Récupération de la vitesse actuelle et maj de la V-Max
    float currentSpeed = GPSManager::getSpeedKmh();
    if (currentSpeed > maxSpeed) maxSpeed = currentSpeed;

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
    spr->drawString(String((int)currentSpeed), 80, 60, 7); 
    spr->drawString("km/h", 80, 100, 2);

    // Affichage V-Max Route (Aligné à droite juste au-dessus de la ligne grise)
    spr->setTextDatum(MR_DATUM);
    spr->setTextColor(TFT_YELLOW);
    spr->drawString("Max Speed ", 210, 50, 2);
    spr->drawString(String((int)maxSpeed), 200, 80, 4);
    spr->drawString("km/h", 200, 100, 2);

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

    if (currentSpeed > maxSpeed) maxSpeed = currentSpeed;

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
    spr->drawString(String((int)currentSpeed), 80, 200, 7); 
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("km/h", 80, 235, 2);

    // Affichage V-Max Piste (Placé juste à droite de la vitesse centrale, sous la jauge de Pitch)
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_CYAN);
    spr->drawString(String((int)maxSpeed), 180, 220, 4);
    spr->drawString("Max km/h", 180, 235, 2);
}

void DisplayManager::drawPageCalibration() {
    // ==========================================
    // EN-TÊTE
    // ==========================================
    spr->setTextDatum(TC_DATUM);
    spr->setTextColor(TFT_ORANGE);
    spr->drawString("CALIBRATION  INJ.", 120, 10, 4);
    spr->drawFastHLine(0, 40, 240, TFT_DARKGREY);

    // Récupération des données en temps réel
    unsigned int pulses = FuelManager::getPulseCount();
    unsigned long openTimeMicros = FuelManager::getTotalOpenTimeMicros();
    float consumed = FuelManager::getConsumedLiters();
    
    // Conversion en millisecondes pour un affichage lisible
    float openTimeMs = openTimeMicros / 1000.0f; 

    // ==========================================
    // DONNÉES INJECTEUR (PULSE WIDTH)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Ouverture totale (ms) :", 10, 60, 2);
    
    spr->setTextColor(TFT_WHITE);
    // Affichage du temps en gros pour le calibrage
    spr->drawFloat(openTimeMs, 1, 10, 94, 6); 

    spr->setTextColor(TFT_WHITE);
    spr->drawString("Impulsions : " + String(pulses), 10, 125, 2);

    // ==========================================
    // CONSOMMATION THÉORIQUE
    // ==========================================
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Conso. calculee :", 10, 140, 2);
    
    spr->setTextColor(TFT_CYAN);
    // On affiche 3 décimales pour la précision du test
    spr->drawFloat(consumed, 3, 10, 175, 6); 
    spr->drawString("L", 160, 180, 4);

    // ==========================================
    // INFO COEFFICIENT
    // ==========================================
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

    // Récupération des données GPS en temps réel
    bool fixValid = GPSManager::isDataValid();
    int satellites = GPSManager::getSatellites();
    double lat = GPSManager::getLatitude();
    double lng = GPSManager::getLongitude();

    // ==========================================
    // ÉTAT DU SIGNAL (FIX)
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Statut Fix :", 10, 65, 2);

    spr->setTextDatum(MR_DATUM);
    if (fixValid) {
        spr->setTextColor(TFT_GREEN);
        spr->drawString("FIX OK", 230, 65, 4);
    } else {
        // Clignotement visuel pour alerter de la recherche de signal
        uint16_t searchColor = ((millis() / 500) % 2 == 0) ? TFT_ORANGE : TFT_DARKGREY;
        spr->setTextColor(searchColor);
        spr->drawString("RECHERCHE...", 230, 65, 2);
    }

    // ==========================================
    // NOMBRE DE SATELLITES
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Satellites :", 10, 115, 2);

    // Affichage du nombre en gros au centre droit
    spr->setTextDatum(MR_DATUM);
    // Code couleur pragmatique : Vert si >= 6 (bonne précision), Orange si < 6, Rouge si 0
    uint16_t satColor = TFT_RED;
    if (satellites >= 6) satColor = TFT_GREEN;
    else if (satellites > 0) satColor = TFT_ORANGE;

    spr->setTextColor(satColor);
    spr->drawString(String(satellites), 195, 115, 6); 
    
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_DARKGREY);
    spr->drawString("Sats", 205, 120, 2);

    // Ligne de séparation
    spr->drawFastHLine(0, 155, 240, TFT_DARKGREY);

    // ==========================================
    // COORDONNÉES GÉOGRAPHIQUES
    // ==========================================
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_LIGHTGREY);
    spr->drawString("Position actuelle :", 10, 175, 2);

    spr->setTextColor(TFT_WHITE);
    // On affiche 6 décimales pour la précision géographique (indispensable sur piste)
    spr->drawString("Lat: " + String(lat, 6), 15, 200, 2);
    spr->drawString("Lng: " + String(lng, 6), 15, 220, 2);

    // Petit indicateur de secours visuel si on utilise les valeurs Fake/Défaut du garage
    if (lat == SettingsManager::gpsDefaultLat && lng == SettingsManager::gpsDefaultLng && !fixValid) {
        spr->setTextDatum(MR_DATUM);
        spr->setTextColor(TFT_YELLOW);
        spr->drawString("SIMULÉ", 230, 210, 2);
    }
}