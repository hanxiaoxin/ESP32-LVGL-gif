#include "lvgl.h"
#include "stdint.h"

extern const uint8_t _binary_logo_bin_start[]; // 数据开始
extern const uint8_t _binary_logo_bin_end[];   // 数据结束
extern const uint8_t _binary_logo_bin_size[];  // 数据大小（不是长度值，是地址）

extern const lv_image_dsc_t logo_img = {
    .header =
        {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = LV_COLOR_FORMAT_RGB565,
            .flags = 0,
            .w = 240,
            .h = 280,
            .stride = 480,
            .reserved_2 = 0,
        },
    .data_size = (size_t)(_binary_logo_bin_end - _binary_logo_bin_start),
    .data = _binary_logo_bin_start,
    .reserved = 0,
};