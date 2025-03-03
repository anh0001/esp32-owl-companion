# Owl Statue Electronics Integration Plan - Dual Voltage System

## Core Components

1. **Main Controller:** M5Stamp C3U (ESP32-C3)
   - Operating voltage: 3.3V (from StampTimerPower)
   - Input current: ~50mA typical
   - 14 available GPIOs
   - Built-in USB Type-C interface
   - Built-in SK6812 RGB LED
   - Programming button (G9)

2. **Power Management:** StampTimerPower Module
   - Battery charging via TP4057
   - 3.3V DCDC output for M5Stamp C3U
   - 5V boost circuit for peripherals
   - Built-in RTC (BM8563)
   - USB Type-C charging port

## Mechanical Design

### Head Mechanism
1. **Nodding Motion:**
   - Pivot point: Central axis through head sides
   - Movement range: 15 degrees forward, 10 degrees back
   - Push rod connection point: Lower back of head
   - Mounting: Brass pivot pins or hidden hinges
   - Material considerations:
     - Low-friction pivot points
     - Nylon/PTFE bushings for smooth motion
     - Lightweight head construction for easy movement

2. **Actuation System:**
   - Location: Servo mounted in upper body section
   - Push rod: 2mm diameter steel or carbon fiber rod
   - Rod ends: Ball joints or flexible connectors
   - Motion ratio: ~2:1 (servo:head movement)
   - Mechanical advantage: Lever arm designed for smooth motion
   - End stops: Soft rubber bumpers to limit travel

## Voltage Rails & Power Distribution

### 3.3V Rail (from StampTimerPower)
1. **Components:**
   - M5Stamp C3U
   - Logic circuits
   - Current requirement: ~100mA total

2. **Distribution:**
   - Direct connection from StampTimerPower
   - Local decoupling: 10µF + 0.1µF
   - Separate from 5V ground plane

### 5V Rail (from StampTimerPower boost)
1. **Components:**
   - WS2812B LED rings (4x rings, 160mA each)
   - MAX98357A amplifier (100mA typical)
   - Vibration motor (32mA)
   - Servo Motor (250mA peak)
   - Total peak: ~1150mA @ 5V

2. **Distribution:**
   - Branched from StampTimerPower 5V output
   - Bulk capacitor: 2200µF (increased for servo)
   - Multiple decoupling points
   - Separate servo power filtering

## Component Integration

### Head Section: LED System Only
1. **WS2812B LED Rings:**
   - Power: 5V from StampTimerPower boost
   - Data: 3.3V from M5Stamp
   - Per-ring requirements:
     - Power: 5V @ 160mA
     - Local capacitor: 100µF
     - Data resistor: 470Ω
   - Wiring: Flexible/strain-relieved to accommodate motion

### Body Section: Motion System
1. **Servo Motor (For Nodding):**
   - Position: Upper body section
   - Mount: Rigid bracket with vibration isolation
   - Operating voltage: 5V
   - Current draw: ~250mA peak
   - PWM control: 3.3V compatible
   - Local capacitor: 470µF
   - Motion linkage:
     - Push rod with ball joints
     - Adjustable length for calibration
     - Spring tension or damping if needed

2. **Vibration Motor:**
   - Power: 5V from StampTimerPower boost
   - Control: 3.3V GPIO via MOSFET
   - MOSFET selection: Logic-level FET for 3.3V gate
   - Protection: Flyback diode
   - Mount: Lower body section, isolated

### Body Section: Audio System
1. **MAX98357A Amplifier:**
   - Power: 5V from StampTimerPower boost
   - Logic: 3.3V I2S from M5Stamp
   - Decoupling: 10µF + 0.1µF
   - Ground connected to 5V plane

2. **28mm Speaker:**
   - Direct connection to amplifier
   - Acoustic isolation maintained
   - Mounted in base section

### Head-Body Connection System
1. **Magnetic Pogo Pins (4-pin):**
   - Pin 1: 5V (LED power)
   - Pin 2: GND (Common ground)
   - Pin 3: LED Data (3.3V)
   - Pin 4: Sensing/Head detection
   - Note: Connector position must allow for nodding motion

## GPIO Assignment
```
ESP32-C3 GPIO Mapping:
GPIO2  → LED Ring Data (3.3V)         // Optimal for WS2812B
GPIO6  → I2S BCLK                     // Standard I2S pin
GPIO7  → I2S LRCLK                    // Standard I2S pin
GPIO8  → I2S DIN                      // Standard I2S pin
GPIO4  → Motor MOSFET (3.3V)          // Vibration control
GPIO18 → I2C SDA (RTC)                // Standard I2C pin
GPIO19 → I2C SCL (RTC)                // Standard I2C pin
GPIO9  → Built-in Button              // Reserved for system
GPIO5  → Head Detection               // Head presence sensing
GPIO10 → Servo Control                // PWM for servo motor
```

## Wiring Specifications
- 28AWG silicone wire throughout
- Color Coding:
  - Red: 5V power
  - Orange: 3.3V power
  - Black: Ground
  - Yellow: LED data
  - Blue: I2S
  - Green: Control signals

## Assembly Sequence

1. **Head Assembly:**
   - Install pivot points
   - Mount LED rings
   - Install power filtering components
   - Secure magnetic connector
   - Add push rod connection point
   - Balance check and adjustment

2. **Body Assembly:**
   - Mount servo in upper body
   - Install push rod with ball joints
   - Mount StampTimerPower at base
   - Install M5Stamp C3U
   - Mount speaker and amplifier
   - Install vibration motor
   - Route and secure all wiring
   - Calibrate motion range

## Testing Procedures

1. **Mechanical Tests:**
   - Smooth nodding motion
   - No binding through range
   - Proper end stop function
   - Push rod alignment
   - Head balance
   - Connector reliability during motion

2. **Electronic Tests:**
   - LED function through motion range
   - Servo control accuracy
   - Power stability during movement
   - Wiring stress test during motion
   - Head detection reliability
   - Audio quality verification
   - Vibration motor function

3. **Power Tests:**
   - Battery duration testing
   - Charging functionality
   - Voltage stability under load
   - Temperature monitoring

## Safety Features

1. **Voltage Protection:**
   - 3.3V brownout detection
   - 5V overvoltage protection
   - Battery protection circuit

2. **Current Limiting:**
   - StampTimerPower built-in limits
   - LED power monitoring
   - Servo current monitoring
   - Soft-start implementation

3. **Mechanical Safety:**
   - Servo movement limiters
   - Head motion end stops
   - Strain relief on all wiring
   - Secure servo mounting
   - Push rod safety stops

4. **Thermal Management:**
   - Power converter cooling
   - LED temperature monitoring
   - Servo temperature monitoring
   - Battery temperature sensing