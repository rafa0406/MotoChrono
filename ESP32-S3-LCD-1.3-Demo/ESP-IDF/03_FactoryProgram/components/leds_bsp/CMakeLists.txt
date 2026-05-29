idf_component_register(
  SRCS "leds_bsp.c"
  PRIV_REQUIRES driver espressif__led_strip
  INCLUDE_DIRS "./")
