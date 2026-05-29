
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "esp_timer.h"
#include "esp_lcd_sh8601.h"
#include "i2c_bsp.h"
#include "touch_bsp.h"

#include "app_bsp.h"

#define EEPROM_HOST  SPI2_HOST
#define Rotate 1        
#define Normal 0        
#define Direction Normal

#if (Direction == Normal) 
  #define EXAMPLE_LCD_H_RES 240   
  #define EXAMPLE_LCD_V_RES 240   
#elif (Direction == Rotate)
  #define EXAMPLE_LCD_H_RES 240   
  #define EXAMPLE_LCD_V_RES 240   
#endif

#define EXAMPLE_LCD_DMA_Line (EXAMPLE_LCD_V_RES / 2)

#define EXAMPLE_USE_TOUCH   0 

#define PIN_NUM_MOSI 41
#define PIN_NUM_CLK  40
#define PIN_NUM_CS   39
#define PIN_NUM_RST  42
#define PIN_NUM_DC   38
#define PIN_NUM_BL   4
#define lcd_rest_0    gpio_set_level(PIN_NUM_RST,0)
#define lcd_rest_1    gpio_set_level(PIN_NUM_RST,1)
#define lcd_bl_0      gpio_set_level(PIN_NUM_BL,0)
#define lcd_bl_1      gpio_set_level(PIN_NUM_BL,1)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define LCD_CMD
#ifdef  LCD_CMD
static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = 
{
  #if (Direction == Normal) 
  {0x36, (uint8_t []){0xC0}, 1, 0},	//鳞镜是0x90    C0
  #elif (Direction == Rotate)
  {0x36, (uint8_t []){0x70}, 1, 0},	
  #endif
#if 1
  //{0x36, (uint8_t []){0x00}, 1, 0},	
  {0x3A, (uint8_t []){0x05}, 1, 0},	
  {0xB2, (uint8_t []){0x0C,0x0C,0x00,0x33,0x33}, 5, 0},	
  {0xB4, (uint8_t []){0x01}, 1, 0},	
  {0xC0, (uint8_t []){0x2C,0x2D}, 2, 0},
  {0xC5, (uint8_t []){0x2E}, 1, 0},
  {0x21, (uint8_t []){0x2E}, 0, 0},
  {0x2A, (uint8_t []){0x00,0x00,0x00,0xEF}, 4, 0},
  {0x2B, (uint8_t []){0x00,0x00,0x00,0xEF}, 4, 0},
  {0x2C, (uint8_t []){0x00}, 0, 0},
  {0x11, (uint8_t []){0x00}, 0, 120},
  {0x29, (uint8_t []){0x29}, 0, 0},
#endif
};
#endif
#if EXAMPLE_USE_TOUCH
static void example_lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  uint16_t tp_x;
  uint16_t tp_y;
  uint8_t win = 0;
  win = getTouch(&tp_x,&tp_y);
  if(win)
  {
    data->point.x = tp_x;
    data->point.y = tp_y;
    data->state = LV_INDEV_STATE_PRESSED;
    ESP_LOGD("tp_user", "Touch position: %d,%d", tp_x, tp_y);
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
#endif
static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
  lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
  lv_disp_flush_ready(disp_driver);
  return false;
}
static void example_increase_lvgl_tick(void *arg)
{
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}
static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
#if (Direction == Normal) 
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1 + 80;
  int offsety2 = area->y2 + 80;
#elif (Direction == Rotate)
  int offsetx1 = area->x1;
  int offsetx2 = area->x2;
  int offsety1 = area->y1;
  int offsety2 = area->y2;
#endif
  // copy a buffer's content to a specific area of the display
  esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}
void app_main(void)
{
  static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
  static lv_disp_drv_t disp_drv;      // contains callback functions

  gpio_config_t gpio_conf = {};
  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_OUTPUT;
  gpio_conf.pin_bit_mask = ((uint64_t)0X01<<PIN_NUM_BL) | ((uint64_t)0X01<<PIN_NUM_RST);
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf)); 
  lcd_bl_1;

  spi_bus_config_t buscfg = 
  {
    .miso_io_num = -1,
    .mosi_io_num = PIN_NUM_MOSI,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz =  EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DMA_Line * sizeof(uint16_t), // RGB565 , 传输屏幕的1/10行的数据
  };
  ESP_ERROR_CHECK(spi_bus_initialize(EEPROM_HOST, &buscfg, SPI_DMA_CH_AUTO));
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_config = {
    .dc_gpio_num = PIN_NUM_DC,
    .cs_gpio_num = PIN_NUM_CS,
    .pclk_hz = 20 * 1000 * 1000,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .spi_mode = 0,
    .trans_queue_depth = 10,
    .on_color_trans_done = example_notify_lvgl_flush_ready,
    .user_ctx = &disp_drv,
  };
#ifdef  LCD_CMD
  sh8601_vendor_config_t vendor_config = 
  {
    .init_cmds = lcd_init_cmds,
    .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
  };
#endif
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EEPROM_HOST, &io_config, &io_handle));

  esp_lcd_panel_handle_t panel_handle = NULL;
  const esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = PIN_NUM_RST,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    .bits_per_pixel = 16,
#ifdef  LCD_CMD
    .vendor_config = &vendor_config,
#endif
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
  };

  ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  //ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
#if EXAMPLE_USE_TOUCH
  I2C_master_Init();
#endif
  lv_init();
  lv_color_t *buf1 = heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DMA_Line * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf1);
  lv_color_t *buf2 = heap_caps_malloc(EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DMA_Line * sizeof(lv_color_t), MALLOC_CAP_DMA);
  assert(buf2);
  lv_disp_draw_buf_init(&disp_buf, buf1, buf2, EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DMA_Line);

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EXAMPLE_LCD_H_RES;
  disp_drv.ver_res = EXAMPLE_LCD_V_RES;
  disp_drv.flush_cb = example_lvgl_flush_cb;
  disp_drv.draw_buf = &disp_buf;
  disp_drv.user_data = panel_handle;
  lv_disp_drv_register(&disp_drv);
#if EXAMPLE_USE_TOUCH
  static lv_indev_drv_t indev_drv;    // Input device driver (Touch)
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = example_lvgl_touch_cb;
  lv_indev_drv_register(&indev_drv);
#endif
  const esp_timer_create_args_t lvgl_tick_timer_args = 
  {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));
  //lv_demo_widgets();
  //lv_demo_music();
  app_Init();
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(10));
    lv_timer_handler();
  }
}