ESP32_WSEPaper-5in65_PhotoFrame
====

Ever wanted to own a digital photo frame that periodically updates its content wirelessly? This is a DIY low-power consumption Digital Photo Frame enabled by color e-Paper module.

![](https://via.placeholder.com/720x480.png?text=PhotoFrame)

### Features

- Color ePaper
- WiFi connectivity
- Landscape orientation only
- Powered by Rechargable Battery (Optional)

## Repository Contents

This project consists of following 3 parts:

- Helper Web API (HWA)
- Firmware
- Battery Unit

### Helper Web API (HWA)

Randomly picks an landscape image and transforms it into vendor specific stream of binary data after image pre-processing. This can match up image orientation, aspect ratio, dimension, gamma, color..etc with the ePaper's hardware specification.

The vendor specific ePaper encoding is described here:
https://www.waveshare.com/wiki/5.65inch_e-Paper_Module_(F)_Manual#Picture_Production

### Firmware

Core software to display images on the ePaper. It transfers encoded data to the ePaper after getting data via WiFi. It then goes into deep-sleep mode for certain period of time.

Currently it does not have a fancy Web User-Interface to manage any configuration; including WiFi SSID, password and HWA URL are statically compiled into the firmware. Everytime you change the configuration, flash the MCU. 

By default, TLS handshake is disabled as it might drain the battery. To protect your privacy, you should place the HWA and MCU on the same private network so that your pictures will never go through internet unencrypted.

### Battery Unit

Reference Only.

Battery power unit for e-Paper ESP32 Driver Board. It has battery charging, protection, voltage regulation circuit for a 18650 Li-Po battery cell.

PCB design is available at EasyEDA. 
https://u.easyeda.com/account/user/projects/index/detail?project=549b435850194f8fa61f18e4934bcfd8&folder=all

## How to Build

### Parts list

| Name                                                                                                                             | Qty | Note                                                                                                                                                                                        |
|----------------------------------------------------------------------------------------------------------------------------------|-----|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [Waveshare Universal e-Paper Raw Panel Driver Board](https://www.waveshare.com/e-paper-esp32-driver-board.htm)                   | 1   | The MCU for this project.                                                                                                                                                                   |
| [Waveshare 5.65inch E-Paper E-Ink Raw Display, without PCB](https://www.waveshare.com/product/displays/5.65inch-e-paper-f.htm)   | 1   | 7-Color ePaper Display module, 600×448 resolution.                                                                                                                                          |
| [Battery Unit PCB](https://u.easyeda.com/account/user/projects/index/detail?project=549b435850194f8fa61f18e4934bcfd8&folder=all) | 1   |                                                                                                                                                                                             |
| 18650 battery cell                                                                                                               | 1   | Care for the dimension. Some battery cell has protection circuit in it.                                                                                                                     |
| Local network Web server                                                                                                         | 1   | I use a Raspberry pi 4B running Web server described below.                                                                                                                                 |
| Photo frame                                                                                                                      | 1   | Where your ePaper and stuffs go into. Get your favorite one at a hardware store. If you want to hang it on walls, you would need extra consideration for it.                                |
| Front panel                                                                                                                      | 1   | 2mm thick acrylic sheet with a rectangular opening for the ePaper. There are many online stores for acrylic sheet you can order. The opening dimension is about 999.999mm(W) x 999.999mm(H) |
 
### Tools 

You need basic electronics toolsets.

### Build Instruction

The MCU has a red power status LED which constantly eats about 0000mA, shortens the battery life. It's directly connected to VCC so it must be physically removed if want to disable it. You can break off the LED using a plier. Be sure not to damage any other component.

## Contribution

Contribution is always welcome. You may fork this repository, change whatever you want and send me a PR.

## TODOs

- Orientation sensing / change

## Disclaimer

This project is a currently-under-development-PoC-project. It is not production ready state Some code is crude and a lot of work still should be done. So use of this artifact is at your own risk. 
You may optionally use Li-Po battery cell which must be properly handled or it may explode, cause serious damages to you. Please be sure what is going to happen before doing anything with battery and its peripherals.

I provide those information as a reference for building a designing PCB, program code. Using them in your project may require some research of one's own, potentially fixing any issues or design flaws if they exist.














## Installation

To install all of the template files, run the following script from the root of your project's directory:

```
bash -c "$(curl -s https://raw.githubusercontent.com/CFPB/development/main/open-source-template.sh)"
```

----

# Project Title

**Description**:  Put a meaningful, short, plain-language description of what
this project is trying to accomplish and why it matters.
Describe the problem(s) this project solves.
Describe how this software can improve the lives of its audience.

Other things to include:

  - **Technology stack**: Indicate the technological nature of the software, including primary programming language(s) and whether the software is intended as standalone or as a module in a framework or other ecosystem.
  - **Status**:  Alpha, Beta, 1.1, etc. It's OK to write a sentence, too. The goal is to let interested people know where this project is at. This is also a good place to link to the [CHANGELOG](CHANGELOG.md).
  - **Links to production or demo instances**
  - Describe what sets this apart from related-projects. Linking to another doc or page is OK if this can't be expressed in a sentence or two.


**Screenshot**: If the software has visual components, place a screenshot after the description; e.g.,

![](https://raw.githubusercontent.com/cfpb/open-source-project-template/main/screenshot.png)


## Dependencies

Describe any dependencies that must be installed for this software to work.
This includes programming languages, databases or other storage mechanisms, build tools, frameworks, and so forth.
If specific versions of other software are required, or known not to work, call that out.

## Installation

Detailed instructions on how to install, configure, and get the project running.
This should be frequently tested to ensure reliability. Alternatively, link to
a separate [INSTALL](INSTALL.md) document.

## Configuration

If the software is configurable, describe it in detail, either here or in other documentation to which you link.

## Usage

Show users how to use the software.
Be specific.
Use appropriate formatting when showing code snippets.

## How to test the software

If the software includes automated tests, detail how to run those tests.

## Known issues

Document any known significant shortcomings with the software.

## Getting help

Instruct users how to get help with this software; this might include links to an issue tracker, wiki, mailing list, etc.

**Example**

If you have questions, concerns, bug reports, etc, please file an issue in this repository's Issue Tracker.

## Getting involved

This section should detail why people should get involved and describe key areas you are
currently focusing on; e.g., trying to get feedback on features, fixing certain bugs, building
important pieces, etc.

General instructions on _how_ to contribute should be stated with a link to [CONTRIBUTING](CONTRIBUTING.md).
