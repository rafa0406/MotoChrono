#include "DisplayManager.h"
#include "Config.h"
#include "GPSManager.h"
#include "IMUManager.h"
#include "src/ui/ui.h" // L'interface générée par EEZ

// On réintègre ta gestion du bouton
unsigned long DisplayManager::lastButtonPressMs = 0;
int currentScreenIndex = 0; // 0=Route, 1=Piste, etc.

void DisplayManager::update(bool isButtonPressed) {
    unsigned long currentMillis = millis();

    // ==========================================
    // 1. MOTEUR LVGL : TEMPS ÉCOULÉ
    // ==========================================
    static uint32_t lastTick = 0;
    lv_tick_inc(currentMillis - lastTick);
    lastTick = currentMillis;

    // ==========================================
    // 2. GESTION DU BOUTON PHYSIQUE (Changement de page)
    // ==========================================
    if (isButtonPressed && (currentMillis - lastButtonPressMs > 300)) { 
        lastButtonPressMs = currentMillis;
        currentScreenIndex++;
        if (currentScreenIndex > 1) currentScreenIndex = 0; // Boucle sur 2 pages pour l'instant

        // On ordonne à LVGL de charger le nouvel écran sans délai
        if (currentScreenIndex == 0) {
            lv_scr_load(ui_ScreenRoute); // Nom de l'écran dans EEZ Studio
        } else if (currentScreenIndex == 1) {
            lv_scr_load(ui_ScreenPiste); // Nom de l'écran dans EEZ Studio
        }
    }

    // ==========================================
    // 3. INJECTION DES DONNÉES DE TÉLÉMÉTRIE DANS LVGL
    // ==========================================
    float speed = GPSManager::getSpeedKmh();
    float roll = IMUManager::getRoll();

    // OPTIMISATION : On met à jour les widgets uniquement s'ils existent (sécurité anti-crash)
    // et selon l'écran actif pour ne pas utiliser de CPU pour rien.
    
    // Exemple de mise à jour pour l'écran Route
    if (currentScreenIndex == 0) {
        if (ui_LabelSpeedRoute != NULL) {
            char speedStr[10];
            snprintf(speedStr, sizeof(speedStr), "%d", (int)speed);
            lv_label_set_text(ui_LabelSpeedRoute, speedStr);
        }
    }
    
    // Exemple de mise à jour pour l'écran Piste
    if (currentScreenIndex == 1) {
        if (ui_LabelSpeedPiste != NULL) {
            char speedStr[10];
            snprintf(speedStr, sizeof(speedStr), "%d", (int)speed);
            lv_label_set_text(ui_LabelSpeedPiste, speedStr);
        }
        
        if (ui_ArcAngle != NULL) {
            // L'arc s'attend à un entier
            lv_arc_set_value(ui_ArcAngle, abs((int)roll));
        }
    }

    // ==========================================
    // 4. MOTEUR LVGL : RAFRAÎCHISSEMENT DE L'ÉCRAN
    // ==========================================
    lv_timer_handler(); 
}