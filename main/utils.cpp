#include "stdint.h"

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