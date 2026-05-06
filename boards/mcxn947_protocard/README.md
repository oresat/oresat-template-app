# Oresat MCXN Proto Card

This repository contains the board files for the Oresat MCXN947 protocard.

They currently enable flexcomm4 UART for the console, I2C0, DAC0, ADC0 channels 1A and 2A, and a bunch of test points as GPIOs.

Additionally, other hardware modules in the MCXN947 and where they are routed to test points are also documented below. Not
all of them are used in the demos however.

## Available devices and/or test points

### On board LED
- Amber LED D4 = PIO1_13

### I2C0
- TP76 = SDA = PIO0_16
- TP77 = SCL = PIO0_17

### SPI
- TP46 = SDO = PIO4_12
- TP45 = SCK = PIO4_13
- TP43 = SDI = PIO4_16
- TP42 = PCS = PIO4_17

### UART (logging output and console input)
- TP3  = RXD = PIO1_8
- TP13 = TXD = PIO1_9

### CAN 0
- JP2 pin 12 = CAN2_H = CAN0_TXD = PIO1_10
- JP2 pin 11 = CAN2_L = CAN0_RXD = PIO1_11

> NOTE: CAN2_H and CAN2_L also connect to the Main Oresat Card Connector. These
> signals connect to CAN0_TXD and CAN0_RXD through U8, the CAN Transceiver.

### QSPI
- TP53 = SS0   = PIO3_0
- TP50 = SCLK  = PIO3_7
- TP51 = DQS   = PIO3_6
- TP49 = DATA0 = PIO3_8
- TP48 = DATA1 = PIO3_9
- TP47 = DATA2 = PIO3_10
- TP61 = DATA3 = PIO3_11

### DAC 0
- TP28 = DAC0 = PIO4_2

### PWM instance 1, 6 channels
- TP18 = PWM1_CHA0 = PIO2_6
- TP19 = PWM1_CHB0 = PIO2_7
- TP14 = PWM1 CHA1 = PIO2_4
- TP17 = PWM1 CHB1 = PIO2_5
- TP11 = PWM1_CHA2 = PIO2_2
- TP12 = PWM1_CHB2 = PIO2_3

### Comparator 0

- TP68 = PIO1_0 = CMP0_IN0P
- TP71 = PIO1_3 = CMP0_IN1N

> NOTE: This connects an MCXN947 analog comparator to one input on TP68. The other input would need
> to be configured in software to connect to the comparator's dedicated DAC, which you can use to set
> an analog level to compare with TP68.
>
> Alternatively, you can use TP71 = PIO1_3 = CMP0_IN1N for a second input that TP68 can be compared to,
> but it is currently set up as a GPIO.
>
> You would need to remove that from line 61 in `src/gpio.c`:\
>    `X(71, 1,  3) \`\
> as well as line 80 in `mcxn947_protocard_cmxn947_cpu0.dtsi`:\
>    `tp71-p1-3-gpios = <&gpio1 3 GPIO_PULL_DOWN>;`\
> and then add it to the phandle array for `pinmux_lpcmp0` in `mcxn947_protocard-pinctrl.dtsi`:
```
	  pinmux_lpcmp0: pinmux_lpcmp0 {
		  group0 {
			  pinmux = <CMP0_IN0_PIO1_0>, /* change ; to , */
				  <CMP0_IN1_PIO1_3>;      /* add new line */
```
> The output can be monitored in software or can trigger an event like an interrupt or a DMA transfer.

### Timer 0
- TP11 = SCT0_OUT0 = PIO2_2

### ADC 0
- TP30 = PIO4_0  = ADC0_CH0 (single ended) = MAX4211 Iout measurement = voltage / sense resistor (0.1 ohms)
- TP44 = PIO4_15 = ADC0_CH1 (single endede)
- (internal) ADC0_CH26A and ADC0_CH26B (differential) = Internal temperature; use equation 17 in
section 41.3.7 in the MCX Nx4x Reference Manual to compute the temperature from the two FIFO values.

### GPIOs
- TP84 = PIO0_3
- TP85 = PIO0_4
- TP86 = PIO0_5
- TP78 = PIO0_18
- TP79 = PIO0_19
- TP80 = PIO0_20
- TP81 = PIO0_21
- TP82 = PIO0_22
- TP83 = PIO0_23
- TP68 = PIO1_0
- TP69 = PIO1_1
- TP70 = PIO1_2
- TP71 = PIO1_3
- TP72 = PIO1_4
- TP73 = PIO1_5
- TP74 = PIO1_6
- TP75 = PIO1_7
- TP5  = PIO1_12
- TP7  = PIO1_14
- TP8  = PIO1_15
- TP9  = PIO2_0
- TP10 = PIO2_1
- TP52 = PIO3_1
- TP60 = PIO3_12
- TP59 = PIO3_13
- TP58 = PIO3_14
- TP57 = PIO3_15
- TP56 = PIO3_16
- TP55 = PIO3_17
- TP62 = PIO3_20
- TP63 = PIO3_21
- TP29 = PIO4_1
- TP27 = PIO4_3
- TP26 = PIO4_4
- TP25 = PIO4_5
- TP22 = PIO4_6
- TP20 = PIO4_7
- TP41 = PIO4_19
- TP64 = PIO5_0
- TP65 = PIO5_1
- TP66 = PIO5_2
- TP67 = PIO5_3

> NOTE: See mcxn947_protocard_mcxn947_cpu0.dtsi for device tree node names for above, and src/gpio.c in apps/template for a demonstration
> of how to access these in C.
