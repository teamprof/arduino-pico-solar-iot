# arduino-solar-iot
## A solar powered mosquito-killing IoT project for Arduino, running on WizFi360-EVB-Pico board
---

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://github.com/teamprof/arduino-eventethernet/blob/main/LICENSE)

<a href="https://www.buymeacoffee.com/teamprofnet" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 38px !important;width: 168px !important;" ></a>

---
## Important Notes
1. This project involves high voltage electronic circuit which requires professional knowhow. It has potential risk and NOT suiable for general person.
2. DO NOT touch the swatter when it is turn on.
3. This project is target for [WizFi360-EVB-Pico] board at this moment.


---

## Why do we need this [Solar powered mosquito-killing IoT product](https://github.com/teamprof/arduino-solar-iot)

1. Mosquitoes aren't just annoying, they spread killer diseases, as well. They are often called one of the most dangerous animals on the planet (https://www.businessinsider.com/11-reasons-mosquitoes-are-the-worst-2014-4). 
2. Mosquitoe-killing swatter is one of weapons to kill Mosquitoes, however, it is battery powered which is difficult to provide 24x7 operation.
3. AC powered modquito-killing product has its limitation of deploying on area where lack of electricity, e.g. rural area.
4. A solar powered mosquito-killing IoT product is introduced to provide 24x7 operation without electricity power line.


### Features
1. On and off the swatter regularly, to provide a 24x7 operation on a battery powered product
2. Regularly report battery status to cloud, to minimize supervision resource 
3. Powered by solar energy, a trend of providing human a better environment in future

---

## Currently supported Boards

