#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "kiss_fft.h"
#include "kiss_fftr.h"

#include "pdm_mic.h"

float getMaxMagnitude(kiss_fft_cpx* fd, int sr, int ns, int sf);
void printFFTData(kiss_fft_cpx* fd, int sr, int ns, int sf);

const int       NUM_SAMPLES = 2000; 
const uint32_t  SAMPLE_RATE = 16000; // this should not be changed

const i2s_port_t MIC_PORT = I2S_NUM_0;
const gpio_num_t MIC_WS   = GPIO_NUM_42;
const gpio_num_t MIC_SD   = GPIO_NUM_41;

const char* MAIN = "main";
const char* FFT = "FFT output";


void app_main(void)
{
  esp_log_level_set("*", ESP_LOG_VERBOSE);

  ESP_LOGI(MAIN, "Initialising microphone");
  ESP_ERROR_CHECK(mic_init(MIC_PORT, MIC_SD, MIC_WS, SAMPLE_RATE));
  
  ESP_LOGI(MAIN, "Microphone initialised. Capturing %d samples at a %dHz in 2 seconds...", NUM_SAMPLES, (int)SAMPLE_RATE);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  int16_t* samples = calloc(NUM_SAMPLES, sizeof(int16_t)); 
  ESP_ERROR_CHECK(mic_read(samples, NUM_SAMPLES));


  ESP_LOGI(FFT, "Samples captured. Running a Real-component-only FFT on captured samples.");
  kiss_fftr_cfg config = kiss_fftr_alloc(NUM_SAMPLES, false, NULL, NULL);
  kiss_fft_cpx* freqDomain = calloc(NUM_SAMPLES, sizeof(kiss_fft_cpx));

  float* fftInput = calloc(NUM_SAMPLES, sizeof(float)); 
  for (int i = 0; i < NUM_SAMPLES; i++) {
    fftInput[i] = (float)samples[i];
  }
  
  kiss_fftr(config, fftInput, freqDomain); // do FFT
  

  int starting_frequency = 0;
  ESP_LOGI(FFT, "Printing first 100 frequencies, and including the %dHz frequency point in normalisation.", starting_frequency);
  ESP_LOGI(FFT, "Note how DC bias affects the normalised signal at this index.");
  printFFTData(freqDomain, SAMPLE_RATE, NUM_SAMPLES, starting_frequency);
  
  ESP_LOGI(FFT, "Now printing first 100 frequencies, and normalising from %dHz onwards.", starting_frequency);
  starting_frequency = 10;
  printFFTData(freqDomain, SAMPLE_RATE, NUM_SAMPLES, starting_frequency);


  // cleanup
  kiss_fftr_free(config);
  kiss_fftr_free(freqDomain);

  ESP_LOGI(MAIN, "Uninstalling microphone");
  ESP_ERROR_CHECK(mic_uninstall());

  ESP_LOGI(MAIN, "Program completed. Press RST button on ESP32 to restart.");
  return;
}


void printFFTData(kiss_fft_cpx* fd, int sr, int ns, int sf) 
{
  float maxMag = getMaxMagnitude(fd, sr, ns, sf);
    for (int i = 0; i < 100; i++) {
    int frequency = i * SAMPLE_RATE / NUM_SAMPLES;
    float amplitude = fabs(fd[i].r);
    int normalised = (int)(fabs(fd[i].r / maxMag) * 100); // as percentage
    fprintf(stdout, "\t%dHz\tAmp: %.2f\t%%: %d\n", frequency, amplitude, normalised);
  }
  fprintf(stdout, "\n\n");
}


float getMaxMagnitude(kiss_fft_cpx* fd, int sr, int ns, int sf) 
{ 
  float mm = 0;
  for (int i = 0; i < ns; i++) {
    int f = i * sr / ns;
    if (f < sf) continue;
    float mag = fabs(fd[i].r);
    if (mag > mm) mm = mag;
  }
  return mm;
}
