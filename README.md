# arduino-solar-iot
## A solar powered mosquito-killing IoT project for Arduino, running on WizFi360-EVB-Pico board
---

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://github.com/teamprof/arduino-eventethernet/blob/main/LICENSE)

<a href="https://www.buymeacoffee.com/teamprofnet" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 38px !important;width: 168px !important;" ></a>

---
## Important Notes
### 1. This project involves high voltage electronic circuit which requires professional knowhow. There is potential risk which is NOT suiable for general person.
### 2. DO NOT touch the swatter.
### 3. This project is target for [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico) board at this moment.


---

## Why do we need this [Solar powered mosquito-killing IoT product](https://github.com/teamprof/arduino-solar-iot)

### 1. Mosquitoes aren't just annoying, they spread killer diseases, as well. They are often called one of the most dangerous animals on the planet (https://www.businessinsider.com/11-reasons-mosquitoes-are-the-worst-2014-4). 
### 2. Mosquitoe-killing swatter is one of weapons to kill Mosquitoes, however, it is battery powered which is difficult to provide 24x7 operation.
### 3. AC powered modquito-killing product has its limitation of installing on area without electricity, e.g. rural area.
### 4. A solar powered mosquito-killing IoT product is introduced to provide 24x7 operation without electricity power line.

---
## Features
#### 1. A solar panel provides energy on charing power bank. Solar energy is a clean energy which is a trend of providing human a better environment in future
#### 2. Power bank provides power to [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico) and Mosquitoe-killing swatter;
#### 3. [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico) turns swatter on and off regularly. This reduces power consumption such that it supports 24x7 operation;
#### 4. [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico) reports battery status to cloud every hour, this minimize supervision resource on site;

---

## Currently supported Boards

1. [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico), using [**Arduino-mbed RP2040** core](https://github.com/arduino/ArduinoCore-mbed)

---

## Prerequisites
### Hardware
1. AllPowers Foldable Solar Panel Charger 5V10W with Built in USB (https://www.aliexpress.com/item/32859296874.html)
   [![Solar panel](images/photo/hardware/solar-panel-2.JPG)](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/photo/hardware/solar-panel-2.JPG)

2. [Joway 6000mAh power bank](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/photo/hardware/power-bank-1.jpg) (or a power bank who supports charging and discharging simultaneously)
   [![Joway 6000mAh power bank](images/photo/hardware/power-bank-1.JPG)](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/photo/hardware/power-bank-1.JPG)

3. Yage YG-5617 mosquito-killing swatter 

   [![Yage YG-5617 mosquito-killing swatter](images/photo/hardware/mosquito-swatter-1.JPG)](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/photo/hardware/mosquito-swatter-1.JPG)

4. [WizFi360-EVB-Pico](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico)

5. [Battery level detection board](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.68e42e8d6qSWMp&id=624781563779&_u=l3no53d8eac4)
   [![Battery detection IC](images/schematic/battery-detection.JPG)](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/schematic/battery-detection.JPG)

6. Load switch board
   [![Load Switch](images/schematic/load-switch.JPG)](https://github.com/teamprof/arduino-pico-solar-iot/blob/main/images/schematic/load-switch.JPG)

### Software
1. [`Azure IoT Hub](https://azure.microsoft.com/en-gb/services/iot-hub/)
2. [`Arduino IDE 1.8.19+` for Arduino](https://www.arduino.cc/en/Main/Software)
3. [`Arduino mbed_rp2040 core 2.7.2-`](https://github.com/arduino/ArduinoCore-mbed) 
4. ['Arduino DebugLog lib'](https://www.arduino.cc/reference/en/libraries/debuglog/)
5. [`WizFi360 library for Arduino'](https://www.arduino.cc/reference/en/libraries/wizfi360/)


---

## Installation TODO

---

## Demo Video TODO
---

## Screenshots 

[![screenshot-arduino](/images/screenshot/log-screen.png)](https://github.com/teamprof/arduino-pico-solar-iot/tree/master/images/screenshot/log-screen.png)


---
## Code Examples TODO
```

```
---


## Troubleshooting

If you get compilation errors, more often than not, you may need to install a newer version of the core for Arduino boards.

Sometimes, the library will only work if you update the board core to the latest version because I am using newly added functions.

---

## Issues

Submit issues to: [Solar powered IoT issues](https://github.com/teamprof/arduino-solar-iot/issues) 

---

## TO DO

1. Search for bug and improvement.

---

## DONE
Basic demo of the followings:
1. Turn mosquito-killing swatter on 10s, then turn it off for 40s
2. Azure IoT Hub Connection, report battery status every hour 

---

## Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.

---

## Contributing

If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

## License

- The library is licensed under GNU LESSER GENERAL PUBLIC LICENSE Version 3
---

## Copyright

- Copyright 2022- teamprof.net@gmail.com

