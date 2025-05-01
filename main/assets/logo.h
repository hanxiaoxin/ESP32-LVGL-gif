#include "lvgl.h"
#include "stdint.h"

#define LOGO_WIDTH 128
#define LOGO_HEIGHT 128

extern const uint8_t _binary_logo_bin_start[], _binary_logo_bin_end[];

extern const lv_image_dsc_t logo = {
    .header =
        {
            .magic = LV_IMAGE_HEADER_MAGIC,
            .cf = LV_COLOR_FORMAT_RGB565,
            .flags = 0 | LV_IMAGE_FLAGS_COMPRESSED,
            .w = LOGO_WIDTH,
            .h = LOGO_HEIGHT,
            .stride = LOGO_WIDTH * 2,
            .reserved_2 = 0,
        },
    .data_size = (size_t)(_binary_logo_bin_end - _binary_logo_bin_start),
    .data = _binary_logo_bin_start + 12,
    .reserved = 0,
};