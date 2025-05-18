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

## Wiring Instructions (For FW16 Matrix)

| ESP Pin       | Matrix Pin   |
|---------------|--------------|
| CLK (SCK)     | CLK          |
| DATA (MOSI)   | DIN          |
| CS (SS)       | CS           |
| GND           | GND          |
| 3.3V or 5V    | VCC          |

Ensure the power supply can handle the current requirements of the LED matrix. Pin assignments will differ based on the MCU and board used. Always double check before powering up.

## GPIO Pins Used

| Board               | SCK  | MOSI | CS (SS) | Soft Reset | Address Scroll |
|----------------------|-----------|-------------|---------|------------|----------------|
| ESP32 (WROOM32D)    | GPIO18    | GPIO23      | GPIO5   | GPIO4      | GPIO13         |
| ESP32-C3 DevKitM-1  | GPIO4     | GPIO6       | GPIO7   | GPIO18     | GPIO2          |
| ESP32-C3 Zero       | GPIO4     | GPIO6       | GPIO7   | GPIO0      | GPIO1          |
| ESP8266 (ESP8266mod)| GPIO14    | GPIO13      | GPIO15  | GPIO4      | GPIO5          |


## Operation

### Access point mode (first boot)
When the module first starts it will be in WiFiManager mode. Nothing is displayed during this time.

The ESP module starts in access point mode, with a unique network name (SSID). Connect to this network using your device and you should be automatically taken to the configuration page. To access the configuration page manually navigate to the static IP address in any web browser (by default this is set as 192.168.4.1).

Access point mode is entered whenever a successful connection to WiFi could not be made.

### Normal operation

#### Power Up

Once a connection to a WiFi network has been established the matrix will scroll the IP address and URL of the configuration page, followed by either the built-in message or whichever message is stored in the ESP file system. The first message to be shown on connection to the WiFi network will always be the local IP address and URL.

#### Web portal

The web portal can be accessed over http using the port defined in platformio.ini (default is port 80). Additonally, mDNS is implemented, so if your device supports it you can access the web portal using the .local URL

The configuration page includes the following parameters:

| Parameter           | Input Type   | Description                                                                 | Limits                          |
|---------------------|--------------|-----------------------------------------------------------------------------|---------------------------------|
| **Message**         | Text Field   | Input the message to be displayed on the matrix.                           | Up to 500 characters.|
| **Intensity**       | Numeric Field| Adjust the brightness of the display.                                      | 0 (dim) to 15 (bright).         |
| **Speed**           | Numeric Field| Set the scrolling speed of the message.                                    | 10 (fast) to 200 (slow).          |
| **Display Flipped** | Checkbox     | Flip the display orientation.                                              | N/A                             |
| **Change Immediately** | Checkbox  | Apply changes instantly without waiting for the current message to finish. | N/A                             |

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
    "change_immediately: true
}
```