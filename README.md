[![PlatformIO](https://github.com/hpsaturn/esp32-wifi-cli/workflows/PlatformIO/badge.svg)](https://github.com/hpsaturn/esp32-wifi-cli/actions/) ![ViewCount](https://views.whatilearened.today/views/github/hpsaturn/esp32-wifi-cli.svg)

# esp32-nmcli

Basic and extendible Wifi network manager CLI via serial command line for ESP32

## Features

- [x] Network manager nmcli command (connect, up, down, scan, list, etc)
- [x] Wifi multi AP and single AP modes
- [x] **New Telnet service** for remote shell via IP and port
- [x] **New shell improved** & migrated to Shellminator development
- [x] **New interactive shell**, autocomplete, history, prompt, logo and others
- [x] extendible custom user commands
- [x] preferences persist in flash ROM
- [x] two parsers: Argument into quotes or two parameters without quotes
- [x] debug messages mode off (new silent mode)
- [x] disable auto connect in the boot
- [x] esp32c3, esp32s3 support
- [x] VT100 compatibility
- [ ] esp8266 support

## Basic implementation

```cpp
#include <Arduino.h>
#include <ESP32WifiCLI.hpp>

void setup() {
  Serial.begin(115200);  // set here the Serial baudrate or in
  wcli.begin();          // the begin method like a parameter
}

void loop() {
  wcli.loop();
}
```

## Network manager commands

This is a sample of the nmcli commands output:

![nmcli preview commands](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/version2nmcli.jpg)

### nmcli connect

For instance for setup a new connection:

```bash
nmcli connect Your SSID password "Your Password"
```

Note that you could enter your ssid with spaces but without quotes, and your password with spaces but into quotes.

### nmcli tips

nmcli support many WiFi networks, and you are able to list those with `nmcli list`, also select your WiFi network default with `nmcli select #` where # is the number or ID of each WiFi network listed with `list` command. Also, you could enable multi network with `nmcli mode multi`, that means that the device will try to detect what WiFi network is available, or force with `nmcli mode single` to use only your WiFi network default.

## Custom parameters

For instance you can extend the available commands adding new ones in the setup, also add your prompt name and custom logo, like this example:

```cpp
void setup() {
  Serial.begin(115200);  // Optional, you can init it on begin()
  Serial.flush();        // Only for showing the message on serial
  
  wcli.setCallback(new mESP32WifiCLICallbacks()); // (optional)
  wcli.setSilentMode(true);  // less debug output

  // Custom commands example:
  wcli.add("sleep", &sleep,     "\t\t<mode> <time> ESP32 sleep mode (deep/light)");
  wcli.add("echo", &echo,       "\t\t\"message\" Echo the msg. Parameter into quotes");
  wcli.add("info", &info,       "\t\tsystem status info");
  wcli.add("setled", &setled,   "\t<PIN> config the LED GPIO for blink");
  wcli.add("blink", &blink,     "\t\t<times> <millis> LED blink");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  
  wcli.shell->attachLogo(logo);
  wcli.shell->clear();
  wcli.begin("MyShell"); // your prompt custom name
}
```

For more details, please review the [M5Atom](examples/M5Atom/main.cpp) and [Advanced](examples/advanced/main.cpp) examples.

## Global configuration

The v0.3.x version uses Shellminator that needs some special build flags. Please add these in your build config, for instance in PlatformIO ini file:

```python
build_flags =
  -D SHELLMINATOR_BUFF_LEN=70
  -D SHELLMINATOR_BUFF_DIM=70
  -D COMMANDER_MAX_COMMAND_SIZE=70
  -D WCLI_MAX_CMDS=7 ; your custom commands count plus one
```

Be careful with the last flag `WCLI_MAX_CMDS` when you are adding more custom commands to your CLI, it should be n+1. Also you can check the [plaformio.ini](platformio.ini) file in this repo to see the configuration for the main example.

## PlatformIO install

You able to install this library with pio pkg command:

`pio pkg install --library "hpsaturn/ESP32 Wifi CLI @^0.3.2"`

Or add it in your ini file. Also you can compile here the examples with a simple `pio run` over root of this repo.

## Arduino IDE requirements

Because Arduino IDE is very bad to resolve dependencies, you need first download and install the next libraries dependencies:

- [Shellminator](https://github.com/hpsaturn/Shellminator.git)
- [Commander-API](https://github.com/hpsaturn/Commander-API.git)

After that install ESP32 Wifi CLI library from this repo. Download it on [releases](https://github.com/hpsaturn/esp32-wifi-cli/releases).

## Changelog

### v0.3.x

![ESP32 nmcli CanAirIO demo](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/esp32_wifi_cli_canairio_demo.gif)

### v0.2.x

![ESP32 Wifi CLI Blink demo](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/esp32_wifi_cli_blink.gif)

![ESP32 Wifi CLI Demo](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/esp32_wifi_cli_demo.gif)

## Projects

The next projects are using `esp32-wifi-cli` library:

### Crypto panel

[![Crypto panel](https://user-images.githubusercontent.com/423856/219856278-1b3013fd-0a04-4464-8947-5a3cb874c843.jpg)](https://youtu.be/oyav6SvN870)

### GPS via Virtual UART port

[![screenshot20221102_231819](https://user-images.githubusercontent.com/423856/199613436-ef607d92-e06d-44ef-8d0f-0e99e49bf481.jpg)](https://hpsaturn.com/virtual-serial-port/)

### Basic T-Display S3 clock

[![ESP32S3 Clock T-Display](https://raw.githubusercontent.com/hpsaturn/esp32-s3-clock/master/pics/preview.jpg)
](https://github.com/hpsaturn/esp32-s3-clock#readme)

### CanAirIO - Air quality network

[![CanAirIO Community](https://raw.githubusercontent.com/kike-canaries/canairio_firmware/master/images/canairio_collage_community.jpg)](https://github.com/kike-canaries/canairio_firmware?tab=readme-ov-file#readme)

### ICENav - ESP32 Based GPS Navigator

[![ICENav v3](https://raw.githubusercontent.com/hpsaturn/esp32-wifi-cli/master/images/icenav_preview.png)](https://github.com/jgauchia/IceNav-v3/?tab=readme-ov-file#readme)

## Credits

- Thanks to [@jgauchia](https://github.com/jgauchia) to use and test this library in [IceNav project](https://github.com/jgauchia/IceNav-v3?tab=readme-ov-file#readme)
- IceNav project: Thanks to @ for integrate and test it
- v0.3.x Extended and using [Shellminator](https://www.shellminator.org/html/index.html) by [Daniel Hajnal](https://github.com/dani007200964)
- v0.2.x Extended from [SerialTerminal](https://github.com/miko007/SerialTerminal) by [Michael Ochmann](https://github.com/miko007)
