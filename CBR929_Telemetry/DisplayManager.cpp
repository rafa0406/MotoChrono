#include "DisplayManager.h"
#include "Config.h"
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h"

// Il faut penser à remplacer le fichier User_Setup.h de la librairie TFT_eSPI par celui fournis dans le répot (libraries\TFT_eSPI\User_Setup.h)

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite* DisplayManager::spr = nullptr;

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;

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
    
    if (spr != nullptr && spr->createSprite(240, 240) != nullptr) {
        spr->setTextDatum(MC_DATUM); 
        spr->fillSprite(TFT_BLACK);
        
        // Cette fois le ROUGE sera vraiment ROUGE !
        spr->setTextColor(TFT_RED);
        spr->drawString("CBR 929 RR", 120, 100, 4); 
        
        spr->setTextColor(TFT_WHITE);
        spr->drawString("System Init...", 120, 140, 2);
        
        spr->pushSprite(0, 0); 
        Serial.println(F("[DISPLAY] 4. Init terminée avec succès !"));
        
        delay(1500); // Petite pause stylée pour admirer le logo
    } else {
        Serial.println(F("[DISPLAY] ❌ ERREUR : PSRAM insuffisante !"));
    }
}

void DisplayManager::update(bool isButtonPressed) {
    if (spr == nullptr) return; 

    if (isButtonPressed && (millis() - lastButtonPressMs > 300)) {
        currentPage = (currentPage == PAGE_ROUTE) ? PAGE_PISTE : PAGE_ROUTE;
        lastButtonPressMs = millis();
    }

    spr->fillSprite(TFT_BLACK);

    if (currentPage == PAGE_ROUTE) {
        drawPageRoute();
    } else {
        drawPagePiste();
    }

    spr->pushSprite(0, 0);
}

void DisplayManager::drawPageRoute() {
    spr->drawFastHLine(0, 120, 240, TFT_DARKGREY);

    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(MC_DATUM);
    spr->drawString(String((int)GPSManager::getSpeedKmh()), 120, 50, 7);
    spr->drawString("km/h", 120, 90, 2);

    float remaining = FuelManager::getRemainingLiters();
    int color = (remaining > 5.0) ? TFT_GREEN : TFT_RED;
    
    spr->setTextColor(color);
    spr->drawString(String(remaining, 1) + " L", 120, 170, 6);
    spr->setTextColor(TFT_WHITE);
    spr->drawString("Essence", 120, 210, 2);

    // Le point GPS sera bien rouge s'il ne capte pas
    spr->fillCircle(220, 20, 5, GPSManager::isDataValid() ? TFT_GREEN : TFT_RED);
}

void DisplayManager::drawPagePiste() {
    spr->setTextColor(TFT_WHITE);
    spr->setTextDatum(ML_DATUM);
    spr->drawString("Roll: " + String(IMUManager::getRoll(), 1) + " deg", 10, 40, 4);
    spr->drawString("Pitch: " + String(IMUManager::getPitch(), 1) + " deg", 10, 80, 4);
    spr->drawString("G-Tot: " + String(IMUManager::getGForceTotal(), 2), 10, 140, 4);
    spr->drawString("G-Lon: " + String(IMUManager::getGForceLong(), 2), 10, 180, 4);
}

void DisplayManager::showByeBye() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Coupure Moteur", 120, 120, 4);
}