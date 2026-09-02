# 📟 PaniScope

<p align="center">
  <img src="demo/header.png" width="100%">
</p>

<p align="center">
  A Mini Digital Oscilloscope & Signal Generator with ESP32
</p>

PaniScope is a custom-built mini oscilloscope based on the ESP32 microcontroller.

The project was created from scratch to explore signal generation, analog-to-digital conversion, waveform processing, and embedded graphical rendering.

Instead of using an external oscilloscope module, this project uses the ESP32's internal DAC and ADC peripherals to generate and capture signals, then visualizes them on a TFT display in real time.


## ✨ Features

- Real-time waveform visualization
- Built-in signal generator
- Sine, square, triangle and pulse waveforms
- Adjustable frequency, amplitude, offset voltage and duty cycle
- Custom oscilloscope-style TFT interface
- ADC signal sampling
- DAC waveform generation
- 

## ## ❓ How It Works

PaniScope consists of three main stages:

Waveform Generation -> Analog Signal Capture -> Real-Time Visualization


### ⭐ 1. Signal Generation (ESP32 DAC)

The ESP32 contains an internal Digital-to-Analog Converter (DAC).


GPIO25 is used as an analog output: GPIO25 → DAC Output


The waveform is generated digitally using a lookup table.

For example, the sine wave is stored as 64 samples:

|  Sample   |  Degree |
| --------- | ------: |
| Sample 0  |  `0°`   |
| Sample 16 |  `90°`  |
| Sample 32 |  `180°` |
| Sample 48 |  `270°` |


During runtime, the samples are continuously sent to the DAC:

```cpp
dacWrite(signalOut, dacValue);
```

The generated digital values become a real analog voltage signal.


Supported waveforms:

* Sine wave
* Square wave
* Triangle wave
* Pulse wave


### ⭐ 1. Signal Sampling (ESP32 ADC)

The generated signal is connected to the ADC input.

ADC pin:
