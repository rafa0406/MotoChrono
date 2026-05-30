#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum DisplayPage {
    PAGE_ROUTE,
    PAGE_PISTE
};

class DisplayManager {
public:
    static void init();
    static void update(bool isButtonPressed);
    static void showByeBye();

private:
    static void drawPageRoute();
    static void drawPagePiste();

    // Seul le Sprite (l'image lourde) reste dynamique !
    static TFT_eSprite* spr; 

    static DisplayPage currentPage;
    static unsigned long lastButtonPressMs;

    // Historique des inclinaisons pour la page Piste
    static float maxLeanLeft;
    static float maxLeanRight;
};

#endif // DISPLAYMANAGER_H