# Source code

- Written in C, using CMSIS and direct registers access  
- IDE: STM32CubeIDE Version: 1.17.0 


# HEX Compilation

1. Open project in STM32CubeIDE
1. Open and edit "config.h"
1. Select via defines **Spoke HW revision** you have, **baud speed of the GPS module** you use, and **frequency band of the LoRa module** you populated
1. Click Build-All (Ctrl+B) for any configuration, either "Debug" or "Release" - they are configured to be equivalent
1. Find "Spoke.hex" in "Debug" or "Release" folder depending on the configuration you compiled for


# Programming

## Using SWD (ST-Link) - guaranteed

1. Run STM32CubeIDE
1. Select ST-LINK programming mode
1. Connect your programmer to SWD interface lines available on all Spoke HW versions
1. Connect at least SWDCLK (marked CL), SWDIO (marked IO) and GND (marked G)
1. With battery connected to Spoke, push down the power button and **hold it down until programming is done**
1. Erase all and flash the HEX file
1. Once flashing is done you should hear sartup beep and LCD showing loading screen
1. Release power button and disconnect from the programmer

## Using UART (STM32 bootloader) - only Spoke HW Rev.2.x

1. Run STM32CubeIDE
1. Select UART programming mode
1. Depending on Spoke PCB variant you either use external USB-UART module connected to console pins or Spoke built-in USB-UART converter if populated
1. In first case connect your external UART module to UART lines (Spoke TX line is marked as TX; Spoke RX line is marked as RX; do not forget to cross lines towards external UART module) or 
1. Connect external UART module to USB or connect Spoke USB-C cable to computer 
1. Click connect in STM32CubeIDE to appropriate COM port (you may need driver for Spoke USB-UART; install "CP210x USB to UART Bridge VCP Drivers" from Silabs website) 
1. When successfully connected to the COM port, enter STM32 bootloader mode by holding down power button when boot button is pressed. **Keep holding power button down untill programming is done**
1. There is alternative way to enter STM32 bootloader if Spoke FW is already flashed, see next section
1. STM32CubeIDE should report on successfult bootloader connect with bootloader version and MCU type. You may release boot button then
1. Erase all and flash the HEX file
1. Release power button and disconnect from the programmer
1. Power up Spoke to make sure it is working

## Using UART (STM32 bootloader) - alternative way any Spoke HW

1. There is a way to use STM32 bootloader in any Spoke HW revision if initial programming was done and Spoke FW is flashed
1. Holding OK button upon power up switches to STM32 bootloader through Spoke FW, so you do not need boot button as required in the above section which is only present in Spoke HW Rev.2.x
1. This makes it possible to use STM32 bootloader even in Spoke HW 1.x for FW updates

# Default settings

- Any FW update is recommended to be done with full MCU flash erase to rewrite settings area in memory
- In some cases settings area is not overwrittent (for example when you are flashing directly from CubeIDE)
- When any Spoke FW is already flashed you can reset setting to default by holding ESC button upon power up. You will be prompted to reset them interactive way