#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h" // Utilisé pour récupérer l'angle d'inclinaison

// Variables statiques globales
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite* DisplayManager::spr = nullptr;

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;

// Variables pour l'inclinomètre
float DisplayManager::maxLeanLeft = 0.0f;
float DisplayManager::maxLeanRight = 0.0f;

void DisplayManager::init() {
    Serial.println(F("[DISPLAY] 1. Activation du rétroéclairage..."));
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH); 

    Serial.println(F("[DISPLAY] 2. Init TFT Hardware..."));
    tft.init(); 
    tft.setRotation(0); 
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
    // ---- 1. Récupération des données ----
    // Attention : IMUManager doit fournir l'angle "Roll" avec Madgwick (négatif = gauche, positif = droite)
    float currentLeanAngle = IMUManager::getRoll(); 
    float currentSpeed = GPSManager::getSpeedKmh();

    // Mise à jour des extremums
    if (currentLeanAngle < maxLeanLeft) maxLeanLeft = currentLeanAngle;
    if (currentLeanAngle > maxLeanRight) maxLeanRight = currentLeanAngle;

    // ---- 2. Constantes de dessin ----
    const int cx = 120; // Centre de l'écran X
    const int cy = 110; // Centre de l'inclinomètre Y (décalé vers le haut)
    const int radius = 90; // Rayon du cadran

    // ---- 3. Dessin du cadran (Horizon + Cercle) ----
    spr->drawLine(cx - radius - 10, cy, cx + radius + 10, cy, TFT_DARKGREY); // Horizon
    spr->drawCircle(cx, cy, radius, TFT_DARKGREY); // Cadran extérieur
    
    // Le point GPS dans le coin
    spr->fillCircle(220, 20, 5, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);

    // ---- 4. Aiguille centrale (Inclinaison courante) ----
    float rad = currentLeanAngle * PI / 180.0;
    int xTop = cx + radius * sin(rad);
    int yTop = cy - radius * cos(rad);
    spr->drawWedgeLine(cx, cy, xTop, yTop, 2, 6, TFT_CYAN); 

    // ---- 5. Indicateurs d'angle MAX (Rouges) ----
    float radMaxL = maxLeanLeft * PI / 180.0;
    spr->fillCircle(cx + radius * sin(radMaxL), cy - radius * cos(radMaxL), 5, TFT_RED);
    
    float radMaxR = maxLeanRight * PI / 180.0;
    spr->fillCircle(cx + radius * sin(radMaxR), cy - radius * cos(radMaxR), 5, TFT_RED);

    // ---- 6. Affichage Textuel des Angles ----
    // Angle Actuel
    spr->setTextDatum(MC_DATUM); 
    spr->setTextColor(TFT_WHITE, TFT_BLACK); // Fond noir explicite pour écraser le texte
    spr->drawFloat(abs(currentLeanAngle), 1, cx, cy + 40, 4); 
    
    // Angle Max Gauche
    spr->setTextDatum(ML_DATUM);
    spr->setTextColor(TFT_RED, TFT_BLACK);
    spr->drawFloat(abs(maxLeanLeft), 1, 10, cy + 70, 2);
    
    // Angle Max Droite
    spr->setTextDatum(MR_DATUM);
    spr->drawFloat(abs(maxLeanRight), 1, 230, cy + 70, 2);

    // ---- 7. Vitesse Actuelle ----
    spr->setTextDatum(MC_DATUM);
    spr->setTextColor(TFT_YELLOW, TFT_BLACK);
    spr->drawString(String((int)currentSpeed), cx, cy + 85, 4); 
    spr->drawString("km/h", cx, cy + 110, 2);
}