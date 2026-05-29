#include <stdio.h>
#include "leds_bsp.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "led_strip_rmt.h"
#include "led_strip.h"
#include "driver/ledc.h"
#define bit_mask (uint64_t)0x01

void leds_gpio_Init(void)
{
  gpio_config_t gpio_conf = {};
  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_OUTPUT;
  gpio_conf.pin_bit_mask = (bit_mask<<example_out_1) \
  | (bit_mask<<example_out_2) \
  | (bit_mask<<example_out_3) \
  | (bit_mask<<example_out_4) \
  | (bit_mask<<example_out_5) \
  | (bit_mask<<example_out_6) \
  ;
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf)); //ESP32板载GPIO

  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_INPUT;
  gpio_conf.pin_bit_mask = (bit_mask<<example_in_1) \
  | (bit_mask<<example_in_2) \
  | (bit_mask<<example_in_3) \
  | (bit_mask<<example_in_4) \
  | (bit_mask<<example_in_5) \
  | (bit_mask<<example_in_6) \
  ;
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));
}
led_strip_handle_t led_strip;
void ws2812_Init(void)
{
  led_strip_config_t strip_config = 
  {
    .strip_gpio_num = BLINK_GPIO,  // The GPIO that connected to the LED strip's data line
    .max_leds = 1,                 // The number of LEDs in the strip,
    .led_model = LED_MODEL_WS2812, // LED strip model, it determines the bit timing
    .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // The color component format is G-R-B
    .flags = 
    {
      .invert_out = false, // don't invert the output signal
    }
  };

  led_strip_rmt_config_t rmt_config = 
  {
    .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
    .resolution_hz = 10 * 1000 * 1000, // RMT counter clock frequency: 10MHz
    .mem_block_symbols = 64,           // the memory size of each RMT channel, in words (4 bytes)  (64 * 4 / 3)
    .flags = 
    {
      .with_dma = true, // DMA feature is available on chips like ESP32-S3/P4
    }
  };

  ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}
void leds_set_pixel(uint8_t value)
{
  switch (value)
  {
    case 1:
      led_strip_set_pixel(led_strip,0,255,0,0); //red  
      break;
    case 2:
      led_strip_set_pixel(led_strip,0,0,255,0); //green
      break;
    case 3:
      led_strip_set_pixel(led_strip,0,0,0,255); //blue
      break;
    case 4:
      led_strip_set_pixel(led_strip,0,255,255,255); //white
      break;
    case 5:
      led_strip_set_pixel(led_strip,0,255,255,0); //yellow
      break;
    case 6:
      led_strip_set_pixel(led_strip,0,0,0,0);    // off
      break;
    default:
      break;
  }
  led_strip_refresh(led_strip);
}
void GPIO_SET(uint8_t pin,uint8_t mode)
{
  gpio_set_level(pin,mode);
}
uint8_t GPIO_GET(uint8_t pin)
{
  return gpio_get_level(pin);
}
/*PWM*/
void pwm_init(void)
{
  ledc_timer_config_t timer_conf = 
  {
    .speed_mode =  LEDC_LOW_SPEED_MODE,
    .duty_resolution = LEDC_TIMER_8_BIT,
    .timer_num =  LEDC_TIMER_3,
    .freq_hz = 30 * 1000,
    .clk_cfg = LEDC_SLOW_CLK_RC_FAST,
  };
  ledc_channel_config_t ledc_conf = 
  {
    .gpio_num = lcd_bl,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel =  LEDC_CHANNEL_1,
    .intr_type =  LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_3,
    .duty = 0x7f,   //占空比
    .hpoint = 0,    //相位
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_timer_config(&timer_conf));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&ledc_conf));
}
void setUpduty(uint16_t duty)
{
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty));
  ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}