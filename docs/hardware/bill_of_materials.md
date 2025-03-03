# Bill of Materials for Steel Owl Statue Project

## Core Electronics Components

| No. | Item | Description | Quantity | Price (JPY) | Total (JPY) | Purpose |
|-----|------|-------------|----------|-------------|-------------|---------|
| 1 | M5Stamp Timer/Power (M5STACK-S005) | Low-power power control module with RTC wake-up | 4 | 1,651 | 6,604 | Power management module |
| 2 | M5Stamp C3U (ESP32-C3) | 32-bit RISC-V MCU with Wi-Fi connectivity | 1 | - | - | Main controller |
| 3 | MAX98357A Amplifier | I2S 3W Class D Amplifier | 3 | 1,516 | 4,548 | Speaker amplifier |
| 4 | 28mm Speaker | Small form factor speaker | 3 | 1,499 | 4,497 | Audio output |
| 5 | Mini Vibration Motor | DC 3V-6V, 32mA, 28x12mm | 2 | 849 | 1,698 | Haptic feedback |
| 6 | Magnetic Pogo Pin Connector (4-pin) | Male + Female waterproof connectors | 4 | 750 | 3,000 | Head-body connection |
| 7 | WS2812B LED Ring (8 LEDs) | 5V addressable RGB LED rings | 4 | 980 | 3,920 | Owl eyes illumination |
| 8 | SPDT Slide Switch | 2 Position 3 Terminal | 1 | 760 | 760 | Power on/off |
| 9 | LiPo Battery (3.7V 1000mAh) | Rechargeable battery with JST connector | 2 | 1,699 | 3,398 | Power source |
| 10 | USB Type-C Female Socket | 4P adapter with PCB | 1 | 680 | 680 | Charging port |



## Wiring & Connectors

| No. | Item | Description | Quantity | Price (JPY) | Total (JPY) | Purpose |
|-----|------|-------------|----------|-------------|-------------|---------|
| 12 | 28AWG Silicone Wire Kit | 7 colors set, 13M/color, 300V | 1 | 2,099 | 2,099 | Internal connections |

## Assembly Tools & Materials

| No. | Item | Description | Quantity | Price (JPY) | Total (JPY) | Purpose |
|-----|------|-------------|----------|-------------|-------------|---------|
| 13 | Glue Gun with Pen Tip | Precision hot glue application | 1 | 5,682 | 5,682 | Component assembly |
| 14 | Transparent Glue Sticks | High temperature replacement | 1 | 900 | 900 | Adhesive material |



## Additional Components (Not in Original List)

| No. | Item | Description | Quantity | Price (JPY) | Notes |
|-----|------|-------------|----------|-------------|-------|
| 11 | Servo Motor | Small servo for nodding motion | 1 | - | For head movement |
| 12 | 2N2222 NPN Transistor | For driving vibration motor | 2 | - | Motor control |
| 13 | Resistors Kit | 1kΩ and 120kΩ values | 1 | - | For voltage dividers and base resistors |
| 14 | Capacitors Kit | 0.1μF, 1μF, 100μF, 470μF | 1 | - | Power filtering |
| 15 | Protection Diode | For motor back-EMF protection | 2 | - | Motor protection |
| 16 | Push Rod with Ball Joints | For head movement mechanism | 1 | - | Motion transfer |
| 17 | Nylon/PTFE Bushings | For smooth pivot motion | 2 | - | Reduce friction |
|
## Steel Owl Statue Specifications

### Overall Dimensions
- Total Height: 200mm
- Head Diameter: 100mm
- Body Top Diameter: 75mm

### Head Section
- Material: Steel with bronze/copper-colored finish
- Spherical shape with sculpted facial features
- Large circular eye cutouts with WS2812B LED rings
- Curved beak protrusion
- Magnetic attachment point at the base
- Ear-like protrusions on sides
- Nodding motion capability (15° forward, 10° back)

### Body Section
- Material: Steel with bronze/copper-colored finish
- Bowl-shaped design
- Decorative base with radiating support legs
- Magnetic attachment point for head connection
- Black interior finish
- Contains all electronic components

## Electrical Specifications

### Power System
- Input: 3.7V LiPo battery (1000mAh)
- Charging: Via USB Type-C port
- Output: Dual voltage (3.3V and 5V)
- Power Management: StampTimerPower module
- Battery Protection: Over-discharge protection circuit
- Estimated Runtime: TBD based on usage patterns

### Control System
- Microcontroller: M5Stamp C3U (ESP32-C3)
- CPU: 32-bit RISC-V single-core processor @ 160MHz
- Memory: 400KB SRAM, 4MB Flash
- Connectivity: Wi-Fi 802.11 b/g/n
- GPIOs Used: 8 (LED, I2S, motor control, servo, sensing)



**Notes:**
1. Some components are purchased in bulk or as kits with extras for redundancy
2. Additional mechanical components for the head motion system need to be sourced separately
3. Prices are in Japanese Yen (JPY) as of March 2025
4. M5Stamp C3U is not listed in the original BOM but is required as per the system diagram