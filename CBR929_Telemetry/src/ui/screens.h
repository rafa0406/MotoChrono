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
    SCREEN_ID_SCREEN_ROUTE = 2,
    SCREEN_ID_SCREEN_PISTE = 3,
    SCREEN_ID_SCREEN_TROPHEES = 4,
    SCREEN_ID_SCREEN_CALIBRATION = 5,
    SCREEN_ID_SCREEN_GPS = 6,
    SCREEN_ID_SCREEN_WI_FI = 7,
    _SCREEN_ID_LAST = 7
};

typedef struct _objects_t {
    lv_obj_t *logo;
    lv_obj_t *screen_route;
    lv_obj_t *screen_piste;
    lv_obj_t *screen_trophees;
    lv_obj_t *screen_calibration;
    lv_obj_t *screen_gps;
    lv_obj_t *screen_wi_fi;
    lv_obj_t *compte_tour_logo;
    lv_obj_t *text_moto_chrono_logo;
    lv_obj_t *text_signature_logo;
    lv_obj_t *label_titre_route;
    lv_obj_t *wdg_route_speed;
    lv_obj_t *wdg_route_speed__label_speed;
    lv_obj_t *wdg_route_speed__label_kmh;
    lv_obj_t *wdg_route_gps;
    lv_obj_t *wdg_route_gps__label_gps;
    lv_obj_t *wdg_route_gps__led_gps_green;
    lv_obj_t *wdg_route_gps__led_gps_red;
    lv_obj_t *wdg_route_rec;
    lv_obj_t *wdg_route_rec__label_rec;
    lv_obj_t *wdg_route_rec__led_rec;
    lv_obj_t *wdg_route_max_speed;
    lv_obj_t *wdg_route_max_speed__label_max_speed;
    lv_obj_t *wdg_route_max_speed__label_kmh_max;
    lv_obj_t *ligne_horizontale;
    lv_obj_t *led_route_fuel_in_reserve;
    lv_obj_t *bar_route_fuel;
    lv_obj_t *label_titre_reservoir;
    lv_obj_t *label_route_fuel_liters_value;
    lv_obj_t *label_l;
    lv_obj_t *label_route_fuel_liters_value_average;
    lv_obj_t *label_l_100km;
    lv_obj_t *ligne_verticale;
    lv_obj_t *label_titre_piste;
    lv_obj_t *wdg_route_speed_1;
    lv_obj_t *wdg_route_speed_1__label_speed;
    lv_obj_t *wdg_route_speed_1__label_kmh;
    lv_obj_t *wdg_route_gps_1;
    lv_obj_t *wdg_route_gps_1__label_gps;
    lv_obj_t *wdg_route_gps_1__led_gps_green;
    lv_obj_t *wdg_route_gps_1__led_gps_red;
    lv_obj_t *wdg_route_rec_1;
    lv_obj_t *wdg_route_rec_1__label_rec;
    lv_obj_t *wdg_route_rec_1__led_rec;
    lv_obj_t *wdg_route_max_speed_1;
    lv_obj_t *wdg_route_max_speed_1__label_max_speed;
    lv_obj_t *wdg_route_max_speed_1__label_kmh_max;
    lv_obj_t *slider_piste_pitch;
    lv_obj_t *led_route_fuel_in_reserve_1;
    lv_obj_t *label_piste_gforce;
    lv_obj_t *label_piste_g;
    lv_obj_t *slider_piste_roll;
    lv_obj_t *label_piste_pitch;
    lv_obj_t *label_piste_roll;
    lv_obj_t *label_piste_roll_degres;
    lv_obj_t *label_piste_roll_degres_1;
} objects_t;

extern objects_t objects;

typedef struct {
    lv_meter_scale_t *Logo;
    lv_meter_indicator_t *tr_min;
} screen_logo_state_t;

extern screen_logo_state_t screen_logo_state;

void create_screen_logo();
void tick_screen_logo();

void create_screen_screen_route();
void tick_screen_screen_route();

void create_screen_screen_piste();
void tick_screen_screen_piste();

void create_screen_screen_trophees();
void tick_screen_screen_trophees();

void create_screen_screen_calibration();
void tick_screen_screen_calibration();

void create_screen_screen_gps();
void tick_screen_screen_gps();

void create_screen_screen_wi_fi();
void tick_screen_screen_wi_fi();

void create_user_widget_wdg_speed(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_wdg_speed(int startWidgetIndex);

void create_user_widget_wdg_max_speed(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_wdg_max_speed(int startWidgetIndex);

void create_user_widget_wdg_gps_fix(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_wdg_gps_fix(int startWidgetIndex);

void create_user_widget_wdg_rec(lv_obj_t *parent_obj, int startWidgetIndex);
void tick_user_widget_wdg_rec(int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/