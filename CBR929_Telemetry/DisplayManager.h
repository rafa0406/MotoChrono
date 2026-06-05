#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

class DisplayManager {
public:
    static void init();
    static void update(); // Fait tourner le moteur graphique LVGL

private:
    // La fonction "Bridge" appelée par LVGL pour envoyer les pixels à l'écran
    static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    
    // Buffers d'affichage alloués dynamiquement en PSRAM
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *buf1;
};

#endif // DISPLAYMANAGER_H