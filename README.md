# PICo24

Ultimate software platform for Microchip PIC24/dsPIC33 MCUs.

## Features

### CMake support
- You can use your favorite IDE or editor, as long as it supports CMake
- Tested on CLion and Sublime Text (VSCode users please see [this issue](https://github.com/microsoft/vscode-cpptools/issues/7534))

### The [PICoBoot](https://github.com/SudoMaker/PICoBoot) USB bootloader
- Updates firmware directly via USB
- No driver installation needed on modern host OS (Linux/macOS/Windows)
- `fastboot` like command line tool
- Non-volatile environment variable (like U-Boot)
- Preinstalled on every PotatoPi PICo24

### Easy to use peripheral libraries
- Work like STM32 HAL APIs
- Macro based pin configuration
- Supports transferring data in [EDS memory regions](https://microchipdeveloper.com/16bit:extending-data-memory-on-a-16-bit-pic-mcu) directly to peripherals

### USB Device features
- Supported functions: CDC ACM/ECM/NCM, Mass Storage, Audio, HID and MIDI
- Multiple functions and multiple function instances at the same time (e.g. 4xACM+1xStorage)

### USB Host features
- HUB support
- Supported functions: CDC ACM, Mass Storage, Audio, HID and UVC

### libc extensions
- malloc can track memory usages, and also allocate in EDS memory regions

### Unix API emulation
- Emulation of Unix APIs such as read/write/sleep/usleep/dprintf
- They can be used to control data flows on UART and USB CDC, and switch RTOS task automatically when I/O is blocked

### Integrated 3rdparty software libraries
- FreeRTOS 9.0.0
- LwIP 2.1.1
- Lua 5.1.5

## Current status
Due to shortage of raw materials of PotatoPi Lite, PotatoPi PICo24 had to be released prematurely to fill the gap. So, not all software features mentioned above are implemented at this moment.

| Feature | Status |
| -------- | -------- |
| PinMux | OK |
| Timer | OK |
| UART | OK, CTS/RTS untested |
| I2C Master | OK, API not finalized |
| I2C Slave | Untested |
| SPI Master | OK |
| SPI Slave | Untested |
| External interrupts | Untested |
| malloc stats | OK |
| EDS malloc | OK |
| EDS string.h ops | OK |
| Unix APIs | OK, currently requires FreeRTOS |
| USB role switch | OK |
| USB Device: HID | OK |
| USB Device: MSD | OK |
| USB Device: MIDI | OK |
| USB Device: Audio | Mostly, currently TX only |
| USB Device: CDC ACM | OK |
| USB Device: CDC ECM | Mostly |
| USB Device: CDC NCM | Mostly, only works with Linux |
| USB Host: HUB | WIP: unstable change notification |
| USB Host: HID | WIP: 1 device only |
| USB Host: CDC ACM | WIP: 1 device only |
| USB Host: MSD | TODO |
| USB Host: Audio | TODO |
| USB Host: UVC | TODO |
| FreeRTOS | OK, well tested |
| LwIP | OK, tested PPPoS, ping and HTTP srv|
| Lua | OK, tested luaL_dostring()|