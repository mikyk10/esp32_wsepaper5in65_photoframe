ESP32_WSEPaper-5in65_PhotoFrame
====

A DIY battery powered digital photo frame with color e-Paper module that periodically updates its content via Wi-Fi.

![](https://via.placeholder.com/720x480.png?text=PhotoFrame)

### Features

- Color ePaper
- WiFi connectivity
- Battery Powered (Optional)

## Repository Contents

This project consists of following 3 parts:

- Helper Web API (HWA)
- Firmware for [MCU](https://www.waveshare.com/e-paper-esp32-driver-board.htm)
- Battery Unit for MCU

### Helper Web API (HWA)

Randomly picks an image and transforms it into vendor specific binary data after image pre-processing. This can match up image orientation, aspect ratio, dimension, gamma, color..etc with the ePaper's hardware specification.

The vendor specific encoding for the ePaper is described here:
https://www.waveshare.com/wiki/5.65inch_e-Paper_Module_(F)_Manual#Picture_Production

### Firmware for MCU

Core software to display an image on the ePaper. It transfers encoded data to the ePaper after getting data via WiFi. It then goes into deep-sleep for certain period of time.
In order to flash the MCU, you should edit `config.h` which describes Wi-Fi credentials and the URL of HWA.

Currently it does not have a fancy Web User-Interface to manage any configuration; including WiFi SSID, password and HWA URL are statically compiled into the firmware. Everytime you change the configuration, flash the MCU. 

TLS handshake is disabled by default as it drain the battery a little more. To protect your privacy, you should place the HWA and MCU on the same private network so that your pictures will never go through internet unencrypted.

### Battery Unit for MCU

Battery power unit for MCU. It has battery charging, protection, voltage regulation circuit for a 18650 Li-Po battery cell.

PCB design is available at EasyEDA. 
https://u.easyeda.com/account/user/projects/index/detail?project=549b435850194f8fa61f18e4934bcfd8&folder=all

## How to Build

### BOM

| Name                                                                                                                             | Qty | Note                                                                                                                                                                                        |
|----------------------------------------------------------------------------------------------------------------------------------|-----|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [Waveshare Universal e-Paper Raw Panel Driver Board](https://www.waveshare.com/e-paper-esp32-driver-board.htm)                   | 1   | The MCU for this project. You can power the MCU by plugging micro-USB instead of the Battery Unit.
                                                                                                                                                                 |
| [Waveshare 5.65inch E-Paper E-Ink Raw Display, without PCB](https://www.waveshare.com/product/displays/5.65inch-e-paper-f.htm)   | 1   | 7-Color ePaper Display module, 600×448 resolution.                                                                                                                                          |
| [Battery Unit PCB](https://u.easyeda.com/account/user/projects/index/detail?project=549b435850194f8fa61f18e4934bcfd8&folder=all) | 1   |                                                                                                                                                                                             |
| 18650 battery cell                                                                                                               | 1   | Care for the dimension. Please note some 18650 battery cell won't fit in battery holder as they have in-built protection circuit, lengthen tottal length. To avoid such case, use an unprotected cell.                                                                                                                     |
| Local network Web server                                                                                                         | 1   | I use a Raspberry pi 4B running Web server.                                                                                                                                 |
| Photo frame                                                                                                                      | 1   | Where your ePaper and stuffs go into. Get your favorite one at a hardware store. If you want to hang it on walls, you would need extra consideration for it. This repository won't cover on this.                               |
| Front panel                                                                                                                      | 1   | 2mm thick acrylic sheet with a rectangular opening for the ePaper. There are many online factory for acrylic sheet you can order. The opening dimension is equal to that of E-Paper. |
 
### Tools 

You need basic electronics toolsets.

### Build TIPS

The MCU has a red power status LED which constantly eats some current, shortens the battery life. It's directly connected to VCC so it must be physically removed if want to disable it. You can break off the LED using a plier. Be sure not to damage any other component.

## Contribution

Contribution is always welcome. You may fork this repository, change whatever you want and send me a PR.

## TODOs

- Orientation sensing / change
- Better documentation

## Disclaimer

This project is a currently PoC project, not production ready. Some artifacts are crude and a lot of work still should be done. Thus use of this is at your own risk. 
Li-Po battery cell which must be properly handled or it may explode, cause serious damages to you. 
I provide those information as reference for building a designing PCB, program code under GPLv3 license. Using them in your project may require some research of one's own, potentially fixing any issues or design flaws if they exist.

