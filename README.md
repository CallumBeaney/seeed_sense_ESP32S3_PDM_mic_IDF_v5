# Basic usage of Seeed Studio Xiao ESP32S3 Sense's PDM microphone for ESP-IDF v5

### What it does
This program performs an FFT on mic input samples using [Mark Borgerding](https://github.com/mborgerding)'s [KISS FFT](https://github.com/mborgerding/kissfft) library. It briefly demonstrates the mic's DC bias. The program expands macros used in the PDM example program provided by Espressif, hard-coding the specific required parameters for extra clarity. The PDM interface is contained within a basic "module style" header with simple error handling.  


### Context
The documentation for the [Sense ESP32S3](https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32S3_Sense/XIAO_ESP32S3_Sense_mic.md) is not the most enlightening, giving only an Arduinolang sketch. Due to using the Arduino framework wrapper for the ESP-IDF, the IDF used is an older version (presently `v4.4.5`), and uses a now-deprecated i2s library.  

Whereas IDF `v5` has a dedicated PDM interface for i2s, `v4` integrated PDM connection with the rest of the standard i2s protocols, like so:
```C++
i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    ...
    ...
};
```

#### Arduino + IDF v4
In the sketch linked above, the microphone is initialised like so: `I2S.setAllPins(-1, 42, 41, -1, -1);`, with the documentation stating:

>_The function takes five integer parameters that represent the GPIO pins connected to the I2S interface's bit clock, word select, data input, data output, and channel select lines, respectively. In this specific code, the -1 values indicate that the corresponding pins are not used, while the 42 and 41 values represent the GPIO pins connected to the word select and data input lines, respectively. The data output and channel select lines are not used in this configuration and are set to -1._   

In the library backend, the bit clock parameter initialised as `-1` is reassigned to `GPIO 14` in the below function called by `setAllPins`, and the word select pin on `GPIO 42` is assigned to the Word Select IO Num parameter:

```C++
void I2SClass::_setSckPin(int sckPin){
  ...
  if(sckPin >= 0){
    _sckPin = sckPin;
  }else{
    _sckPin = PIN_I2S_SCK; // expands to "14"
  }
  ...
}
```

Elsewhere in the setup process, `_sckPin` is passed to the bit clock parameter:

```C++
esp_i2s::i2s_pin_config_t pin_config = {
    .bck_io_num = digitalPinToGPIONumber(_sckPin),
    .ws_io_num = digitalPinToGPIONumber(_fsPin), // word select
    ...
  };
```
   
#### ESP IDF v5 
In IDF `v5`'s dedicated PDM interface, the Word Select (`GPIO 42`) is used for the clock:  
```
i2s_pdm_rx_gpio_config_t gpio_cfg = {
    .clk = wordSelect,
    .din = serialData,
    .invert_flags = {.clk_inv = false,},
  };
```
See `pdm_mic.c` in this repo for more information.
  
