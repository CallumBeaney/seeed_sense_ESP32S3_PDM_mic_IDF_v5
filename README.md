# I2S implementation for the Seeed Studio Xiao Sense ESP32S3 model

The Sense ESP32S3 model uses a Pulse Modulation Density microphone, which on some models appears to be a different microphone than what is shown in the tech sheet.  

The documentation for the [Sense ESP32S3](https://github.com/Seeed-Studio/wiki-documents/blob/docusaurus-version/docs/Sensor/SeeedStudio_XIAO/SeeedStudio_XIAO_ESP32S3_Sense/XIAO_ESP32S3_Sense_mic.md) is not the most enlightening, giving only COBOL-esqe Arduinolang sketch that, due to using the Arduino framework wrapper for the ESP-IDF, uses an API wrapped around an old (deprecated) version of the ESP-IDF (v4) and its i2s library.  

The ESP-IDF `v5` uses a new i2s protocol interface that is significantly different to `v4`'s, which integrates PDM connection with the rest of the standard i2s protocols. `v5` has a dedicated `driver/i2s_pdm.h` interface.

In the sketch linked above, the microphone is initialised like so: `I2S.setAllPins(-1, 42, 41, -1, -1);`, with the documentation stating:

>_The function takes five integer parameters that represent the GPIO pins connected to the I2S interface's bit clock, word select, data input, data output, and channel select lines, respectively. In this specific code, the -1 values indicate that the corresponding pins are not used, while the 42 and 41 values represent the GPIO pins connected to the word select and data input lines, respectively. The data output and channel select lines are not used in this configuration and are set to -1._  

In the library backend, the bit clock initialised as `-1` is reassigned to `GPIO 14`. This is a red herring and likely a workaround for the old protocol interface. The Word Select (`GPIO 42`) is actually used as the clock, and the data input stays as the serial data (`GPIO 41`).  

This program demonstrates a basic `v5` process of performing an FFT on mic input samples using the excellent [Mark Borgerding](https://github.com/mborgerding)'s [KISS FFT](https://github.com/mborgerding/kissfft) library. The program expands macros used in the PDM example program, hard-coding the specific required parameters for extra clarity. It additionally provides a very basic "module style" header with simple error handling for using the PDM interface.  