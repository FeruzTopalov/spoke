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
