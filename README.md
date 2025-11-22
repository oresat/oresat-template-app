# Oresat Template App

OreSat firmware project template using Zephyr RTOS.

This template demonstrates the use of the following hardware modules:
- Blink an LED forever using the GPIO API.
- Periodically read and display temperature, humidity, and barometric pressure using an I2C bus connected to a Bosch Sensortec BME-280 sensor, using the sensor API.
- Generate a continuous 16-bit sawtooth waveform on a DAC output pin using the DAC driver API.
- Periodically monitor two 12-bit ADC channels and display the samples, using the ADC driver API.

*If no BME-280 is connected, the I2C demo will fail. The initial address transaction is still visible on an oscilloscope.*

---

Supported boards contain Devicetree overlays in `boards/`. These overlays contain the `oresat_demo` label, which enables the demos in `Kconfig`. If a board does not have an overlay with the label, it will only log the messages found in `src/main.c`. Current supported boards are:
- FRDM-MCXN947
    - SCL: J7.8
    - SDA: J7.6
    - DAC: J1.4
    - ADC channel 0: J4.8
    - ADC channel 1: J4.12

---

More information on building and running the application can be found in [Oresat MCXN947 Demo], from which this template originated.

Please follow the installation steps in [Oresat Zephyr Setup].

[Oresat Zephyr Setup]:https://github.com/oresat/oresat-firmware/blob/zephyr/README.md
[Oresat MCXN947 Demo]:https://github.com/plskeggs/oresat-mcxn947-demo/blob/feature-demo-other-hardware/README.md
