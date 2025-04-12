# Release FW Rev.4 (for HW Rev.1.x) 12-Apr-2025 
Switch to LoRa modulation, new TDMA approach, minor fixes


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


Known issues:
- GPS NMEA parsing at 115200 baud may fail due to insufficient buffer size


# Release HW Rev.1.1 28-Jan-2024 
Fix of the HW release 1


Implemented:
- TEMP pin of the TP4056 connected to the GND directly


Known issues:
- No known issues


# Release FW Rev.3 (for HW Rev.1) 28-Jan-2024 
Switch to LoRa modulation, new TDMA approach, minor fixes


Implemented:
- LoRa modulation: SF12, BW125, CR 4/8, Header off, CRC
- TDMA approach: timeslot each even second (0, 2, 4...), RX/TX in 900 ms after PPS interrupt based on NMEA parsed in the same slot
- Other minor fixes


Known issues:
- Might be not stable, need to be tested for a long run


# Release FW Rev.2 (for HW Rev.1) 21-Nov-2023 
Lots of fixes, power consumption improvements, changes in apperance


Implemented:
- Reduced power consumption: MCU sleep mode, GPS aggressive 1 Hz mode via UBX configuration, PPS led reduced duty cycle
- Fix ADC battery readings; using internal reference for measurements
- Fix LCD buffer clear isuue when the update is ongoing
- Add device hard switch off at 2.8 V battery voltage
- Add more info for "reset to default" menu
- Add GPS configuration reset upon "reset to default"
- Change loading sequence; add infromation output to LCD for debug


Known issues:
- No known issues


# Release FW Rev.1 (for HW Rev.1) 01-Nov-2023 
The very first release of the firmware. Lots of imperfections and weak spots in code, as well as some minor bugs, but correctly working HW and SW with no major issues. 


Implemented:
- GPS NMEA parsing
- Radio communication, GFSK 1200 bps
- Interface and menu
- 4 memory points
- 2D compass with calibration and compensation


Known issues:
- ADC battery readings are incorrect below 3.3 V battery voltage due to Vref = VCC


# Release HW Rev.1 26-Mar-2023 
The very first release of the hardware. 


Implemented:
- See schematic


Known issues:
- TEMP pin of the TP4056 must be connected to the GND directly to disable temp protection feature. Cut the trace between U5 pin 1 and R8; then connect the pin 1 to GND.
