#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "utils.h"
#include "config.h"

#define MAX_CHAR_SIZE 64

static const char *TAG = "SD-IMAGE";

#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO SD_NUM_MISO
#define PIN_NUM_MOSI SD_NUM_MOSI
#define PIN_NUM_CLK SD_NUM_CLK
#define PIN_NUM_CS SD_NUM_CS

sdmmc_host_t host = SDSPI_HOST_DEFAULT();
sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;
char data[MAX_CHAR_SIZE];
const char *test_data =
    "12345678901234567890123456789012345678901234567890123456789012345678901234"
    "56789012345678901234567890123456789012345678901234567890123456789012345678"
    "90123456789012345678901234567890123456789012345678901234567890123456789012"
    "34567890123456789012345678901234567890123456789012345678901234567890123456"
    "78901234567890123456789012345678901234567890123456789012345678901234567890"
    "12345678901234567890123456789012345678901234567890123456789012345678901234"
    "56789012345678901234567890123456789012345678901234567890123456789012345678"
    "90123456789012345678901234567890123456789012345678901234567890123456789012"
    "34567890123456789012345678901234567890123456789012345678901234567890123456"
    "78901234567890123456789012345678901234567890123456789012345678901234567890"
    "12345678901234567890123456789012345678901234567890123456789012345678901234"
    "56789012345678901234567890123456789012345678901234567890123456789012345678"
    "90123456789012345678901234567890123456789012345678901234567890123456789012"
    "34567890123456789012345678901234567890";

esp_err_t sd_write_file(const char *path, char *data) {
  char file_target[256];
  snprintf(file_target, sizeof(file_target), MOUNT_POINT "/%s", path);
  ESP_LOGI(TAG, "Opening file %s", path);
  FILE *f = fopen(path, "w");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return ESP_FAIL;
  }
  fprintf(f, data);
  fclose(f);
  ESP_LOGI(TAG, "File written");

  return ESP_OK;
}

esp_err_t sd_read_file(const char *path) {
  char file_target[256];
  snprintf(file_target, sizeof(file_target), MOUNT_POINT "/%s", path);

  // print_heap();
  ESP_LOGI(TAG, "Reading file %s", path);
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return ESP_FAIL;
  }
  char line[MAX_CHAR_SIZE];
  fgets(line, sizeof(line), f);
  fclose(f);

  // strip newline
  char *pos = strchr(line, '\n');
  if (pos) {
    *pos = '\0';
  }
  ESP_LOGI(TAG, "Read from file: '%s'", line);

  return ESP_OK;
}

void sd_test_speed() {
  const char *file_test = MOUNT_POINT "/test.txt";

  struct stat st;
  if (stat(file_test, &st) == 0) {
    // Delete it if it exists
    unlink(file_test);
  }

  TickType_t startTick = xTaskGetTickCount();

  FILE *f = fopen(file_test, "w");
  if (f == NULL) {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return;
  }

  for (int i = 0; i < 1000; i++) {
    fprintf(f, test_data);
  }

  TickType_t endTick = xTaskGetTickCount();

  fclose(f);

  ESP_LOGI(TAG, "Benchmark writing 1MB in %lu milliseconds (%.2fMB/s)",
           endTick - startTick, 1000. / (endTick - startTick));

  // read
  esp_err_t ret = sd_read_file(file_test);
  if (ret != ESP_OK) {
    return;
  }
}

void sd_init() {
  esp_err_t ret;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 4,
      .allocation_unit_size = 16 * 1024};
 
  ESP_LOGI(TAG, "Initializing SD card");
  ESP_LOGI(TAG, "Using SPI peripheral");

  
  host.max_freq_khz = 20000;

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = PIN_NUM_MOSI,
      .miso_io_num = PIN_NUM_MISO,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 5000,
  };

  ret = spi_bus_initialize((spi_host_device_t)(host.slot), &bus_cfg,
                           SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize bus.");
    return;
  }

  // This initializes the slot without card detect (CD) and write protect (WP)
  // signals. Modify slot_config.gpio_cd and slot_config.gpio_wp if your board
  // has these signals.
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = PIN_NUM_CS;
  slot_config.host_id = (spi_host_device_t)host.slot;

  ESP_LOGI(TAG, "Mounting filesystem");
  ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config,
                                &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set the "
                    "CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    } else {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%s). "
               "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
    }
    return;
  }
  ESP_LOGI(TAG, "Filesystem mounted");

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);
}

void sd_deinit() {
  // All done, unmount partition and disable SPI peripheral
  esp_vfs_fat_sdcard_unmount(mount_point, card);
  ESP_LOGI(TAG, "Card unmounted");

  // deinitialize the bus after all devices are removed
  spi_bus_free((spi_host_device_t)host.slot);
}