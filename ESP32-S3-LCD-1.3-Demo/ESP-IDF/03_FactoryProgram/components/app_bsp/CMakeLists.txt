idf_component_register(
  SRCS "app_bsp.c"
  PRIV_REQUIRES leds_bsp ui_bsp qmi8658 i2c_bsp sd_card_bsp adc_bsp ble_scan_bsp esp_wifi_bsp
  INCLUDE_DIRS "./")
