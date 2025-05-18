# ESP Matrix Display
A simple scrolling message display that can be updated over WiFi from a web browser. Written using PlatformIO.

## Hardware Requirements

The code has been tested on the Wemos D1 R2 board and on a generic ESP32-C3 devboard, though any ESP8266 or ESP32 should work (check the pin numbers).

The matrix module used was a generic 32x8 module, commonly sold on Amazon, Ebay etc. The MD_Parola library defines this as FC16 hardware.

## Installation

1. Install [PlatformIO](https://platformio.org/).

2. Clone this repository

3. Open the project in your preferred editor/IDE

4. Identify the appropriate environment for your development board and set the appropriate build flags in```platformio.ini``` (See below).

5. Build and upload the file system image

    ```pio run --target buildfs --environment <environment>```

    ```pio run --target uploadfs --environment <environment>```

6. Build and upload the firmware.

    ```pio run --target upload --environment <environment>```

## Build Flags

There are several build flags that can be set in the platformio.ini file to change the way the project is build. See [PlatformIO Build flags](https://docs.platformio.org/en/stable/projectconf/sections/env/options/build/build_flags.html) for more details.


| Flag                  | Purpose                                               | Mandatory | Example                                      |
| ----------------------|-------------------------------------------------------|-----------|----------------------------------------------|
|```HAS_NEOPIXEL```     |Define if a neopixel (WS2812b) is present              |No         |N/A                                           |
|```NEOPIXEL_PIN```     |Sets the GPIO pin that the WS2812b LED is connected to |No         |```NEOPIXEL_PIN=10```                         |
|```FACTORY_RESET_PIN```|Sets the GPIO pin used for the factory reset button    |Yes        |```FACTORY_RESET_PIN=2```                     |
|```ADDR_SCROLL_PIN```  |Sets the GPIO pin used for the address scroll button   |No         |```ADDR_SCROLL_PIN=4```                       |
|```WEB_SERVER_PORT```  |Sets the port that the http server responds on         |No         |```WEB_SERVER_PORT=8080```                    |
|```LED_COLOUR_ORDER``` |Sets the channel order for the neopixel colours        |No         |```LED_COLOUR_ORDER=GRB```                    |
|```APNAME_PREFIX```    |SSID Prefix for setup AP                               |No         |```APNAME_PREFIXAPNAME_PREFIX="\"Matrix_\""```|
|```HOSTNAME_PREFIX```  |Prefix for network hostname and URL                    |No         |```HOSTNAME_PREFIX="\"c3matrix-\""```         |

## Libraries

The required libraries are listed in the [platformio.ini](http://_vscodecontentref_/0) file. PlatformIO will automatically download and install them when you build the project.

## Wiring Instructions (For FC16 Matrix)

| ESP Pin       | Matrix Pin   |
|---------------|--------------|
| CLK (SCK)     | CLK          |
| DATA (MOSI)   | DIN          |
| CS (SS)       | CS           |
| GND           | GND          |
| 3.3V or 5V    | VCC          |

Ensure the power supply can handle the current requirements of the LED matrix. Pin assignments will differ based on the MCU and board used. Always double check before powering up.

## GPIO Pins Used

| Board            | SCK         |     MOSI    | CS (SS)     | Factory Reset | Address Scroll |
|---------------------|-------------|-------------|-------------|------------|----------------|
| ESP32               | GPIO18      | GPIO23      | GPIO5       | GPIO4      | GPIO13         |
| ESP32-C3 DevKitM-1  | GPIO4       | GPIO6       | GPIO7       | GPIO18     | GPIO2          |
| ESP32-C3 Zero       | GPIO4       | GPIO6       | GPIO7       | GPIO0      | GPIO1          |
| ESP8266 (Wemos D1)  | GPIO14 (D5) | GPIO13 (D7) | GPIO15 (D8) | GPIO4 (D2) | GPIO5 (D1)     |

The button pins have been chosen so that they use internal pullups, so no external pullup is required. This allows the device to function even if those pins are left unconnected.

## Operation

On power up, the display will show ```BOOT``` for a short time.

### Access point mode (first boot)
When the module first starts it will be in WiFiManager mode. During this time, the display will show ```SETUP```

The ESP module starts in access point mode, with a unique network name (SSID) that is determined by the ```APNAME_PREFIX``` definition and the last six hex characters of the device's ID/MAC address. Connect to this network using your device and you should be automatically taken to the configuration page. To access the configuration page manually, navigate to the static IP address in any web browser (by default this is set as 192.168.4.1).

Access point mode is entered whenever a successful connection to WiFi could not be made.

### Normal operation

#### Power Up

Once a connection to a WiFi network has been established the matrix will scroll the IP address and URL of the configuration page, followed by either the built-in message or whichever message is stored in the ESP file system. The first message to be shown on connection to the WiFi network will always be the local IP address and URL.

#### Buttons

There are two optional buttons that have been implemented.

* **Factory Reset** - wipes all user data (messages, display settings, WiFi settings) and puts the device back into access point mode. If pressed, the display will show ```RESET``` while user data is erased.

* **Address Scroll** - Clears the display and scrolls the IP address and URL.

#### Web portal

The web portal can be accessed over http using the port defined in platformio.ini (default is port 80). Additionally, mDNS is implemented, so if your device supports it you can access the web portal using the .local URL. This URL can be customised by using the build flag ```HOSTNAME_PREFIX```.


The configuration page includes the following parameters:

| Parameter              | Input Type  | Description                                                                | Limits                          |
|------------------------|-------------|----------------------------------------------------------------------------|---------------------------------|
| **Message**            |Text Field   | Input the message to be displayed on the matrix.                           | Up to 500 characters.           |
| **Intensity**          |Numeric Field| Adjust the brightness of the display.                                      | 0 (dim) to 15 (bright).         |
| **Speed**              |Numeric Field| Set the scrolling speed of the message.                                    | 10 (fast) to 200 (slow).        |
| **Display Flipped**    |Checkbox     | Flip the display orientation.                                              | N/A                             |
| **Change Immediately** |Checkbox     | Apply changes instantly without waiting for the current message to finish. | N/A                             |

When a new message or setting is submitted, the display updates immediately (if "Change Immediately" is checked) or after the current message finishes. All settings and messages are stored in the ESP module's file system, ensuring persistence across power cycles.

#### REST API

Messages and settings can be set and retrieved using the ```/api/message``` API endpoint.

* **HTTP GET** requests return the following parameters as a JSON payload:
```
{
    "message": "The quick brown fox jumps over the lazy dog",
    "message_max_len": 500,
    "intensity": 5,
    "intensity_min": 0,
    "intensity_max": 15,
    "speed": 30,
    "speed_min": 10,
    "speed_max": 200,
    "display_flipped": false
}
```
* **HTTP PUT** requests can accept the following parameters in any combination, using ```Content-Type: application/json```
```
{
    "message": "The quick brown fox jumps over the lazy dog",
    "intensity": 5,
    "speed": 30,
    "display_flipped": false,
    "change_immediately": true
}
```

TODO
* Neopixel
* Screen messages