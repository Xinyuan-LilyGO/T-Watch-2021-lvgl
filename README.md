<h1 align = "center"> 🌟T-Watch-2021-lvgl🌟</h1>

### Arduino

1. In Arduino Preferences, on the Settings tab, enter the `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json` URL in the `Additional boards manager URLs` input box. 
2. Click OK and the software will install. 
3. Search for ESP32 in Tools → Board Manager and install ESP32-Arduino SDK (version 2.0.3 or above)
![](image/Arduino_board.png)
4. Copy everything from this repository lib folder to [Arduino library folder](https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries#manual-installation) (e.g. `C:\Users\YourName\Documents\Arduino\libraries`)
5. Select the correct settings in the Tools menu as shown below.

| Setting                  | Value                            |
| :----------------------- | :------------------------------- |
| Board                    | ESP32 Dev Module                 |
| USB CDC On Boot          | Disabled                         |
| CPU Frequency            | 240MHz (WiFi)                    |
| Core Debug Level         | None                             |
| USB DFU On Boot          | Enabled                          |
| Events Run On            | Core 1                           |
| Flash Mode               | QIO 80MHz                        |
| Flash Size               | 8MB (64Mb)                       |
| JTAG Adapter             | Disabled                         |
| Arduino Runs On          | Core 1                           |
| USB Firmware MSC On Boot | Disabled                         |
| Partition Scheme         | Huge APP (3MB APP/1.5MB SPIFFS)  |
| PSRAM                    | OPI PSRAM                        |
| USB Mode                 | Disabled                         |

### PlatformIO

1. PlatformIO plug-in installation: click on the extension on the left column → search platformIO → install the first plug-in
2. Click Platforms → Embedded → search Espressif 32 in the input box → select the corresponding firmware installation
