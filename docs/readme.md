# terralink

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

## receiver
board: NodeMCU 1.0(ESP-12E Module)
dev: /dev/ttyUSBx

libs to include:
- ESPAsyncTCP by dvarrel
- ESPAsyncWebSrv by dvarrel

connect to wifi:
- ssid: ESP32-Access-Point
- pass: 123456789
- ip: 192.168.4.1

## transmitter
board: esp32s3 dev module
dev: /dev/ttyACMx

## repeater
board: esp32-wroom-da module
dev: /dev/ttyUSBx

## literature
- lora ra-01: https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/datasheet/module/ra-01_product_specification_en_v1.1.pdf
- bcc: TP4056(03962A) - https://components101.com/modules/tp4056a-li-ion-battery-chargingdischarging-module