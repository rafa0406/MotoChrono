#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: titre
//

void init_style_titre_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_lighten(lv_color_hex(0xffffff), 255));
    lv_style_set_text_font(style, &lv_font_montserrat_16);
};

lv_style_t *get_style_titre_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_titre_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_titre(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_titre_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_titre(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_titre_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: speed
//

void init_style_speed_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xf6ff00));
    lv_style_set_text_font(style, &lv_font_montserrat_48);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_RIGHT);
    lv_style_set_align(style, LV_ALIGN_TOP_RIGHT);
};

lv_style_t *get_style_speed_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_speed_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_speed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_speed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_speed(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_speed_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: kmh
//

void init_style_kmh_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xf6ff00));
    lv_style_set_text_font(style, &lv_font_montserrat_14);
};

lv_style_t *get_style_kmh_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_kmh_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_kmh(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_kmh_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_kmh(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_kmh_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: speed_max
//

void init_style_speed_max_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0x00fffd));
    lv_style_set_text_font(style, &lv_font_montserrat_48);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_RIGHT);
    lv_style_set_align(style, LV_ALIGN_TOP_RIGHT);
};

lv_style_t *get_style_speed_max_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_speed_max_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_speed_max(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_speed_max_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_speed_max(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_speed_max_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: kmh_max
//

void init_style_kmh_max_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0x00fffd));
    lv_style_set_text_font(style, &lv_font_montserrat_14);
};

lv_style_t *get_style_kmh_max_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_kmh_max_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_kmh_max(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_kmh_max_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_kmh_max(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_kmh_max_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
// Style: GPS
//

void init_style_gps_MAIN_CHECKED(lv_style_t *style) {
    lv_style_set_text_color(style, lv_color_hex(0xffffff));
};

lv_style_t *get_style_gps_MAIN_CHECKED() {
    static lv_style_t *style;
    if (!style) {
        style = (lv_style_t *)lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_gps_MAIN_CHECKED(style);
    }
    return style;
};

void add_style_gps(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_gps_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

void remove_style_gps(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_gps_MAIN_CHECKED(), LV_PART_MAIN | LV_STATE_CHECKED);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_titre,
        add_style_speed,
        add_style_kmh,
        add_style_speed_max,
        add_style_kmh_max,
        add_style_gps,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_titre,
        remove_style_speed,
        remove_style_kmh,
        remove_style_speed_max,
        remove_style_kmh_max,
        remove_style_gps,
    };
    remove_style_funcs[styleIndex](obj);
}