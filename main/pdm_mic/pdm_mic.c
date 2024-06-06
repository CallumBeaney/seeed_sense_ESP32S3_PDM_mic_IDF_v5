#include "pdm_mic.h"

static const char* TAG = "Mic";
static i2s_chan_handle_t m_receiverHandle = NULL;


esp_err_t mic_init(i2s_port_t port, gpio_num_t serialData, gpio_num_t wordSelect, uint32_t sampleRate)
{
  i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(port, I2S_ROLE_MASTER);

  ESP_ERROR_CHECK(i2s_new_channel(&channelConfig, NULL, &m_receiverHandle));

  i2s_pdm_rx_clk_config_t clock = {
    .sample_rate_hz = sampleRate,
    .clk_src = SOC_MOD_CLK_PLL_F160M, // `I2S_CLK_SRC_DEFAULT` defaults to this
    .mclk_multiple = I2S_MCLK_MULTIPLE_256,
    .dn_sample_mode = I2S_PDM_DSR_8S,
    .bclk_div = 8,
  }; 

  i2s_pdm_rx_slot_config_t slot = {
    .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT, // "only support 16 bits for PDM mode"
    .slot_bit_width = I2S_SLOT_BIT_WIDTH_16BIT, // "only support 16 bits for PDM mode"
    .slot_mode = I2S_SLOT_MODE_MONO,
    /* "The default mono slot is the left slot (whose 'select pin' of the PDM microphone is pulled down)" */
    .slot_mask = I2S_PDM_SLOT_LEFT, 
    /* "I2S PDM only transmits or receives the PDM device whose 'select' pin is pulled down" */
  };

  i2s_pdm_rx_gpio_config_t gpio = {
    .clk = wordSelect, // note that the word select is used as the clock
    .din = serialData,
    .invert_flags = {.clk_inv = false,},
  };

  i2s_pdm_rx_config_t receiverConfig = {
    .clk_cfg = clock,
    .slot_cfg = slot,
    .gpio_cfg = gpio,
  };

  esp_err_t err;
  err = i2s_channel_init_pdm_rx_mode(m_receiverHandle, &receiverConfig);
  if (err != ESP_OK) goto error;
  
  err = i2s_channel_enable(m_receiverHandle);
  if (err != ESP_OK) goto error;

  return ESP_OK;

  error:
    ESP_LOGI(TAG, "Failed to initialise microphone: %s", esp_err_to_name(err));
    return err;
}


esp_err_t mic_read(int16_t* buffer, int bufferLength) 
{   
  esp_err_t err = ESP_OK;

  if (m_receiverHandle == NULL) {
    err = ESP_ERR_INVALID_STATE;
    ESP_LOGE(TAG, "Mic must be initialized before use: %s", esp_err_to_name(err));
    return err;
  }

  if (buffer == NULL) {
    err = ESP_ERR_INVALID_STATE;
    ESP_LOGE(TAG, "Input buffer NULL: %s", esp_err_to_name(err));
    return err;
  }

  size_t bytesRead = 0;
  size_t bytesToRead = (sizeof(int16_t) * bufferLength);
  err = i2s_channel_read(m_receiverHandle, buffer, bytesToRead, &bytesRead, 1000);

  return err;
}


esp_err_t mic_uninstall() 
{
  esp_err_t err = ESP_OK;

  if (m_receiverHandle == NULL) {
    err = ESP_ERR_INVALID_STATE;
    ESP_LOGE(TAG, "Mic must be initialized before use: %s", esp_err_to_name(err));
    return err;
  }

  err = i2s_channel_disable(m_receiverHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error disabling microphone channel: %s", esp_err_to_name(err));
    return err;
  }

  err = i2s_del_channel(m_receiverHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error deleting microphone channel: %s", esp_err_to_name(err));
    return err;
  }

  return err;
}