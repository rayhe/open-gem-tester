# 🔬 OpenGemTester - Open Source Precision Gemstone Tester

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform: ESP32-S3](https://img.shields.io/badge/Platform-ESP32--S3-orange.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![PlatformIO](https://img.shields.io/badge/Build-PlatformIO-brightgreen.svg)](https://platformio.org/)

**A ~$30 open-source gemstone tester that outperforms $300 commercial units.**

---


## What Is This?

OpenGemTester is an open-source, ESP32-based gemstone identification tool that uses **thermal decay curve analysis** and **electrical conductivity testing** to identify gemstones with high accuracy. It can distinguish diamond from moissanite, sapphire from glass, and a dozen other materials, all for about $30 in parts.

### Why Does This Exist?

The $12 "Diamond Selector II" testers on Amazon use a single thermal threshold and a row of LEDs. They cannot tell diamond from moissanite. The Presidium Multi Tester III costs $350, adds electrical testing, and still gets poor reviews because it relies on coarse LED bar graphs and manual calibration dials.

We can do better. An ESP32, a 16-bit ADC, and actual curve fitting give us more precision than any commercial handheld tester on the market. And because it is open source, anyone can build one, improve it, or adapt it.

---

## How It Works

### Thermal Decay Analysis

A small copper probe with an embedded thermistor is heated to a known temperature above ambient. When you touch the probe to a gemstone, heat flows from the probe into the stone. How *fast* that heat flows depends on the stone's thermal conductivity.

Unlike cheap testers that just check whether the temperature dropped past a single threshold, OpenGemTester **samples the entire cooling curve at 475 samples per second** using a 16-bit ADC. It then fits an exponential decay model to hundreds of data points, extracting a precise time constant (tau) that maps directly to thermal conductivity. This gives us a continuous, high-resolution measurement instead of a binary yes/no.

### Electrical Conductivity

Thermal testing alone cannot reliably separate diamond from moissanite, because both are excellent thermal conductors. However, they differ dramatically in electrical conductivity:

- **Diamond** (Type IIa, the most common type in jewelry) is a near-perfect electrical insulator.
- **Moissanite** (silicon carbide) is a wide-bandgap semiconductor with measurable conductivity.

By applying a small voltage across the probe contacts and measuring resistance, OpenGemTester definitively separates these two materials.

### Ambient Temperature Compensation

A DS18B20 digital temperature sensor continuously monitors ambient temperature. The firmware automatically adjusts all thermal calculations, so readings remain accurate whether you are testing in a cold workshop or a warm showroom.

---

## Comparison vs. Commercial Testers

| Feature | Diamond Selector II ($12) | Presidium Gem Tester II ($130) | Presidium Multi Tester III ($350) | **OpenGemTester (~$30)** |
|---|---|---|---|---|
| Test method | Single thermal threshold | Thermal with LED scale | Thermal + electrical | **Thermal curve fit + electrical** |
| Distinguishes diamond from moissanite | No | No | Yes | **Yes** |
| Display | 9 LED bar | 9 LED bar | 9 LED bar + buzzer tones | **128x64 OLED with text** |
| Material identification | "Diamond" or not | General range | Diamond / moissanite / simulant | **Specific material name + confidence %** |
| Ambient compensation | No | Manual calibration dial | Manual calibration dial | **Automatic (digital sensor)** |
| ADC resolution | ~8-bit equivalent | ~10-bit equivalent | ~10-bit equivalent | **16-bit (ADS1115)** |
| Connectivity | None | None | None | **BLE + USB serial** |
| Open source | No | No | No | **Yes (MIT license)** |
| Calibration | None | Manual dial | Manual dial | **Guided self-calibration** |
| Data logging | No | No | No | **Yes (BLE + serial)** |

---

## Features

- **Thermal decay curve fitting** with 16-bit ADC sampling at 475 SPS
- **Dual-mode testing**: thermal conductivity + electrical conductivity
- **Automatic ambient temperature compensation** via DS18B20 digital sensor
- **128x64 OLED display** showing material name, confidence percentage, thermal conductivity, and resistance
- **Guided self-calibration** using air and common glass (no factory calibration needed)
- **BLE connectivity** for phone apps, data logging, and home automation integration
- **USB serial output** for real-time curve data and debugging
- **Open source hardware and firmware** under the MIT license
- **3D-printable enclosure** with ergonomic pen-style form factor

---

## Materials It Can Identify

| Material | Thermal Conductivity (W/mK) | Electrical Test | Identification Confidence |
|---|---|---|---|
| Diamond | ~2200 | Insulator | High |
| Moissanite (SiC) | ~490 | Semiconductor | High |
| Sapphire (Al2O3) | ~35 | Insulator | High |
| Ruby (Al2O3 + Cr) | ~35 | Insulator | High |
| Emerald (Be3Al2Si6O18) | ~1.4 | Insulator | Medium-High |
| Cubic Zirconia (ZrO2) | ~2.0 | Insulator | High |
| Glass (soda-lime) | ~1.0 | Insulator | High |
| Quartz (SiO2) | ~1.4 to 12 | Insulator | Medium |
| Topaz (Al2SiO4) | ~11 | Insulator | Medium-High |
| Spinel (MgAl2O4) | ~15 | Insulator | Medium-High |
| YAG (Y3Al5O12) | ~11 | Insulator | Medium-High |
| Metals (Cu, Al, Fe, etc.) | 50 to 400+ | Conductor | High (flagged as metal) |

> **Note:** Confidence depends on stone size, surface quality, and calibration. Small stones (under ~0.05 ct) may produce lower confidence due to limited thermal mass.

---

## Quick Start

### Parts Needed

| Component | Approximate Cost |
|---|---|
| ESP32-S3 DevKitC-1 | ~$10 |
| ADS1115 16-bit ADC breakout | ~$10 |
| SSD1306 0.96" OLED (I2C) | ~$6 |
| DS18B20 digital temperature sensor | ~$5 |
| Custom copper probe with NTC 100K thermistor | ~$5 in parts |
| MOSFETs, resistors, button, buzzer | ~$5 |
| **Total** | **~$30 to $40** |

### Build and Flash

1. **Assemble the hardware** following the schematic in [`hardware/`](hardware/).
2. **Clone the repository:**
   ```bash
   git clone https://github.com/rayhe/open-gem-tester.git
   cd open-gem-tester
   ```
3. **Build and flash the firmware** (requires [PlatformIO](https://platformio.org/)):
   ```bash
   cd firmware
   pio run -t upload
   ```
4. **Calibrate** the device following the [Calibration Guide](docs/calibration-guide.md).
5. **Test a stone** by pressing the button, waiting for the probe to heat, and touching it to a gemstone.

---

## Hardware

Full schematics, PCB layout files, and a bill of materials are in the [`hardware/`](hardware/) directory. The design uses through-hole components and common breakout boards, so no custom PCB fabrication is required for a prototype build. A KiCad project is included for those who want to make a proper PCB.

See [`hardware/README.md`](hardware/README.md) for details.

---

## Firmware

The firmware is written in C++ for the ESP32-S3 using the Arduino framework and PlatformIO. It handles ADC sampling, curve fitting, display output, BLE communication, and the calibration routine.

### Building with PlatformIO

```bash
cd firmware
pio run              # Build
pio run -t upload    # Build and flash
pio device monitor   # Serial monitor (115200 baud)
```

See [`firmware/README.md`](firmware/README.md) for architecture details, configuration options, and the curve-fitting algorithm.

---

## Enclosure

A 3D-printable enclosure in pen-style form factor is provided in the [`enclosure/`](enclosure/) directory. STL files are ready to print, and the original Fusion 360 / STEP source files are included for modification.

See [`enclosure/README.md`](enclosure/README.md) for printing recommendations and assembly instructions.

---

## Calibration

OpenGemTester uses a guided self-calibration routine that requires only a piece of window glass and optionally a known diamond or sapphire. The calibration maps your specific probe's thermal time constants to known thermal conductivity values, compensating for variations in probe geometry and thermistor placement.

See the full [Calibration Guide](docs/calibration-guide.md) for step-by-step instructions.

---

## BLE Integration

The ESP32-S3's built-in Bluetooth Low Energy radio broadcasts test results as a BLE characteristic. You can:

- **Log results** to a phone app for record-keeping
- **Stream raw curve data** to a computer for analysis
- **Integrate with home automation** (e.g., Home Assistant) for batch testing workflows
- **Build custom dashboards** using the published BLE service UUID and characteristic format

The BLE protocol is documented in [`docs/ble-protocol.md`](docs/ble-protocol.md).

---

## Documentation

- [Theory of Operation](docs/theory.md) - Physics of thermal and electrical gemstone testing
- [Calibration Guide](docs/calibration-guide.md) - Step-by-step calibration procedure
- [BLE Protocol](docs/ble-protocol.md) - Bluetooth Low Energy service specification
- [Troubleshooting](docs/troubleshooting.md) - Common issues and solutions

---

## Contributing

Contributions are welcome! Whether it is a bug fix, new material profile, hardware improvement, or documentation update, feel free to open an issue or submit a pull request.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes (`git commit -am 'Add your feature'`)
4. Push to the branch (`git push origin feature/your-feature`)
5. Open a Pull Request

Please keep code clean, document your changes, and add tests where applicable.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## Acknowledgments

- **JerryRigEverything** for the videos that inspired this project and demonstrated the limitations of commercial testers
- The gemological research community, particularly the body of work on thermal probe methods for gem identification (see Hoover & Hoover, 1982, "Thermal conductivity probe for gem identification")
- The ESP32 and Arduino open-source communities for the excellent tooling and libraries
- Everyone who contributed to the ADS1115, SSD1306, and DS18B20 Arduino libraries

---

*Built with curiosity, a soldering iron, and a healthy skepticism of $12 Amazon gadgets.*
