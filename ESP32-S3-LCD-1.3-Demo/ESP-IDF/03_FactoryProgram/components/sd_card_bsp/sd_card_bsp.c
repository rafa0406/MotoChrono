#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "sd_card_bsp.h"
#include "driver/sdmmc_host.h"

#define USER_SDMMC 1
#define USER_SPI   0

#define SD_Read_Mode USER_SDMMC

#if (SD_Read_Mode == USER_SPI)
#define SD_SPI SPI3_HOST
#define PIN_NUM_MISO  (gpio_num_t)16
#define PIN_NUM_MOSI  (gpio_num_t)18
#define PIN_NUM_CLK   (gpio_num_t)21
#define PIN_NUM_CS    (gpio_num_t)17
#else
#define SDMMC_CMD  (gpio_num_t)18
#define SDMMC_D0   (gpio_num_t)16
#define SDMMC_CLK  (gpio_num_t)21
#endif

#define EXAMPLE_MAX_CHAR_SIZE    64  //Maximum size of data to read
#define SDlist "/sd_card"            //Directory, similar to a standard path

esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);

sdmmc_card_t *card = NULL;

void SD_card_Init(void)
{
#if (SD_Read_Mode == USER_SPI)
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,  // Maximum number of open files
    .allocation_unit_size = 512, // Similar to sector size
  };
  spi_bus_config_t bus_cfg = {
    .mosi_io_num = PIN_NUM_MOSI,
    .miso_io_num = PIN_NUM_MISO,
    .sclk_io_num = PIN_NUM_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 4000,   // Maximum transfer size   
  };
  ESP_ERROR_CHECK_WITHOUT_ABORT(spi_bus_initialize(SD_SPI, &bus_cfg, SDSPI_DEFAULT_DMA));

  //sdspi_dev_handle_t dev_handle;
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = (gpio_num_t)PIN_NUM_CS;
  slot_config.host_id = SD_SPI;
  //sdspi_host_init_device(&slot_config,&dev_handle);
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = SD_SPI;
  
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_vfs_fat_sdspi_mount(SDlist, &host, &slot_config, &mount_config, &card)); // Mount the SD card
  if(card != NULL)
  {
    sdmmc_card_print_info(stdout, card); // Print card information
    printf("practical_size:%.2f\n",(float)(card->csd.capacity)/2048/1024);// Size in GB
  }
  else
  {
    printf("Failed to open the SD card.\n");
  }
#else
  esp_vfs_fat_sdmmc_mount_config_t mount_config = 
  {
    .format_if_mount_failed = false,     // Format SD card if mounting fails true
    .max_files = 5,                     // Maximum number of open files
    .allocation_unit_size = 512,        // Allocation unit size (similar to sector size)
    //.disk_status_check_enable = 1,
  };
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  //host.max_freq_khz = SDMMC_FREQ_HIGHSPEED; // High-speed mode

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = 1;           // 1-wire mode
  slot_config.clk = SDMMC_CLK;
  slot_config.cmd = SDMMC_CMD;
  slot_config.d0 = SDMMC_D0;
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_vfs_fat_sdmmc_mount(SDlist, &host, &slot_config, &mount_config, &card));

  if(card != NULL)
  {
    sdmmc_card_print_info(stdout, card); // Print card information
    printf("practical_size:%.2f\n",(float)(card->csd.capacity) / 2048 / 1024); // Size in GB
  }
  else
  {
    printf("Failed to open the SD card.\n");
  }
#endif
}
/*read*/
float sd_cadr_get_value(void)
{
  if(card != NULL)
  {
    return (float)(card->csd.capacity)/2048/1024; //G
  }
  else
  return 0;
}

/* Write data
path: file path
data: data to write
*/
esp_err_t s_example_write_file(const char *path, char *data)
{
  esp_err_t err;
  if(card == NULL)
  {
    return ESP_ERR_NOT_FOUND;
  }
  err = sdmmc_get_status(card); // Check if the SD card is present
  if(err != ESP_OK)
  {
    return err;
  }
  FILE *f = fopen(path, "w"); // Open the file path
  if(f == NULL)
  {
    printf("path:Write Wrong path\n");
    return ESP_ERR_NOT_FOUND;
  }
  fprintf(f, data); // Write data to file
  fclose(f);
  return ESP_OK;
}
/*
Read data
path: file path
*/
esp_err_t s_example_read_file(const char *path)
{
  esp_err_t err;
  if(card == NULL)
  {
    return ESP_ERR_NOT_FOUND;
  }
  err = sdmmc_get_status(card); // Check if the SD card is present
  if(err != ESP_OK)
  {
    return err;
  }
  FILE *f = fopen(path, "r");
  if (f == NULL)
  {
    printf("path:Read Wrong path\n");
  }
  char line[EXAMPLE_MAX_CHAR_SIZE];
  fgets(line, sizeof(line), f); // Read data
  fclose(f);
  char *pos = strchr(line, '\n'); // Find newline character
  if (pos) 
  {
    *pos = '\0';
  }
  return ESP_OK;
}
/*
struct stat st;
stat(file_foo, &st);// Get file information. Returns 0 on success. file_foo is a string, and the file name requires an extension. Used to check if the file exists.
unlink(file_foo);// Delete file. Returns 0 on success.
rename(char*,char*);// Rename file.
esp_vfs_fat_sdcard_format();// Format the SD card.
esp_vfs_fat_sdcard_unmount(mount_point, card);// Unmount the VFS.
FILE *fopen(const char *filename, const char *mode);
"w": Open file in write mode. If the file exists, its content is cleared; if it doesn't exist, a new file is created.
"a": Open file in append mode. If the file doesn't exist, a new file is created.
mkdir(dirname, mode); Create a directory.

To read data other than text files, use "rb" mode. This mode is for opening files in read-only binary mode, suitable for images and other binary files.
Using "r" mode will open the file in text mode, which may cause errors or corruption when reading binary files.
For image files (e.g., JPEG, PNG), always use "rb" mode to correctly read the file content.

To get file size, use the following functions:
  fseek(file, 0, SEEK_END): Move the file pointer to the end of the file.
  ftell(file): Returns the current file pointer position, which equals the file size in bytes.
*/
