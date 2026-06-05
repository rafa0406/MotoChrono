#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_SPEED = 0,
    FLOW_GLOBAL_VARIABLE_MAX_SPEED = 1,
    FLOW_GLOBAL_VARIABLE_GPS_FIX_OK = 2,
    FLOW_GLOBAL_VARIABLE_GPS_FIX_KO = 3,
    FLOW_GLOBAL_VARIABLE_REC_START = 4,
    FLOW_GLOBAL_VARIABLE_FUEL_MAX = 5,
    FLOW_GLOBAL_VARIABLE_FUEL_RESERVE = 6,
    FLOW_GLOBAL_VARIABLE_FUEL_VALUE = 7,
    FLOW_GLOBAL_VARIABLE_FUEL_AVERAGE_VALUE = 8,
    FLOW_GLOBAL_VARIABLE_FUEL_IS_RESEVE = 9,
    FLOW_GLOBAL_VARIABLE_FUEL_VALUE_INT = 10,
    FLOW_GLOBAL_VARIABLE_G_FORCE = 11,
    FLOW_GLOBAL_VARIABLE_ROLL_VALUE = 12,
    FLOW_GLOBAL_VARIABLE_PITCH_VALUE = 13,
    FLOW_GLOBAL_VARIABLE_ROLL_VALUE_INT = 14,
    FLOW_GLOBAL_VARIABLE_PITCH_VALUE_INT = 15
};

// Native global variables

extern const char *get_var_speed();
extern void set_var_speed(const char *value);
extern const char *get_var_max_speed();
extern void set_var_max_speed(const char *value);
extern bool get_var_gps_fix_ok();
extern void set_var_gps_fix_ok(bool value);
extern bool get_var_gps_fix_ko();
extern void set_var_gps_fix_ko(bool value);
extern bool get_var_rec_start();
extern void set_var_rec_start(bool value);
extern float get_var_fuel_max();
extern void set_var_fuel_max(float value);
extern float get_var_fuel_reserve();
extern void set_var_fuel_reserve(float value);
extern const char *get_var_fuel_value();
extern void set_var_fuel_value(const char *value);
extern const char *get_var_fuel_average_value();
extern void set_var_fuel_average_value(const char *value);
extern bool get_var_fuel_is_reseve();
extern void set_var_fuel_is_reseve(bool value);
extern int32_t get_var_fuel_value_int();
extern void set_var_fuel_value_int(int32_t value);
extern const char *get_var_g_force();
extern void set_var_g_force(const char *value);
extern const char *get_var_roll_value();
extern void set_var_roll_value(const char *value);
extern const char *get_var_pitch_value();
extern void set_var_pitch_value(const char *value);
extern int32_t get_var_roll_value_int();
extern void set_var_roll_value_int(int32_t value);
extern int32_t get_var_pitch_value_int();
extern void set_var_pitch_value_int(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/