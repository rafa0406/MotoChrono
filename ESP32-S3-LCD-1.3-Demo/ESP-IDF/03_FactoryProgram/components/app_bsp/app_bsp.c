#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_bsp.h"
#include "gui_guider.h"
#include "leds_bsp.h"
#include "qmi8658.h"
#include "i2c_bsp.h"
#include "sd_card_bsp.h"
#include "adc_bsp.h"
#include "ble_scan_bsp.h"
#include "esp_wifi_bsp.h"
#include "esp_log.h"
#include "esp_err.h"
static char *TAG = "userApp";

lv_ui guider_ui;
void example_app_port_task(void *arg);
void example_rgb_port_task(void *arg);
void example_wsled_port_task(void *arg);
void example_lcd_bl_task(void *arg);
void example_scan_task(void *arg);
void example_tp_task(void *arg);
void app_Init(void)
{
  setup_ui(&guider_ui);
  pwm_init();
  setUpduty(0xff);
  xTaskCreate(example_app_port_task, "example_app_port_task", 3 * 1024, &guider_ui, 2, NULL);
  xTaskCreate(example_rgb_port_task, "example_rgb_port_task", 3 * 1024, &guider_ui, 2, NULL);
  xTaskCreate(example_wsled_port_task, "example_wsled_port_task", 3 * 1024, &guider_ui, 2, NULL);
  xTaskCreate(example_lcd_bl_task, "example_lcd_bl_task", 3 * 1024, &guider_ui, 2, NULL);
  xTaskCreate(example_scan_task, "example_scan_task", 3 * 1024, &guider_ui, 2, NULL);      //快速扫描WIFI BLE 事件
}
#if 1
void esp_wifi_ble_setscan(uint8_t mode)
{
  static uint8_t wifi_ble_flag = 0;
  if(mode != wifi_ble_flag)
  {
    if(mode) //wifi 需要释放ble
    {
      ble_scan_Deinit();
      espwifi_Init();
    }
    else
    {
      espwifi_Deinit();
      ble_scan_Init();
    }
    wifi_ble_flag = mode;
  }
}
void example_scan_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  nvs_flash_Init();
  ble_scan_class_init();
  ble_scan_Init();
  ble_scan_setconf();
  uint8_t mac[6];
  uint8_t connt = 0;
  char lv_buf_set[30] = {0};
  //espwifi_Init();
  lv_label_set_text(ui->screen_label_16, ".....");
  lv_label_set_text(ui->screen_label_18, ".....");
  for(;xQueueReceive(ble_Queue,mac,3500) == pdTRUE;)
  {
    ESP_LOGI(TAG, "%d",connt);
    connt++;
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  sprintf(lv_buf_set,"%d,T",connt);
  lv_label_set_text(ui->screen_label_16, lv_buf_set);
  connt = 0;
  esp_wifi_ble_setscan(1);
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_start(NULL,true));               //扫描可用AP
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_scan_get_ap_num(&connt));
  vTaskDelay(pdMS_TO_TICKS(5000));
  sprintf(lv_buf_set,"%d,T",connt);
  lv_label_set_text(ui->screen_label_18, lv_buf_set);
  connt = 0;
  espwifi_Deinit();
  vTaskDelete(NULL);
}
void user_app_sd_read(lv_ui *obj)
{
  char sd_values[15] = {0};
  SD_card_Init();
  float sd_value = sd_cadr_get_value();
  if(sd_value)
  {
    sprintf(sd_values,"%.2fG",sd_value);
    lv_label_set_text(obj->screen_label_10, sd_values);
  }
  else
  {
    lv_label_set_text(obj->screen_label_10, "NULL");
  }
}
void example_wsled_port_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  ws2812_Init();
  lv_label_set_text(ui->screen_label_14, "OFF");
  vTaskDelay(pdMS_TO_TICKS(5000));
  for(;;)
  {
    leds_set_pixel(1);
    lv_label_set_text(ui->screen_label_14, "Red");
    vTaskDelay(pdMS_TO_TICKS(2000));
    leds_set_pixel(2);
    lv_label_set_text(ui->screen_label_14, "Green");
    vTaskDelay(pdMS_TO_TICKS(2000));
    leds_set_pixel(3);
    lv_label_set_text(ui->screen_label_14, "Blue");
    vTaskDelay(pdMS_TO_TICKS(2000));
    leds_set_pixel(4);
    lv_label_set_text(ui->screen_label_14, "White");
    vTaskDelay(pdMS_TO_TICKS(2000));
    leds_set_pixel(5);
    lv_label_set_text(ui->screen_label_14, "Yellow");
    vTaskDelay(pdMS_TO_TICKS(2000));
    leds_set_pixel(6);
    lv_label_set_text(ui->screen_label_14, "OFF");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
float acc[3] = {0};
void example_app_port_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  uint32_t stimes = 0;
  uint32_t qmi_test = 0;
  //float acc[3] = {0};
  float gyro[3] = {0};
  uint32_t adc_test = 0;
  float adc_value;
  char qmi_values[45] = {0};
  char adc_values[15] = {0};
  I2C_master_Init();
  qmi8658_init();
  user_app_sd_read(ui);
  adc_bsp_init();
  for(;;)
  {
    if(stimes - qmi_test > 1)   //2s
    {
      qmi_test = stimes;
      qmi8658_read_xyz(acc,gyro);
      sprintf(qmi_values,"(%.2f %.2f %.2f)",acc[0],acc[1],acc[2]);
      lv_label_set_text(ui->screen_label_12, qmi_values);
      //sprintf(qmi_values,"gx:%.2f gy:%.2f gz:%.2f",gyro[0],gyro[1],gyro[2]);
      //lv_label_set_text(obj->screen_label_19, qmi_values);
    }
    if(stimes - adc_test > 1) //2s
    {
      adc_test = stimes;
      adc_get_value(&adc_value,NULL);
      if(adc_value)
      {
        sprintf(adc_values,"%.2fV",adc_value);
        lv_label_set_text(ui->screen_label_11, adc_values);
        //sprintf(adc_values,"%d",(uint16_t)adc_value[1]);
        //lv_label_set_text(obj->screen_label_8, adc_values);  
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    stimes++;
  }
}
void example_lcd_bl_task(void *arg)
{
  for(;;)
  {
    if(acc[1] > 7 && acc[2] < 5)
    {
      setUpduty(0);
    }
    else if (acc[1] < 5 && acc[2] > 7)
    {
      setUpduty(0xff);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
void example_rgb_port_task(void *arg)
{
  lv_ui *ui = (lv_ui *)arg;
  lv_obj_add_flag(ui->screen_cont_2,LV_OBJ_FLAG_HIDDEN);    //隐藏
  lv_obj_clear_flag(ui->screen_cont_1,LV_OBJ_FLAG_HIDDEN);  //显示

  lv_obj_clear_flag(ui->screen_img_1,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_3, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));
  lv_obj_clear_flag(ui->screen_img_2,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_3, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));
  lv_obj_clear_flag(ui->screen_img_3,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_img_2, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(ui->screen_img_1, LV_OBJ_FLAG_HIDDEN);
  vTaskDelay(pdMS_TO_TICKS(1500));

  lv_obj_clear_flag(ui->screen_cont_2,LV_OBJ_FLAG_HIDDEN); //显示
  lv_obj_add_flag(ui->screen_cont_1, LV_OBJ_FLAG_HIDDEN);  
  vTaskDelete(NULL);
}

#endif