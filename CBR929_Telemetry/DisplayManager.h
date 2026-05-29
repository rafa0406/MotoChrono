#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h> // Bibliothèque ultra-rapide pour écran TFT

// Enumération pour nos différentes pages d'affichage
enum DisplayPage {
    PAGE_ROUTE,
    PAGE_PISTE
};

class DisplayManager {
public:
    // Initialise l'écran et le sprite en mémoire
    static void init();

    // Met à jour l'affichage (à appeler sur le Core 1)
    // Prend en paramètre l'état brut du bouton pour gérer le changement de page
    static void update(bool isButtonPressed);

    // Affiche un écran de fin avant la coupure de l'alimentation
    static void showByeBye();

private:
    static TFT_eSPI tft;
    static TFT_eSprite spr; // Le "Sprite" pour un affichage sans clignotement

    static DisplayPage currentPage;
    static unsigned long lastButtonPressMs;

    // Fonctions de dessin spécifiques à chaque page
    static void drawPageRoute();
    static void drawPagePiste();
};

#endif // DISPLAY_MANAGER_H