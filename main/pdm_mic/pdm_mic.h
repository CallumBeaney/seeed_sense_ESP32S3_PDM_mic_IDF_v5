#ifndef PDM_MIC_H
#define PDM_MIC_H

#include "driver/i2s_pdm.h"
#include "driver/i2s_types.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_log.h"

esp_err_t mic_init(i2s_port_t portPin, gpio_num_t serialDataPin, gpio_num_t wordSelectPin, uint32_t sampleRate);
esp_err_t mic_read(int16_t* buffer, int numberOfSamples);
esp_err_t mic_uninstall(void);

#endif