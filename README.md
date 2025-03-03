# ESP32 Owl Companion

An interactive owl statue project powered by ESP32-C3, featuring motion, light, sound, and haptic feedback.

![Owl Companion Animation](docs/media/images/owl-animation.gif)

## Overview

This project combines art and technology to create an interactive steel owl statue with the following capabilities:
- Head nodding motion via servo
- LED illuminated eyes
- Sound playback (owl hooting)
- Vibration haptic feedback
- Battery powered operation

The owl is designed with a detachable head that connects via magnetic pogo pins, allowing for modular design and easy maintenance.

## Hardware Components

The project uses the following main components:
- **M5Stamp C3U**: ESP32-C3 based controller module
- **StampTimerPower**: Power management module with battery charging, RTC, and power control
- **WS2812B LED Rings**: Provides illumination for the owl's eyes
- **MAX98357A Amplifier**: Audio amplifier for the speaker
- **28mm Speaker**: For sound output
- **Vibration Motor**: For haptic feedback
- **Servo Motor**: For head nodding motion
- **LiPo Battery**: 3.7V 1000mAh for power

See the [Bill of Materials](docs/hardware/bill_of_materials.md) for a complete parts list.

## System Architecture

The system uses a dual voltage design (3.3V and 5V) with the following subsystems:
- **Power System**: Battery charging, 3.3V and 5V power rails
- **Motion System**: Servo control for head nodding
- **Light System**: WS2812B LED control
- **Sound System**: I2S audio with MAX98357A amplifier
- **Haptic System**: Vibration motor control

![System Diagram](schematics/system-diagram.svg)

## Getting Started

### Prerequisites

- Arduino IDE or PlatformIO
- ESP32 board support
- Required libraries:
  - FastLED or NeoPixel for WS2812B control
  - ESP32 I2S Audio
  - Servo library
  - ESP32 RTC libraries

### Hardware Assembly

1. Assemble the power management system
2. Install the motion control system
3. Wire the LED and audio components
4. Connect the head and body using the magnetic pogo pins
5. Install the firmware

See the [Build Plan](docs/planning/build_plan.md) for detailed assembly instructions.

### Software Setup

1. Clone this repository
```bash
git clone https://github.com/yourusername/esp32-owl-companion.git
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
- Charges via USB-C
- RTC for time-based functions
- Deep sleep capabilities for extended battery life

### Interaction Modes

The owl has several interaction modes:
- **Passive Mode**: Occasional eye blinking and head movement
- **Alert Mode**: Responds to motion or sound with more active behaviors
- **Interactive Mode**: Responds to touch or proximity

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