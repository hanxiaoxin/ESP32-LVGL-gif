#include "esp_heap_caps.h"
#include "esp_log.h"
#include "stdint.h"
#include "lvgl.h"

#define HEAP_SIZE 200

static const char *TAG = "UTILS";
static uint8_t *big_buf;

void print_binary(uint8_t *bin_end, uint8_t *bin_start) {
  // 计算嵌入数据的大小
  int logo_bin_size = bin_end - bin_start;

  // 打印起始地址、结束地址和大小
  ESP_LOGI(TAG, "logo.bin start address: %p\n", bin_start);
  ESP_LOGI(TAG, "logo.bin end address  : %p\n", bin_end);
  ESP_LOGI(TAG, "logo.bin size         : %d bytes\n", logo_bin_size);

  // 打印前 64 字节的十六进制内容
  ESP_LOGI(TAG, "First 64 bytes of logo.bin:\n");
  for (int i = 0; i < 64 && i < logo_bin_size; i++) {
    ESP_LOGI(TAG, "%02X ", bin_start[i]);
    if ((i + 1) % 16 == 0) {
      ESP_LOGI(TAG, "\n");
    }
  }
  ESP_LOGI(TAG, "\n");
}

void print_heap(){
  int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
  // heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
  // ESP_LOGI(TAG, "Largest free internal block: %d",
  //          heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
  ESP_LOGI(TAG, "Free internal: %u minimal internal: %u", free_sram,
           min_free_sram);

  size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  ESP_LOGI("MEM", "External PSRAM: %d bytes free", free_psram);
}

void malloc_heap() {
  big_buf = (uint8_t *)heap_caps_malloc(HEAP_SIZE * 1024, MALLOC_CAP_INTERNAL);
  if (big_buf) {
    ESP_LOGW(TAG, "malloc %d KB heap success", HEAP_SIZE);
  } else {
    ESP_LOGE(TAG, "malloc %d KB heap failed", HEAP_SIZE);
  }
}

void free_heap() {
  if (big_buf != nullptr) {
    heap_caps_free(big_buf);
    big_buf = nullptr;
  }
  // print_heap();
}