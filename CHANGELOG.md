# Release HW Rev.2.1 17-Jan-2026
Major improvements keeping all HW revisions support


Implemented:
- Fixed power stall issue by swapping PWR_HOLD and RF_TXEN pins at MCU
- Fixed BLE UART module stuck in configuration mode
- Reduced Buzzer drive type from differential to single-ended
- Supported both: passive piezo and passive magnetic buzzers
- Added PWM-mode LCD backlight control
- Added separate ALARM button
- Added TU10-F QUESCAN SMD GPS module as an alternative
- Connected GPS receiver to MUX channel-4 for debug access
- Updated SMA footprint to fit both: 6.35 and 9.52 mm width connectors
- LEDs moved to the right to align with mounting hole
- Commented the schematic on components optional placement
- Other minor fixes


Known issues & limitations:
- No known issues


# Release FW Rev.5.0 (for HW Rev.1.x, Rev. 2.0, Rev 2.1) 06-Jan-2026 
All HW versions support, 868 MHz band with 1% duty cycle compliance, new packet format and lots of fixes and improvements


Implemented:
- Added support of the Spoke HW Rev.2.x
- Unified FW for all HW versions
- FW configuration is done at compile-time through defines in "config.h"
- PWM-controlled LCD backligh for Spoke HW Rev.2.x
- Single-ended PWM-controled passive buzzer drive for Spoke HW Rev.2.x
- PWR_HOLD line fix for Spoke HW Rev.2.x by swapping with RF_TXEN
- Separate BTN_ALARM line for alarm button for Spoke HW Rev.2.x
- Added 868 MHz band support for E22-900M22S radio module
- Added +14 dBm output power setting
- Added 120 sec update interval
- Updated packet format to 13 bytes with LDRO enabled
- Extended to POWER menu two-collumns
- Added DIAG menu with debug and test information
- Switching USB-UART/BLE-UART from menu
- Reading LoRa SNR instead of RSSI
- Removed GPS baud setting from menu
- Updated console-reports data sturcutre
- Implemented Kalman filter for accelerometer Z and magnetometed X, Y channels
- Minor and major fixes everywhere
- Refactoring for better code perception


Known issues & limitations:
- This FW version is incompatible over-the-air with previous FW versions because of different radio packet structure
- GPS NMEA parsing at 115200 baud may fail due to insufficient buffer size


# Release HW Rev.2.0 20-Sep-2025
Added USB Type C, USB-UART converter, UART-BLE Module, new LCD, DC/DC regulator


Implemented:
- New PCB size 100 x 50 mm
- Type C USB with CP2102N-28pin USB-UART converter
- UART-BLE connectivity by E104-BT5005A module
- Multiplexer to switch between USB and BLE
- New monochrome LCD with ST7567 controller
- DC/DC regulator LM3671MF instead of linear
- BOOT button to run the STM32 UART bootloader
- SMA connector for LoRa antenna
- MCU crystal in 3225 package
- 0603 components everywhere
- 3 mm TH LEDs
- MCU lines kept the same assignment as in HW Rev.1.x. for back-compatibility


Known issues & limitations:
- When no FW flashed (erased MCU) the PWR_HOLD line is held high by an internal JTDI pull-up resistor, so the device power does not switch off when power button gets released; battery disconnect is required
- BLE UART hangs in configuration mode because of logic levels conflict at MODE pin 


# Release FW Rev.4.0 (for HW Rev.1.x) 12-Apr-2025 
New look, compass with calibration, more settings & stabilization


Implemented:
- Dozens of bug fixes
- Revised interface, icons
- Added beep mute feature while scrolling devices
- Added GPS baud setting
- Implemented interactive compass calibration from menu
- Magnetic and True north icons
- Implemented navigational data output in Base64 through console
- Enabled watchdog
- Implemented periph. clock gating
- Improved uptime counter accuracy
- Changed battery level thresholds
- Added jump to bootloader at power up
- Changed optimization level to reduce flash usage


Known issues & limitations:
- GPS NMEA parsing at 115200 baud may fail due to insufficient buffer size


# Release HW Rev.1.2 02-Jun-2024 
Improvements


Implemented:
- Moved GPS module 2 mm down to align with buttons center
- Added text captions for GPS lines
- Updated stack-up to 1.6 mm overall thickness


Known issues & limitations:
- No known issues


# Release HW Rev.1.1 28-Jan-2024 
Fix of the HW release 1


Implemented:
- TEMP pin of the TP4056 connected to the GND directly


Known issues & limitations:
- No known issues


# Release FW Rev.3.0 (for HW Rev.1.0) 28-Jan-2024 
Switch to LoRa modulation, new TDMA approach, minor fixes


Implemented:
- LoRa modulation: SF12, BW125, CR 4/8, Header off, CRC
- TDMA approach: timeslot each even second (0, 2, 4...), RX/TX in 900 ms after PPS interrupt based on NMEA parsed in the same slot
- Other minor fixes


Known issues & limitations:
- Might be not stable, need to be tested for a long run


# Release FW Rev.2.0 (for HW Rev.1.0) 21-Nov-2023 
Lots of fixes, power consumption improvements, changes in apperance


Implemented:
- Reduced power consumption: MCU sleep mode, GPS aggressive 1 Hz mode via UBX configuration, PPS led reduced duty cycle
- Fix ADC battery readings; using internal reference for measurements
- Fix LCD buffer clear isuue when the update is ongoing
- Add device hard switch off at 2.8 V battery voltage
- Add more info for "reset to default" menu
- Add GPS configuration reset upon "reset to default"
- Change loading sequence; add infromation output to LCD for debug


Known issues & limitations:
- No known issues


# Release FW Rev.1.0 (for HW Rev.1.0) 01-Nov-2023 
The very first release of the firmware. Lots of imperfections and weak spots in code, as well as some minor bugs, but correctly working HW and SW with no major issues. 


Implemented:
- GPS NMEA parsing
- Radio communication, GFSK 1200 bps
- Interface and menu
- 4 memory points
- 2D compass with calibration and compensation


Known issues & limitations:
- ADC battery readings are incorrect below 3.3 V battery voltage due to Vref = VCC


# Release HW Rev.1.0 26-Mar-2023 
The very first release of the hardware. 


Implemented:
- See schematic


Known issues & limitations:
- TEMP pin of the TP4056 must be connected to the GND directly to disable temp protection feature. Cut the trace between U5 pin 1 and R8; then connect the pin 1 to GND.
