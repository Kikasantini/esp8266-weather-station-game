# ESP8266 Connected Weather Station and Number Guessing Game

An academic connected-object project built around an ESP8266. It combines a BME280 weather sensor, an OLED display, a local web server, physical controls, LEDs, and a buzzer. A number-guessing game provides an additional interactive mode.

## Features

- Displays temperature, pressure, estimated altitude, humidity, and network time on an SSD1306 OLED.
- Serves local pages for the sensor readings, game score, and game status.
- Uses three LEDs to show a randomly selected number from 1 to 7 in binary.
- Lets the player cycle through guesses with a single button click and confirm with a long press.
- Increases the score after a correct answer; a wrong answer ends the round and sounds the buzzer.

## Hardware

- ESP8266 development board (the sketch uses NodeMCU-style `D` pin names)
- BME280 temperature, pressure, and humidity sensor
- SSD1306 128×64 I²C OLED display
- Push button, three LEDs with current-limiting resistors, and buzzer

## Pin connections

| Component | ESP8266 pin | Notes |
| --- | --- | --- |
| Push button | D3 | Button library input |
| Buzzer | D5 | Tone output |
| LED 1 | D0 | Number display bit |
| LED 2 | D6 | Number display bit |
| LED 3 | D7 | Number display bit |
| OLED and BME280 SDA | D2 | Shared I²C data |
| OLED and BME280 SCL | D1 | Shared I²C clock |
| OLED and BME280 power | 3.3 V | Follow the breakout board ratings |

## Arduino libraries

Install these libraries through the Arduino Library Manager: `NTPClient`, `PinButton`, `Adafruit GFX Library`, `Adafruit SSD1306`, `Adafruit Unified Sensor`, and `Adafruit BME280 Library`. Select an ESP8266 board package compatible with the board you use.

## Setup

1. Open `firmware/ConnectedWeatherStation/ConnectedWeatherStation.ino` in the Arduino IDE.
2. In the same folder, copy `MyWifi.example.h` to `MyWifi.h`.
3. Edit `MyWifi.h` with the Wi-Fi network credentials for your own test network. The real `MyWifi.h` is ignored by Git.
4. Confirm the board and port in the Arduino IDE, then compile and upload the sketch.
5. Open the serial monitor to find the ESP8266's local IP address. Visit that address from a device on the same network to use the web pages.

## Button controls

- **Single click:** refresh the sensor display, or change the current guess while playing.
- **Double click:** show or hide the device's local IP address when outside the game.
- **Long press:** start a game or submit the current guess.

## Project context

Created for the Connected Objects Programming course at Institut Grasset in 2022. The uploaded course handout is not included in this repository; the README summarizes the project and its setup. The project is an academic prototype and expects the hardware and libraries listed above.

## Project media

- `media/hardware-prototype.jpeg`
- `media/web-interface-home.jpg`
- `media/wiring-diagram.png`
- `media/breadboard-layout.png`
