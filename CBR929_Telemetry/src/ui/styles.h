#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: titre
lv_style_t *get_style_titre_MAIN_DEFAULT();
void add_style_titre(lv_obj_t *obj);
void remove_style_titre(lv_obj_t *obj);

// Style: speed
lv_style_t *get_style_speed_MAIN_DEFAULT();
void add_style_speed(lv_obj_t *obj);
void remove_style_speed(lv_obj_t *obj);

// Style: kmh
lv_style_t *get_style_kmh_MAIN_DEFAULT();
void add_style_kmh(lv_obj_t *obj);
void remove_style_kmh(lv_obj_t *obj);

// Style: speed_max
lv_style_t *get_style_speed_max_MAIN_DEFAULT();
void add_style_speed_max(lv_obj_t *obj);
void remove_style_speed_max(lv_obj_t *obj);

// Style: kmh_max
lv_style_t *get_style_kmh_max_MAIN_DEFAULT();
void add_style_kmh_max(lv_obj_t *obj);
void remove_style_kmh_max(lv_obj_t *obj);

// Style: GPS
lv_style_t *get_style_gps_MAIN_CHECKED();
void add_style_gps(lv_obj_t *obj);
void remove_style_gps(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/