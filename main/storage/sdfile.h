esp_err_t sd_read_file(const char *path);
esp_err_t sd_write_file(const char *path, char *data);
void sd_test_speed();
void sd_init();
void sd_deinit();