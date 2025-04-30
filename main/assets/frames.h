#include "lvgl.h"
#include "stdint.h"

#define FRAME_WIDTH 240
#define FRAME_HEIGHT 280
#define FRAME_STRIDE (FRAME_WIDTH * 2) // RGB565 每像素2字节

#define FRAME_DSC(index)                                                       \
  {                                                                            \
      .header =                                                                \
          {                                                                    \
              .magic = LV_IMAGE_HEADER_MAGIC,                                  \
              .cf = LV_COLOR_FORMAT_RGB565,                                    \
              .flags = 0,                                                      \
              .w = FRAME_WIDTH,                                                \
              .h = FRAME_HEIGHT,                                               \
              .stride = FRAME_STRIDE,                                          \
              .reserved_2 = 0,                                                 \
          },                                                                   \
      .data_size = (size_t)(_binary_frame_##index##_bin_end -                  \
                            _binary_frame_##index##_bin_start),                \
      .data = _binary_frame_##index##_bin_start,                               \
      .reserved = 0,                                                           \
  }

extern const uint8_t _binary_frame_00_bin_start[], _binary_frame_00_bin_end[];
extern const uint8_t _binary_frame_01_bin_start[], _binary_frame_01_bin_end[];
extern const uint8_t _binary_frame_02_bin_start[], _binary_frame_02_bin_end[];
extern const uint8_t _binary_frame_03_bin_start[], _binary_frame_03_bin_end[];
extern const uint8_t _binary_frame_04_bin_start[], _binary_frame_04_bin_end[];
extern const uint8_t _binary_frame_05_bin_start[], _binary_frame_05_bin_end[];
extern const uint8_t _binary_frame_06_bin_start[], _binary_frame_06_bin_end[];
extern const uint8_t _binary_frame_07_bin_start[], _binary_frame_07_bin_end[];
extern const uint8_t _binary_frame_08_bin_start[], _binary_frame_08_bin_end[];
extern const uint8_t _binary_frame_09_bin_start[], _binary_frame_09_bin_end[];
extern const uint8_t _binary_frame_10_bin_start[], _binary_frame_10_bin_end[];
extern const uint8_t _binary_frame_11_bin_start[], _binary_frame_11_bin_end[];
extern const uint8_t _binary_frame_12_bin_start[], _binary_frame_12_bin_end[];

// frame 描述数组
const lv_image_dsc_t frames[] = {
    FRAME_DSC(00), FRAME_DSC(01), FRAME_DSC(02), FRAME_DSC(03), FRAME_DSC(04),
    FRAME_DSC(05), FRAME_DSC(06), FRAME_DSC(07), FRAME_DSC(08), FRAME_DSC(09),
    // FRAME_DSC(10), FRAME_DSC(11), FRAME_DSC(12),
};
const int frames_count = sizeof(frames) / sizeof(lv_image_dsc_t);