# ESP Matrix Display
A simple scrolling message display that can be updated over WiFi from a web browser. Written using PlatformIO.

## Hardware Requirements

The code has been tested on the Wemos D1 R2 board and on a generic ESP32-C3 devboard, though any ESP8266 or ESP32 should work (check the pin numbers).

The matrix module used was a generic 32x8 module, commonly sold on Amazon, Ebay etc. The MD_Parola library defines this as FC16 hardware.

## Installation

1. Install [PlatformIO](https://platformio.org/).

2. Clone this repository

3. Open the project in your preferred editor/IDE

4. Select the appropriate environment for your development board (or create your own) in ```platformio.ini```

5. Build and upload the file system image

    ```pio run --target buildfs --environment <environment>```

    ```pio run --target uploadfs --environment <environment>```

6. Build and upload the firmware.

    ```pio run --target upload --enviroment <environment>```

## Libraries

The required libraries are listed in the [platformio.ini](http://_vscodecontentref_/0) file. PlatformIO will automatically download and install them when you build the project.
| Library | Version | Author |
|---|---|---|
| [MD_MAX72XX](https://github.com/MajicDesigns/MD_MAX72XX) | 3.5.1 | majicDesigns |
| [MD_Parola](https://github.com/MajicDesigns/MD_Parola) | 3.7.3 | majicDesigns |
| [WiFiManager](https://github.com/tzapu/WiFiManager) | 2.0.17 | tzapu |

## Wiring Instructions

| ESP Pin       | Matrix Pin   |
|---------------|--------------|
| CLK (SCK)     | CLK          |
| DATA (MOSI)   | DIN          |
| CS (SS)       | CS           |
| GND           | GND          |
| 3.3V or 5V    | VCC          |

Ensure the power supply can handle the current requirements of the LED matrix. Pin assignments will differ based on the MCU and board used. Always double check before powering up.

## Operation

### Access point mode (first boot)
When the module first starts it will be in WiFiManager mode. Nothing is displayed during this time.

The ESP module starts in access point mode, with a unique network name (SSID). Connect to this network using your device and you should be automatically taken to the configuration page. To access the configuration page manually navigate to the static IP address in any web browser (by default this is set as 10.0.0.1).

Access point mode is entered whenever a successful connection to WiFi could not be made.

### Normal operation

Once a connection to a WiFi network has been established the matrix will scroll the IP address of the configuration page, followed by either the built-in message or whichever message is stored in the ESP file system. The first message to be shown on connection to the WiFi network will always be the local IP address.

The configuration page is a simple form that has fields for a message, the display intensity (brightness) and scroll speed. When a new message is sent the old message is interrupted and the display cleared. Messages are stored in the ESP module's file system, so they are retained even when the system is powered down.