#ifndef LEDS_BSP_H
#define LEDS_BSP_H


#define example_out_1  2
#define example_out_2  3
#define example_out_3  4
#define example_out_4  5
#define example_out_5  7
#define example_out_6  8

#define example_in_1   9
#define example_in_2   10
#define example_in_3   11
#define example_in_4   12
#define example_in_5   13
#define example_in_6   14

#define BLINK_GPIO     15
#define lcd_bl         20
void leds_gpio_Init(void);
void GPIO_SET(uint8_t pin,uint8_t mode);
uint8_t GPIO_GET(uint8_t pin);
void ws2812_Init(void);
void leds_set_pixel(uint8_t value);
void pwm_init(void);
void setUpduty(uint16_t duty);
#endif
