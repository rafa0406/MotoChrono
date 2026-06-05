#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_LOGO = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *logo;
    lv_obj_t *obj0;
} objects_t;

extern objects_t objects;

typedef struct {
    lv_meter_scale_t *Logo;
    lv_meter_indicator_t *tr_min;
} screen_logo_state_t;

extern screen_logo_state_t screen_logo_state;

void create_screen_logo();
void tick_screen_logo();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/