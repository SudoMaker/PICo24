# Board: PotatoPi PICo24

## Note
Feel free to open an issue if you have any questions.

## PinOut

- CNxx: Change notification is supported (share one interrupt)
- INTxx: Pins with a dedicated change interrupt
- RPxx: Function remapping is supported
- ANxx: Analog function is supported
  
If you don't understand the terms above, please read [this doc](https://ww1.microchip.com/downloads/en/DeviceDoc/39711b.pdf).

- All MCU pins can be used as ordinary GPIOs

### 40 pin RPI header

- [RPI]: Raspberry Pi's default function

| [RPI] Function | MCU Pin | Pin | Pin |MCU Pin | [RPI] Function |
| :-------- | :--------: | :--------: | :--------: | :--------: | --------: |
| 3V3 | - | 1 | 2 | - | 5V |
| [I2C1_SDA]/RP4/CN54 | RD9 | 3 | 4 | - | 5V |
| [I2C1_SCL]/RP3/CN55 | RD10 | 5 | 6 | - | GND |
| CN65 | RE7 | 7 | 8 | RG7 | [UART1_TX]/RP26/CN9 |
| GND | - | 9 | 10 | RG6 | [UART1_RX]/RP21/CN8 |
| CN64 | RE6 | 11 | 12 | RE5 | CN63 |
| CN62 | RE4 | 13 | 14 | - | GND |
| CN60 | RE2 | 15 | 16 | RE3 | CN61 |
| 3V3 | - | 17 | 18 | RE1 | CN59 |
| [SPI1_MOSI]/RP20/CN14 | RD5 | 19 | 20 | - | GND |
| [SPI1_MISO]/RP25/CN13 | RD4 | 21 | 22 | RE0 | CN58 |
| [SPI1_SCLK]/RP22/CN52 | RD3 | 23 | 24 | RD2 | [SPI1_CS]/RP23/CN51 |
| GND | - | 25 | 26 | RD1 | RP24/CN50 |
| RP9/CN27 | RB9 | 27 | 28 | RB8 | RP8/CN26 |
| AN10/CN28 | RB10 | 29 | 30 | - | GND |
| AN11/CN29 | RB11 | 31 | 32 | RB12 | AN12/CN30 |
| AN13/CN31 | RB13 | 33 | 34 | - | GND |
| [SPI2_MISO]/RP2/CN53 | RD8 | 35 | 36 | RB14 | AN14/RP14/CN32 |
| AN15/RP29/CN12 | RB15 | 37 | 38 | RB7 | [SPI2_MOSI]/AN7/RP7/CN25 |
| GND | - | 39 | 40 | RB6 | [SPI2_SCLK]/AN6/RP6/CN24 |

### Standard ICSP connector
Works with Microchip's hardware tools, such as the PICKit 3.

| Pin | Description | MCU Pin |
| -------- | -------- | -------- |
| 1 | MCLR# | MCLR# |
| 2 | 3V3 | - |
| 3 | GND | - |
| 4 | PGD | RB4 |
| 5 | PGC | RB5 |
| 6 | NC | - |

### SOP-8 slot
Compatible with W25QXX-like SPI Flash/SRAM/PSRAM/FRAM/etc.

Should be connected to SPI3.

| Pin | Description | MCU Pin | MCU Function |
| -------- | -------- | -------- | -------- |
| 1 | SPI CS# | **RG8** | **RP19/CN10** |
| 2 | SPI MISO | RB2 | RP13/CN4 |
| 3 | 3V3 | - | - |
| 4 | GND | - | - |
| 5 | SPI MOSI | RB1 | RP1/CN3 |
| 6 | SPI SCLK | RB0 | RP0/CN2 |
| 7 | 3V3 | - | - |
| 8 | 3V3 | - | - |

### SOIC-8 slot
Compatible with W25QXX-like SPI Flash/SRAM/PSRAM/FRAM/etc.

Should be connected to SPI3.

| Pin | Description | MCU Pin | MCU Function |
| -------- | -------- | -------- | -------- |
| 1 | SPI CS# | **RG9** | **RP27/CN11** |
| 2 | SPI MISO | RB2 | RP13/CN4 |
| 3 | 3V3 | - | - |
| 4 | GND | - | - |
| 5 | SPI MOSI | RB1 | RP1/CN3 |
| 6 | SPI SCLK | RB0 | RP0/CN2 |
| 7 | 3V3 | - | - |
| 8 | 3V3 | - | - |

### SOT-23-5 slot
Compatible with AT24-like I2C EEPROM. 

Connected to I2C2.

| Pin | Description | MCU Pin | MCU Function |
| -------- | -------- | -------- | -------- |
| 1 | I2C SCL | RF5 | RP17/CN18 |
| 2 | GND | - | - |
| 3 | I2C SDA | RF4 | RP10/CN17 |
| 4 | 3V3 | - | - |
| 5 | GND | - | - |

### Onboard EPSON RX8900CE RTC

Connected to I2C2.

| Pin | Description | MCU Pin | MCU Function |
| -------- | -------- | -------- | -------- |
| 1 | FOE | - | - |
| 2 | 3V3 | - | - |
| 3 | VBAT | - | - |
| 4 | NC | - | - |
| 5 | I2C SCL | RF5 | RP17/CN18 |
| 6 | NC | - | - |
| 7 | I2C SDA | RF4 | RP10/CN17 |
| 8 | NC | - | - |
| 9 | GND | - | - |
| 10 | INT# | RD0 | INT0/CN49 |

### Onboard OnSemi FUSB301 Type-C Controller

Connected to I2C2.

| Pin | Description | MCU Pin | MCU Function |
| -------- | -------- | -------- | -------- |
| 1 | I2C SDA | RF4 | RP10/CN17 |
| 2 | I2C SCL | RF5 | RP17/CN18 |
| 3 | USB ID | RF3 | USBID/RP16/CN71 |
| 4 | GND | - | - |
| 5 | 3V3 | - | - |
| 6 | CC1 | - | - |
| 7 | CC2 | - | - |
| 8 | VBUS | - | - |
| 9 | NC | - | - |
| 10 | NC | - | - |
