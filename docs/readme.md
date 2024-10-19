# terralink

## 3d model
battery = 55x50
bolt - m3: diameter = 2.87-2.98mm

## lora ra-01 sx1278

| | |
|-|-|
| Module | Model Ra-01 |
| Package | SMD-16 |
| Size | 17*16*(3.2 ± 0.1) mm |
| Interface | SPI |
| Programmable | bit rate UP to 300Kbps |
| Frequency | Range 410-525 MHz |
| Antenna | Spring antenna with gain of 2.5 dBi |
| Max | Transmit Power 18±1 dBm |
| Power | Supply 2.5~3.7VˈTypical 3.3V |
| Operating Temperature | -30 ć ~ 85 ć |
| Storage Environment | -40 ć ~ 90 ć , < 90%RH |
| Weight | 0.45g |

| | | | |
|-|-|-|-|
| Frequency | Spread factor | SNR | Sensitivity |
| 433MHz | 7 | -7 | -125 |
| 433MHz | 10 |-15 | -134 |
| 433Mhz | 12 | -20 | -141 |
| 470MHz | 7 | -7 | -126 |
| 470MHz | 10 | -15 | -135 |
| 470MHz | 12 | -20 | -141 |

## esp32-s3 zero

## buzzer
- 5v - input voltage
- gnd - ground
- buz - control pin

## battery charge controller TP4056(03962A)
|||
|-|-|
| pin | description |
| OUT + | This pins outputs the positive voltage from battery. It should be connected to the circuit which has to be powered by the battery |
| OUT - | This pin outputs negative voltage from battery. It should be connected to the ground of circuit which has to be powered by the battery |
| B + | Outputs positive voltage from USB cable to charge to battery. It should be connected to the positive of the battery |
| B - | Outputs negative voltage from USB cable for charging battery. It should be connected to negative of the battery |
| IN + | Should provide +5V, can be used if charge cable not available |
| IN - | Should provide ground of the +5V supply, can be used if charge cable not available |
| LED Red | This LED turns on while the battery is charging |
| LED Green | This LED turns on after the battery is fully charged |

## resistor voltage divider

## compilling
tools -> usb cdc on boot -> enabled
tools -> upload mode -> uart0 / hardware cdc
tools -> upload speed -> 115200
port: /dev/ttyACM0
board: esp32s3 dev module

## programming
pip install esptool
esptool.py --chip esp32s3 erase_flash
Sketch -> export compiled binary
cd transmitter/build
esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 115200 write_flash -z 0x1000 transmitter.ino.bin
(optional) reflash using arduino ide

## literature
- lora ra-01: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/datasheet/module/ra-01_product_specification_en_v1.1.pdf
- bcc: TP4056(03962A) - https://components101.com/modules/tp4056a-li-ion-battery-chargingdischarging-module