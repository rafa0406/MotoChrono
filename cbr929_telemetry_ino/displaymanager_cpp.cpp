/*
  =============================================================================
  CONFIGURATION REQUISE POUR LA BIBLIOTHÈQUE TFT_eSPI (User_Setup.h) :
  =============================================================================
  Pour faire fonctionner l'écran IPS 1.3" de la carte Waveshare ESP32-S3-LCD-1.3,
  vous devez modifier le fichier "User_Setup.h" situé dans le dossier de la 
  bibliothèque "TFT_eSPI" (dans votre dossier de bibliothèques Arduino). 
  
  Assurez-vous d'activer et de configurer les lignes suivantes :

  #define ST7789_DRIVER      // Sélection du contrôleur d'écran ST7789
  #define TFT_WIDTH  240     // Largeur de l'écran (pixels)
  #define TFT_HEIGHT 240     // Hauteur de l'écran (pixels)

  // --- Configuration des broches (Pins) internes de la carte Waveshare ---
  #define TFT_MOSI 41
  #define TFT_SCLK 40
  #define TFT_CS   39
  #define TFT_DC   42
  #define TFT_RST  -1        // Géré par le bouton Reset physique de la carte
  #define TFT_BL   43        // Contrôle du rétroéclairage de l'écran (Backlight)
  #define TFT_BACKLIGHT_ON HIGH // Rétroéclairage activé à l'état HAUT

  // --- Configuration de la vitesse du bus SPI ---
  #define SPI_FREQUENCY  40000000 // 40MHz pour une fluidité d'affichage optimale
  =============================================================================
*/

#include "DisplayManager.h"
#include "Config.h"

// Inclusions pour récupérer les données en direct
#include "FuelManager.h"
#include "GPSManager.h"
#include "IMUManager.h"

// Instanciation de l'écran et du sprite
TFT_eSPI DisplayManager::tft = TFT_eSPI(); 
TFT_eSprite DisplayManager::spr = TFT_eSprite(&tft);

DisplayPage DisplayManager::currentPage = PAGE_ROUTE;
unsigned long DisplayManager::lastButtonPressMs = 0;

void DisplayManager::init() {
    Serial.println(F("[DISPLAY] Initialisation de l'écran LCD..."));

    // Initialisation du hardware TFT
    tft.init();
    tft.setRotation(0); // Ajuster (0, 1, 2, 3) selon le sens de montage sur la moto
    tft.fillScreen(TFT_BLACK);

    // Création du Sprite 240x240 (Taille exacte de l'écran 1.3")
    spr.createSprite(240, 240);
    spr.setTextDatum(MC_DATUM); // Alignement du texte au centre par défaut

    // Écran de démarrage
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_RED);
    spr.drawString("CBR 929 RR", 120, 100, 4); // Police taille 4
    spr.setTextColor(TFT_WHITE);
    spr.drawString("System Init...", 120, 140, 2);
    spr.pushSprite(0, 0); // Envoi du sprite à l'écran

    delay(1500); // Petite pause pour le style
}

void DisplayManager::update(bool isButtonPressed) {
    unsigned long now = millis();

    // --- 1. GESTION DU BOUTON (Anti-rebond et changement de page) ---
    if (isButtonPressed && (now - lastButtonPressMs > 500)) {
        lastButtonPressMs = now;
        
        // Bascule entre les pages
        if (currentPage == PAGE_ROUTE) {
            currentPage = PAGE_PISTE;
        } else {
            currentPage = PAGE_ROUTE;
        }
    }

    // --- 2. DESSIN EN ARRIÈRE-PLAN (Dans le Sprite) ---
    spr.fillSprite(TFT_BLACK); // On efface la frame précédente

    // On délègue le dessin à la bonne fonction selon la page
    if (currentPage == PAGE_ROUTE) {
        drawPageRoute();
    } else {
        drawPagePiste();
    }

    // Indicateur de fixation GPS (Petit point en haut à droite)
    if (GPSManager::isDataValid()) {
        spr.fillCircle(230, 10, 4, TFT_GREEN);
    } else {
        spr.fillCircle(230, 10, 4, TFT_RED);
    }

    // --- 3. AFFICHAGE PHYSIQUE ---
    spr.pushSprite(0, 0); // Copie la mémoire RAM vers l'écran LCD d'un seul coup (Zéro scintillement)
}

void DisplayManager::drawPageRoute() {
    float fuelLiters = FuelManager::getRemainingLiters();
    float fuelPercent = (fuelLiters / TANK_CAPACITY_LITERS) * 100.0f;

    // --- JAUGE D'ESSENCE ---
    spr.setTextColor(TFT_WHITE);
    spr.drawString("CARBURANT", 120, 30, 2);

    // Choix de la couleur selon le niveau (Pragmatisme visuel)
    uint16_t fuelColor = TFT_GREEN;
    if (fuelPercent < 25.0f) fuelColor = TFT_ORANGE;
    if (fuelPercent < 15.0f) fuelColor = TFT_RED; // Équivalent voyant de réserve

    // Affichage Numérique
    char fuelStr[10];
    sprintf(fuelStr, "%.1f L", fuelLiters);
    spr.setTextColor(fuelColor);
    spr.drawString(fuelStr, 120, 80, 6); // Grosse police

    // Barre de progression graphique
    spr.drawRect(20, 130, 200, 20, TFT_WHITE);
    spr.fillRect(22, 132, (196 * fuelPercent / 100.0f), 16, fuelColor);

    // --- VITESSE ---
    spr.setTextColor(TFT_LIGHTGREY);
    spr.drawString("Vitesse (GPS)", 60, 190, 2);
    
    char speedStr[10];
    sprintf(speedStr, "%.0f", GPSManager::getSpeedKmh());
    spr.setTextColor(TFT_WHITE);
    spr.drawString(speedStr, 60, 215, 4);
    spr.drawString("km/h", 120, 220, 2);
}

void DisplayManager::drawPagePiste() {
    // Mode Piste : Focus sur la dynamique
    spr.setTextColor(TFT_RED);
    spr.drawString("TRACK MODE", 120, 20, 2);

    // Angle d'inclinaison
    spr.setTextColor(TFT_WHITE);
    spr.drawString("Angle", 60, 70, 2);
    char rollStr[10];
    sprintf(rollStr, "%.0f", abs(IMUManager::getRoll())); // abs() pour ne pas avoir de négatif
    spr.drawString(rollStr, 60, 100, 4);

    // Force G Longitudinale (Freinage)
    spr.drawString("G-Force", 180, 70, 2);
    char gStr[10];
    sprintf(gStr, "%.1f", IMUManager::getGForceLong());
    spr.drawString(gStr, 180, 100, 4);

    // Vitesse géante
    spr.drawString("Vitesse", 120, 160, 2);
    char speedStr[10];
    sprintf(speedStr, "%.0f", GPSManager::getSpeedKmh());
    spr.drawString(speedStr, 120, 200, 6); // Très grosse police pour le circuit
}

void DisplayManager::showByeBye() {
    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_RED);
    spr.drawString("OFF", 120, 100, 6);
    spr.setTextColor(TFT_WHITE);
    spr.drawString("Sauvegarde...", 120, 150, 2);
    spr.pushSprite(0, 0);
}