#define USER_SETUP_INFO "Waveshare_S3_1_3"

// --- PILOTE ET RESOLUTION ---
#define ST7789_DRIVER     
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
#define CGRAM_OFFSET
#define TFT_RGB_ORDER TFT_BGR 

// --- BROCHES HARDWARE DE LA CARTE WAVESHARE ---
#define TFT_MOSI 41
#define TFT_SCLK 40
#define TFT_CS   39
#define TFT_DC   38
#define TFT_RST  42

// --- LA SOLUTION ANTI-CRASH ESP32-S3 (CORE V3) ---
#define USE_HSPI_PORT
#define TFT_MISO -1 

// --- POLICES D'ÉCRITURE ---
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4

// --- VITESSE DU BUS ---
#define SPI_FREQUENCY  40000000