# ESP32 Owl Companion

An interactive owl statue project powered by ESP32-C3, featuring motion, light, sound, and haptic feedback.

![Owl Companion Animation](docs/media/images/owl-animation.gif)

## Overview

This project combines art and technology to create an interactive steel owl statue with the following capabilities:
- Head nodding motion via servo
- LED illuminated eyes with WS2812B rings
- Sound playback (owl hooting)
- Vibration haptic feedback
- Battery powered operation with deep sleep support
- HTTP-based remote control

The owl is designed with a detachable head that connects via magnetic pogo pins, allowing for modular design and easy maintenance. The statue features a bronze/copper-colored finish with a modern interpretative design.

## Hardware Components

The project uses the following main components:
- **M5Stamp C3U**: ESP32-C3 RISC-V MCU controller module with Wi-Fi
- **StampTimerPower**: Power management module with battery charging, RTC, and power control
- **WS2812B LED Rings**: 8-LED 5V rings for the owl's eyes
- **MAX98357A Amplifier**: I2S audio amplifier for the speaker
- **28mm Speaker**: For sound output
- **Vibration Motor**: DC 3V-6V, 32mA for haptic feedback
- **Servo Motor**: For head nodding motion (15° forward, 10° back)
- **4-pin Magnetic Pogo Connector**: For detachable head connection
- **LiPo Battery**: 3.7V 1000mAh with JST connector
- **Slide Switch**: For power control

See the [Bill of Materials](docs/hardware/bill_of_materials.md) for a complete parts list.

## Physical Design

### Owl Statue Specifications
- **Total Height**: 200mm
- **Head Diameter**: 100mm
- **Body Top Diameter**: 75mm
- **Materials**: Steel with bronze/copper-colored finish
- **Features**:
  - Detachable head via magnetic connection
  - Decorative base with radiating support legs
  - Contrasting black interior
  - Bowl-shaped body design
  - Large circular eye cutouts with LED illumination

## System Architecture

The system uses a dual voltage design (3.3V and 5V) with the following subsystems:

### Power System
- Battery charging via TP4057 on StampTimerPower
- 3.3V DCDC output for M5Stamp C3U
- 5V boost circuit for peripherals
- USB Type-C charging port
- Battery protection circuit

### Head Section
- WS2812B LED rings powered by 5V
- 4-pin magnetic pogo connector:
  - Pin 1: 5V (LED power)
  - Pin 2: GND (Common ground)
  - Pin 3: LED Data (3.3V)
  - Pin 4: Sensing/Head detection

### Body Section
- Motion system with servo motor for nodding
- Audio system with MAX98357A amplifier and 28mm speaker
- Vibration motor controlled via MOSFET
- Power management and control logic

### GPIO Assignment
```
ESP32-C3 GPIO Mapping:
GPIO2  → LED Ring Data (3.3V)
GPIO6  → I2S BCLK
GPIO7  → I2S LRCLK
GPIO8  → I2S DIN
GPIO4  → Motor MOSFET (3.3V)
GPIO18 → I2C SDA (RTC)
GPIO19 → I2C SCL (RTC)
GPIO9  → Built-in Button
GPIO5  → Head Detection
GPIO10 → Servo Control
```

![System Diagram](schematics/system-diagram.svg)

## Control System

The owl features an HTTP-based control system:

1. **Status Check Request**
   - Device polls server every 5 seconds to check if action should be triggered
   - `GET http://54.250.108.126/getConfig.php?configKey=owl_motor`

2. **Action Trigger**
   - When value is "true", the owl executes programmed behaviors for 20 seconds:
     - Nodding head movement
     - LED eye patterns
     - Sound playback sequence

3. **Flag Reset**
   - Device immediately sends request to reset the flag to false
   - `GET http://54.250.108.126/setConfig.php?configKey=owl_motor&configValue=false`

4. **External Activation**
   - Any system can remotely trigger the owl by setting the flag to "true"
   - `GET http://54.250.108.126/setConfig.php?configKey=owl_motor&configValue=true`

## Getting Started

### Prerequisites

- Arduino IDE or PlatformIO
- ESP32 board support
- Required libraries:
  - FastLED or NeoPixel for WS2812B control
  - ESP32 I2S Audio
  - Servo library
  - ESP32 RTC libraries
  - ESP32 WiFi libraries
  - ArduinoJson (for HTTP responses)

### Hardware Assembly

1. Assemble the power management system
   - Connect StampTimerPower to LiPo battery
   - Verify 3.3V and 5V outputs

2. Install the motion control system
   - Mount servo in upper body
   - Install push rod with ball joints
   - Calibrate motion range

3. Wire the LED and audio components
   - Connect WS2812B LED rings with proper filtering
   - Wire MAX98357A and speaker

4. Connect the head and body using the magnetic pogo pins
   - Ensure proper alignment
   - Test connection reliability during motion

5. Install the firmware

See the [Build Plan](docs/planning/build_plan.md) for detailed assembly instructions.

### Software Setup

1. Clone this repository
```bash
git clone https://github.com/anh0001/esp32-owl-companion.git
```

2. Open the firmware project in Arduino IDE or PlatformIO
```bash
cd esp32-owl-companion/firmware/owl_main
```

3. Configure your board settings for ESP32-C3
4. Upload the firmware to your M5Stamp C3U

## Usage

### Power Management

The owl uses the StampTimerPower module for efficient power management:
- Charges via USB-C with the TP4057 controller
- BM8563 RTC for time-based functions and wake-up
- Deep sleep capabilities for extended battery life
- Battery protection circuit prevents over-discharge

### Interaction Modes

The owl has several interaction modes:
- **Passive Mode**: Occasional eye blinking and head movement
- **Alert Mode**: Responds to motion or sound with more active behaviors
- **Interactive Mode**: Responds to touch or proximity
- **Remote Mode**: Controlled via HTTP requests

## Development

### Adding Custom Sounds

To add custom sounds:
1. Convert your audio files to 8-bit, 16KHz WAV format
2. Place them in the `docs/media/sounds` directory
3. Update the sound references in `owl_hoot.h`

### Modifying LED Patterns

LED patterns are defined in the main firmware. You can create custom patterns by:
1. Adding new pattern functions
2. Registering them in the pattern scheduler

### Power Optimization

To optimize power consumption:
1. Utilize the StampTimerPower's RTC wake-up functionality
2. Implement deep sleep between interaction periods
3. Use the voltage monitoring to protect the battery

## Testing Procedures

1. **Mechanical Tests**
   - Verify smooth nodding motion
   - Check for proper push rod alignment
   - Test head balance and connector reliability

2. **Electronic Tests**
   - LED function through motion range
   - Power stability during movement
   - Audio quality verification
   - Vibration motor function

3. **Power Tests**
   - Battery duration testing
   - Charging functionality
   - Voltage stability under load

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- M5Stack for the excellent ESP32 modules
- The ESP32 community for libraries and examples
- Contributors to this project

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request