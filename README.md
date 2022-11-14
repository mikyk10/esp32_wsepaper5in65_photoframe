# esp32_wsepaper5in65_photoframe

A low-power comsumption photoframe enabled by 7 color ePaper module.

Have you ever wanted to own a battery powered dynamic photoframe that periodically updates its content wirelessly.


## BOM

- Waveshare Universal e-Paper ESP32 module
- Waveshare .... (epaper)
- Battery board

## Contents

This project consists of following 3 parts:

- Firmware for MCU
- Battery Unit PCB design
- Web Application

### Firmware

It may communicate with the server using TLS, the battery life will be shorten as it consumes a lot of processing power.  


### Battery Unit PCB design

Charging circuit to handle 18650 Li-Po batt.
There are handful of hand-soldering of SMD components. Most of them are 0805 size. 

### Web Application

An web application that helps to show images to the display.
It corrects gamma, aspect-ratio, resize and down-samples color of an image and encodes it into vendor specific octet stream that represents the image.

This may need to run on a private network as it .....................

 
# Contribution

Contribution is always welcome. You may fork this repository, change whatever you want and send me a PR.


# Disclaimer

This is a currently-under-development-PoC-project; does not gurantee to ...
Thus I am not take any responsibility of any damage of using this code, PCB design. liability

You may need to research, fix any existing issue of design flaw inside this project.
If you want to 

This project handles a Li-Po battery which must properly treated or it may explode, cause a fire.
Please know what you're going to do before doing anything with it.