1. [**WizFi360-EVB-Pico**](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico), using [**Arduino-mbed RP2040** core](https://github.com/arduino/ArduinoCore-mbed)

---

## Prerequisites
### Hardware
1. AllPowers Foldable Solar Panel Charger 5V10W with Built in USB (https://www.aliexpress.com/item/32859296874.html)
2. Joway 6000mAh power bank (a power bank who supports charging and discharging simultaneously)
3. Yage YG-5617 mosquito-killing swater
4. [WizFi360-EVB-Pico](https://docs.wiznet.io/Product/Open-Source-Hardware/wizfi360-evb-pico)
5. Load switch board

### Software
1. [`Azure IoT Hub](https://azure.microsoft.com/en-gb/services/iot-hub/)
2. [`Arduino IDE 1.8.19+` for Arduino](https://www.arduino.cc/en/Main/Software)
3. [`Arduino mbed_rp2040 core 2.7.2-`](https://github.com/arduino/ArduinoCore-mbed) 
4. ['Arduino DebugLog lib'](https://www.arduino.cc/reference/en/libraries/debuglog/)
5. [`WizFi360 library for Arduino'](https://www.arduino.cc/reference/en/libraries/wizfi360/)


---

## Installation

1. Please refer to "Hardware block diagran" about the connection between different components
2. Download the latest release `arduin-solar-iot.zip`, and extract the .zip file
3. Install DebugLog library (https://www.arduino.cc/reference/en/libraries/debuglog/) 
4. Launch arduino-ide, compile and upload to WizFi360-EVB-Pico board


---


## Code Examples TODO
```
There are three threads running. They are ThreadBattery, ThreadWizFi360 and ThreadSwatter.
   +---------------+                  +----------------+
   |               |                  |                |
   |               |  battery state   |                |   publish MQTT    +---------------+
   |               | ---------------> |                | ----------------> | Azure IoT Hub |
   | ThreadBattery |                  | ThreadWizFi360 |                   +---------------+
   |               |                  |                |
   |               |                  |                |       time        +---------------+
   |               |                  |                |  <--------------> |  SNTP server  |
   |               |                  |                |                   +---------------+
   +---------------+                  +----------------+


   +---------------+                  +------------------+
   |               |      on/off      | mosquito-killing |
   | ThreadSwatter | ---------------> |     swatter      |
   |               |                  |                  |
   +---------------+                  +------------------+

```
1. ThreadBattery's responsibility is to monitor power bank's battery state. It reports ThreadWizFi360 about the battery state, either state changed or hourly.

2. ThreadWizFi360's responsibility is to synchronize RTC by SNTP, and publish battery state to Azure IoT Hub by MQTT.

3. ThreadSwatter's responsibility is to turn on and off the mosquito-killing swatter by controlling the load-switch's EN pin. It turns on 5s and off 40s repeatedly.

---

## Code Examples TODO

[EventEthernetDemo](examples/EventEthernetDemo)

1. Receive Ethernet interrupt/status events
```
#define PIN_ETH_CS 17u   // W5100S-EVB-Pico: nCS = GPIO17
#define PIN_ETH_INTN 21u // W5100S-EVB-Pico: INTn = GPIO21

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
static byte mac[] = {
    0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

static void onEthernetEvent(uint8_t ir, uint8_t ir2, uint8_t slir)
{
  LOG_DEBUG("ir = ", DebugLogBase::HEX, ir, "(hex), ir2 = ", DebugLogBase::HEX, ir2, "(hex), slir = ", DebugLogBase::HEX, slir, " (hex)");
}

```

---

## Demos TODO

Before running the demo example in Arduino IDE, launch couples of Linux terminals and start UDP and TCP listeners on PC.

On terminal 1, type "nc -u -l 8060" <enter> (without quotation mark) to listen UDP data on the Linux PC's port 8060

On terminal 2, type "nc -l 8080" <enter> to listen TCP on the Linux PC's port 8080.

Connect the [W5100S-EVB-Pico] to PC with an USB cable. Plug an Ethernet cable to [W5100S-EVB-Pico].

Launch Arduino IDE, run the the demo example [EventEthernetDemo](examples/EventEthernetDemo). 
Launch Arduino serial monitor and it should show the following output 
```
current log level is 5
[DEBUG] EventEthernetDemo.ino L.345 initNetwork : Ethernet.init( 17 ,  21 ) // W5100S-EVB-Pico: nCS pin =  17 , Intn pin =  21
[DEBUG] EventEthernetDemo.ino L.372 initNetwork : localIP():  192.168.0.126
[DEBUG] EventEthernetDemo.ino L.373 initNetwork : dnsServerIP():  192.168.0.1
[DEBUG] EventEthernetDemo.ino L.294 initUdpEvent : Listening UDP on  192.168.0.126 : 8060
[DEBUG] EventEthernetDemo.ino L.111 initWebServer : Simple Web server at:  192.168.0.126 : 80
[DEBUG] EventEthernetDemo.ino L.218 initTcpClientEvent : Connected to 192.168.0.171 : 8080
[DEBUG] EventEthernetDemo.ino L.210 onTcpClientEvent : SnIR::CON



<<< launch a browser on PC and visit 192.168.0.126, the Arduino Serial Monitor should display >>>
[DEBUG] EventEthernetDemo.ino L.94 onTcpServerEvent : SnIR::RECV
[DEBUG] EventEthernetDemo.ino L.102 onTcpServerEvent : SnIR::CON
[DEBUG] EventEthernetDemo.ino L.36 handleNewConnectionEvent : new client from  192.168.0.171
[DEBUG] EventEthernetDemo.ino L.94 onTcpServerEvent : SnIR::RECV
[DEBUG] EventEthernetDemo.ino L.98 onTcpServerEvent : SnIR::DISCON

<<< launch the third Linux terminal 3, type "nc -u 192.168.0.126 8060" <enter>, it connects to [W5100S-EVB-Pico] UDP listener. >>>
<<< On terminal 3, type "hello from terminal" <enter> >>>
<<< terminal 3 should show: >>>
hello from terminal
echo received data: hello from terminal

<<< Linux terminal 1 should show: >>>
Received 1 packet(s), content = hello from terminal

<<< The Arduino Serial Monitor should show: >>>
[DEBUG] EventEthernetDemo.ino L.269 onUdpEvent : SnIR::RECV
[DEBUG] EventEthernetDemo.ino L.240 readUdpPacket : Received  20  bytes from  192.168.0.171 : 45974
[DEBUG] EventEthernetDemo.ino L.250 writeUdpPacket : Address =  192.168.0.171 : 45974 , data =  echo received data: hello from terminal
[DEBUG] EventEthernetDemo.ino L.250 writeUdpPacket : Address =  192.168.0.171 : 8060 , data =  Received 1 packet(s), content = hello from terminal

<<<  On terminal 2, type hey <enter> >>>
<<< terminal 2 should show: >>>
  hey
  
<<< Arduino Serial Monitor should show: >>>
  [DEBUG] EventEthernetDemo.ino L.80 onTcpClientEvent : SnIR::RECV
  [DEBUG] EventEthernetDemo.ino L.58 readTcpPacket : Received 4 bytes from 192.168.0.171 : 8080
  [DEBUG] EventEthernetDemo.ino L.59 readTcpPacket : Content: hey

<<< On Arduino Serial Monitor, type "hello" <control+enter> >>>
<<< terminal 2 should show: >>>
  hey
  hello

```

## Debug TODO

Debug is disabled by "#define DEBUGLOG_DISABLE_LOG"

Enable trace debug by "#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE"

Example of EventEthernetDemo.ino
```
// Disable Logging Macro (Release Mode)
// #define DEBUGLOG_DISABLE_LOG

// You can also set default log level by defining macro (default: INFO)
#define DEBUGLOG_DEFAULT_LOG_LEVEL_TRACE
#include <DebugLog.h> // https://github.com/hideakitai/DebugLog

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }
  LOG_ATTACH_SERIAL(Serial);

  // The default log_leval is DebugLogLevel::LVL_INFO
  // 0: NONE, 1: ERROR, 2: WARN, 3: INFO, 4: DEBUG, 5: TRACE
  PRINTLN("current log level is", (int)LOG_GET_LEVEL());

  // ...
}
```
## Screenshots TODO

[![screenshot-arduino](/doc/screenshot-arduino.jpg)](https://github.com/teamprof/arduino-eventethernet/blob/main/doc/screenshot-arduino.jpg)

[![screenshot-terminals](/doc/screenshot-terminals.jpg)](https://github.com/teamprof/arduino-eventethernet/blob/main/doc/screenshot-terminals.jpg)

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
2. Azure IoT Hub Connection, report battery status every hour or battery status is changed
3. Monitoring power bank battery status (charging or discharging)

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

