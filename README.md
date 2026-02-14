# Oresat Template App

### Overview
OreSat firmware project template using Zephyr RTOS.

This template demonstrates the use of the following hardware modules:
- Blink an LED forever using the GPIO API.
- Periodically read and display temperature, humidity, and barometric pressure using an I2C bus connected to a Bosch Sensortec BME-280 sensor, using the sensor API.
- Generate a continuous 16-bit sawtooth waveform on a DAC output pin using the DAC driver API.
- Periodically monitor two 12-bit ADC channels and display the samples, using the ADC driver API.

*If no BME-280 is connected, the I2C demo will fail. The initial address transaction is still visible on an oscilloscope.*

---
### Supported Boards

Supported boards contain Devicetree overlays in `boards/`. These overlays contain the `oresat_demo` label, which enables the demos in `Kconfig`. If a board does not have an overlay with the label, it will only log the messages found in `src/main.c`. Current supported boards are:
- FRDM-MCXN947
    - SCL:           J7.8
    - SDA:           J7.6
    - DAC:           J1.4
    - ADC channel 0: J4.8
    - ADC channel 1: J4.12
- NUCLEO-R091RC
    - SDA:      Arduino.D14  Morpho.PB9
    - SCL:      Arduino.D15  Morpho.PB8
    - DAC:      Arduino.A2   Morpho.PA4
    - ADC1_IN0: Arduino.A0   Morpho.PA0
    - ADC1_IN1: Arduino.A1   Morpho.PA1
- NUCLEO-U545RE
    - SDA:      Arduino.D14
    - SCL:      Arduino.D15
    - DAC:      Arduino.A2
    - ADC1_IN0: Arduino.A0
    - ADC1_IN1: Arduino.A1

### Building
Ensure you are in the `template` directory (`cd src/oresat/firmware/apps/template`) prior to building.
|               |                                                       |
| ------------- | ----------------------------------------------------- |
| FRDM-MCXN947  | `west build -p always -b frdm_mcxn947/mcxn947/cpu0 .` |
| NUCLEO-R091RC | `west build -p always -b nucleo_f091rc .`             |
| NUCLEO-U545RE | `west build -p always -b nucleo_u545re_q .`           |

### Flashing
Ensure you are in the `template` directory (`cd src/oresat/firmware/apps/template`) prior to flashing.
|               |                         |
| ------------- | ----------------------- |
| OpenOCD       | `west flash -r openocd` |
| PyOCD         | `west flash -r pyocd`   |

**Note**: be sure to install the required pyocd packs for the board's SoC:
`pyocd pack install NXP.MCXN947_DFP@25.09.00`
`pyocd pack install stm32u5`

**Note**: OpenOCD works with the one provided by Zephyr on the STM32F091, but not on the STM32U545. The latter might require you
to install the custom version of OpenOCD developed by ST, which is distributed as part of their STM32CubeIDE. It is unknown for
certain if this is correct. Accordingly, we recommend using pyocd to flash the STM32U545.

---
### Other Information

More information on the application and adding other boards can be found in [Oresat MCXN947 Demo], from which this template originated.

Please follow the installation steps in [Oresat Zephyr Setup].

[Oresat Zephyr Setup]:https://github.com/oresat/oresat-firmware/blob/zephyr/README.md
[Oresat MCXN947 Demo]:https://github.com/plskeggs/oresat-mcxn947-demo/blob/feature-demo-other-hardware/README.md
