#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

screen_logo_state_t screen_logo_state;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_screen_piste_slider_piste_pitch(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_slider_get_value(ta);
            set_var_roll_value_int(value);
        }
    }
}

static void event_handler_cb_screen_piste_slider_piste_roll(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_slider_get_value(ta);
            set_var_pitch_value_int(value);
        }
    }
}

//
// Screens
//

void create_screen_logo() {
    screen_logo_state_t *state = &screen_logo_state;
    (void)state;
    lv_obj_t *obj = lv_obj_create(0);
    objects.logo = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // compte_tour_logo
            lv_obj_t *obj = lv_meter_create(parent_obj);
            objects.compte_tour_logo = obj;
            lv_obj_set_pos(obj, 15, 15);
            lv_obj_set_size(obj, 210, 210);
            {
                lv_meter_scale_t *scale = lv_meter_add_scale(obj);
                state->Logo = scale;
                lv_meter_set_scale_ticks(obj, scale, 27, 2, 8, lv_color_hex(0xff0000));
                lv_meter_set_scale_major_ticks(obj, scale, 2, 3, 10, lv_color_hex(0xff0000), 10);
                lv_meter_set_scale_range(obj, scale, 0, 13, 230, 90);
                {
                    lv_meter_indicator_t *indicator = lv_meter_add_needle_line(obj, scale, 8, lv_color_hex(0xff0000), -28);
                    state->tr_min = indicator;
                    lv_meter_set_indicator_value(obj, indicator, 0);
                }
            }
        }
        {
            // text_moto_chrono_logo
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_moto_chrono_logo = obj;
            lv_obj_set_pos(obj, 132, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Moto\nChrono");
        }
        {
            // text_signature_logo
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.text_signature_logo = obj;
            lv_obj_set_pos(obj, 205, 209);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_decor(obj, LV_TEXT_DECOR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RC");
        }
    }
    
    tick_screen_logo();
}

void tick_screen_logo() {
    screen_logo_state_t *state = &screen_logo_state;
    (void)state;
}

void create_screen_screen_route() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_route = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_titre_route
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_titre_route = obj;
            lv_obj_set_pos(obj, 64, 2);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_titre(obj);
            lv_label_set_text_static(obj, "MODE ROUTE");
        }
        {
            // wdg_route_speed
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_speed = obj;
            lv_obj_set_pos(obj, 20, 31);
            lv_obj_set_size(obj, 100, 65);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_speed(obj, 12);
        }
        {
            // wdg_route_gps
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_gps = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 60, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_gps_fix(obj, 15);
        }
        {
            // wdg_route_rec
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_rec = obj;
            lv_obj_set_pos(obj, 180, 0);
            lv_obj_set_size(obj, 60, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_rec(obj, 19);
        }
        {
            // wdg_route_max_speed
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_max_speed = obj;
            lv_obj_set_pos(obj, 120, 31);
            lv_obj_set_size(obj, 100, 65);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_max_speed(obj, 22);
        }
        {
            // ligne_horizontale
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.ligne_horizontale = obj;
            lv_obj_set_pos(obj, 0, 100);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 240, 0 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // led_route_fuel_in_reserve
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.led_route_fuel_in_reserve = obj;
            lv_obj_set_pos(obj, 181, 113);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xffa000));
            lv_led_set_brightness(obj, 255);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
        }
        {
            // bar_route_fuel
            lv_obj_t *obj = lv_bar_create(parent_obj);
            objects.bar_route_fuel = obj;
            lv_obj_set_pos(obj, 13, 181);
            lv_obj_set_size(obj, 214, 41);
            lv_bar_set_range(obj, 0, 100);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xff0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x29ff00), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_grad_stop(obj, 100, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x42f321), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // label_titre_reservoir
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_titre_reservoir = obj;
            lv_obj_set_pos(obj, 72, 103);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_titre(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x59ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RESERVOIR");
        }
        {
            // label_route_fuel_liters_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_route_fuel_liters_value = obj;
            lv_obj_set_pos(obj, 12, 124);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00ff1c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_L
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_l = obj;
            lv_obj_set_pos(obj, 104, 148);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00ff39), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "L");
        }
        {
            // label_route_fuel_liters_value_average
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_route_fuel_liters_value_average = obj;
            lv_obj_set_pos(obj, 132, 150);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00ff39), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_L_100km
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_l_100km = obj;
            lv_obj_set_pos(obj, 165, 154);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00ff39), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "L/100km");
        }
        {
            // ligne_verticale
            lv_obj_t *obj = lv_line_create(parent_obj);
            objects.ligne_verticale = obj;
            lv_obj_set_pos(obj, 125, 124);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            static lv_point_t line_points[] = {
                { 0, 0 },
                { 0, 50 }
            };
            lv_line_set_points(obj, line_points, 2);
            lv_obj_set_style_line_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_color(obj, lv_color_hex(0x5e5e5e), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_screen_route();
}

void tick_screen_screen_route() {
    tick_user_widget_wdg_speed(12);
    tick_user_widget_wdg_gps_fix(15);
    tick_user_widget_wdg_rec(19);
    tick_user_widget_wdg_max_speed(22);
    {
        bool new_val = get_var_fuel_is_reseve();
        bool cur_val = lv_obj_has_flag(objects.led_route_fuel_in_reserve, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_route_fuel_in_reserve;
            if (new_val) {
                lv_obj_add_flag(objects.led_route_fuel_in_reserve, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.led_route_fuel_in_reserve, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_fuel_max();
        int32_t cur_val = lv_bar_get_max_value(objects.bar_route_fuel);
        if (new_val != cur_val) {
            int16_t min = lv_bar_get_min_value(objects.bar_route_fuel);
            int16_t max = new_val;
            if (min < max) {
                lv_bar_set_range(objects.bar_route_fuel, min, max);
            }
        }
    }
    {
        int32_t new_val = get_var_fuel_value_int();
        int32_t cur_val = lv_bar_get_value(objects.bar_route_fuel);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.bar_route_fuel;
            lv_bar_set_value(objects.bar_route_fuel, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_fuel_value();
        const char *cur_val = lv_label_get_text(objects.label_route_fuel_liters_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_route_fuel_liters_value;
            lv_label_set_text(objects.label_route_fuel_liters_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_fuel_average_value();
        const char *cur_val = lv_label_get_text(objects.label_route_fuel_liters_value_average);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_route_fuel_liters_value_average;
            lv_label_set_text(objects.label_route_fuel_liters_value_average, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_screen_piste() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_piste = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_titre_piste
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_titre_piste = obj;
            lv_obj_set_pos(obj, 69, 2);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_titre(obj);
            lv_label_set_text_static(obj, "MODE PISTE");
        }
        {
            // wdg_route_speed_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_speed_1 = obj;
            lv_obj_set_pos(obj, 20, 31);
            lv_obj_set_size(obj, 100, 65);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_speed(obj, 35);
        }
        {
            // wdg_route_gps_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_gps_1 = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 60, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_gps_fix(obj, 38);
        }
        {
            // wdg_route_rec_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_rec_1 = obj;
            lv_obj_set_pos(obj, 180, 0);
            lv_obj_set_size(obj, 60, 20);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_rec(obj, 42);
        }
        {
            // wdg_route_max_speed_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.wdg_route_max_speed_1 = obj;
            lv_obj_set_pos(obj, 120, 31);
            lv_obj_set_size(obj, 100, 65);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            create_user_widget_wdg_max_speed(obj, 45);
        }
        {
            // slider_piste_pitch
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.slider_piste_pitch = obj;
            lv_obj_set_pos(obj, 14, 114);
            lv_obj_set_size(obj, 6, 100);
            lv_slider_set_range(obj, -90, 90);
            lv_obj_add_event_cb(obj, event_handler_cb_screen_piste_slider_piste_pitch, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2f21f3), LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // led_route_fuel_in_reserve_1
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.led_route_fuel_in_reserve_1 = obj;
            lv_obj_set_pos(obj, 181, 113);
            lv_obj_set_size(obj, 32, 32);
            lv_led_set_color(obj, lv_color_hex(0xffa000));
            lv_led_set_brightness(obj, 255);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
        }
        {
            // label_piste_gforce
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_gforce = obj;
            lv_obj_set_pos(obj, -95, 107);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x29ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_40, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_TOP_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_piste_g
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_g = obj;
            lv_obj_set_pos(obj, 83, 142);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x1fff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "G-force");
        }
        {
            // slider_piste_roll
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.slider_piste_roll = obj;
            lv_obj_set_pos(obj, 30, 208);
            lv_obj_set_size(obj, 180, 6);
            lv_slider_set_range(obj, -90, 90);
            lv_obj_add_event_cb(obj, event_handler_cb_screen_piste_slider_piste_roll, LV_EVENT_ALL, 0);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2f21f3), LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // label_piste_pitch
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_pitch = obj;
            lv_obj_set_pos(obj, -81, -65);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00e3ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_piste_roll
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_roll = obj;
            lv_obj_set_pos(obj, -1, -38);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00e3ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_piste_roll_degres
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_roll_degres = obj;
            lv_obj_set_pos(obj, 130, 180);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00e3ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // label_piste_roll_degres_1
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.label_piste_roll_degres_1 = obj;
            lv_obj_set_pos(obj, 50, 152);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00e3ff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
    }
    
    tick_screen_screen_piste();
}

void tick_screen_screen_piste() {
    tick_user_widget_wdg_speed(35);
    tick_user_widget_wdg_gps_fix(38);
    tick_user_widget_wdg_rec(42);
    tick_user_widget_wdg_max_speed(45);
    {
        int32_t new_val = get_var_roll_value_int();
        int32_t cur_val = lv_slider_get_value(objects.slider_piste_pitch);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.slider_piste_pitch;
            lv_slider_set_value(objects.slider_piste_pitch, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_fuel_is_reseve();
        bool cur_val = lv_obj_has_flag(objects.led_route_fuel_in_reserve_1, LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.led_route_fuel_in_reserve_1;
            if (new_val) {
                lv_obj_add_flag(objects.led_route_fuel_in_reserve_1, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(objects.led_route_fuel_in_reserve_1, LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_g_force();
        const char *cur_val = lv_label_get_text(objects.label_piste_gforce);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_piste_gforce;
            lv_label_set_text(objects.label_piste_gforce, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = get_var_pitch_value_int();
        int32_t cur_val = lv_slider_get_value(objects.slider_piste_roll);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.slider_piste_roll;
            lv_slider_set_value(objects.slider_piste_roll, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_roll_value();
        const char *cur_val = lv_label_get_text(objects.label_piste_pitch);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_piste_pitch;
            lv_label_set_text(objects.label_piste_pitch, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_pitch_value();
        const char *cur_val = lv_label_get_text(objects.label_piste_roll);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_piste_roll;
            lv_label_set_text(objects.label_piste_roll, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_pitch_value();
        const char *cur_val = lv_label_get_text(objects.label_piste_roll_degres);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_piste_roll_degres;
            lv_label_set_text(objects.label_piste_roll_degres, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = get_var_pitch_value();
        const char *cur_val = lv_label_get_text(objects.label_piste_roll_degres_1);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.label_piste_roll_degres_1;
            lv_label_set_text(objects.label_piste_roll_degres_1, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_screen_trophees() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_trophees = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    
    tick_screen_screen_trophees();
}

void tick_screen_screen_trophees() {
}

void create_screen_screen_calibration() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_calibration = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    
    tick_screen_screen_calibration();
}

void tick_screen_screen_calibration() {
}

void create_screen_screen_gps() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_gps = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    
    tick_screen_screen_gps();
}

void tick_screen_screen_gps() {
}

void create_screen_screen_wi_fi() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.screen_wi_fi = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 240);
    
    tick_screen_screen_wi_fi();
}

void tick_screen_screen_wi_fi() {
}

void create_user_widget_wdg_speed(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_speed
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, -9, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_speed(obj);
            lv_label_set_text(obj, "");
        }
        {
            // label_kmh
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
            lv_obj_set_pos(obj, -9, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_kmh(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xfaff00), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(obj, LV_ALIGN_BOTTOM_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Km/h");
        }
    }
}

void tick_user_widget_wdg_speed(int startWidgetIndex) {
    (void)startWidgetIndex;
    {
        const char *new_val = get_var_speed();
        const char *cur_val = lv_label_get_text(((lv_obj_t **)&objects)[startWidgetIndex + 0]);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 0];
            lv_label_set_text(((lv_obj_t **)&objects)[startWidgetIndex + 0], new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_user_widget_wdg_max_speed(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_max_speed
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, -9, 0);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_speed_max(obj);
            lv_label_set_text(obj, "");
        }
        {
            // label_kmh_max
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
            lv_obj_set_pos(obj, 16, 49);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_kmh_max(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x00fffd), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MAX km/h");
        }
    }
}

void tick_user_widget_wdg_max_speed(int startWidgetIndex) {
    (void)startWidgetIndex;
    {
        const char *new_val = get_var_max_speed();
        const char *cur_val = lv_label_get_text(((lv_obj_t **)&objects)[startWidgetIndex + 0]);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 0];
            lv_label_set_text(((lv_obj_t **)&objects)[startWidgetIndex + 0], new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_user_widget_wdg_gps_fix(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_gps
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, 6, 2);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            add_style_gps(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "GPS");
        }
        {
            // led_gps_green
            lv_obj_t *obj = lv_led_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
            lv_obj_set_pos(obj, 45, 5);
            lv_obj_set_size(obj, 10, 10);
            lv_led_set_color(obj, lv_color_hex(0x1aff00));
            lv_led_set_brightness(obj, 255);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
        {
            // led_gps_red
            lv_obj_t *obj = lv_led_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 2] = obj;
            lv_obj_set_pos(obj, 45, 5);
            lv_obj_set_size(obj, 10, 10);
            lv_led_set_color(obj, lv_color_hex(0xff0000));
            lv_led_set_brightness(obj, 255);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
    }
}

void tick_user_widget_wdg_gps_fix(int startWidgetIndex) {
    (void)startWidgetIndex;
    {
        bool new_val = get_var_gps_fix_ko();
        bool cur_val = lv_obj_has_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 1];
            if (new_val) {
                lv_obj_add_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_gps_fix_ok();
        bool cur_val = lv_obj_has_flag(((lv_obj_t **)&objects)[startWidgetIndex + 2], LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 2];
            if (new_val) {
                lv_obj_add_flag(((lv_obj_t **)&objects)[startWidgetIndex + 2], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(((lv_obj_t **)&objects)[startWidgetIndex + 2], LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_user_widget_wdg_rec(lv_obj_t *parent_obj, int startWidgetIndex) {
    (void)startWidgetIndex;
    lv_obj_t *obj = parent_obj;
    {
        lv_obj_t *parent_obj = obj;
        {
            // label_rec
            lv_obj_t *obj = lv_label_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 0] = obj;
            lv_obj_set_pos(obj, 9, 2);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            add_style_gps(obj);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_opa(obj, 0, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_label_set_text_static(obj, "REC");
        }
        {
            // led_rec
            lv_obj_t *obj = lv_led_create(parent_obj);
            ((lv_obj_t **)&objects)[startWidgetIndex + 1] = obj;
            lv_obj_set_pos(obj, 45, 5);
            lv_obj_set_size(obj, 10, 10);
            lv_led_set_color(obj, lv_color_hex(0xff0000));
            lv_led_set_brightness(obj, 255);
            lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        }
    }
}

void tick_user_widget_wdg_rec(int startWidgetIndex) {
    (void)startWidgetIndex;
    {
        bool new_val = get_var_rec_start();
        bool cur_val = lv_obj_has_flag(((lv_obj_t **)&objects)[startWidgetIndex + 0], LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 0];
            if (new_val) {
                lv_obj_add_flag(((lv_obj_t **)&objects)[startWidgetIndex + 0], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(((lv_obj_t **)&objects)[startWidgetIndex + 0], LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = get_var_rec_start();
        bool cur_val = lv_obj_has_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
        if (new_val != cur_val) {
            tick_value_change_obj = ((lv_obj_t **)&objects)[startWidgetIndex + 1];
            if (new_val) {
                lv_obj_add_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(((lv_obj_t **)&objects)[startWidgetIndex + 1], LV_OBJ_FLAG_HIDDEN);
            }
            tick_value_change_obj = NULL;
        }
    }
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_logo,
    tick_screen_screen_route,
    tick_screen_screen_piste,
    tick_screen_screen_trophees,
    tick_screen_screen_calibration,
    tick_screen_screen_gps,
    tick_screen_screen_wi_fi,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 7) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_logo();
    create_screen_screen_route();
    create_screen_screen_piste();
    create_screen_screen_trophees();
    create_screen_screen_calibration();
    create_screen_screen_gps();
    create_screen_screen_wi_fi();
}