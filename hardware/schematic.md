# OpenGemTester - Schematic and Circuit Description

> Complete wiring reference for the ESP32-S3-based gemstone tester.

## Pin Assignment Summary

| ESP32-S3 Pin | Function | Connected To |
|-------------|----------|-------------|
| GPIO 8 | I2C SDA | ADS1115 SDA, SSD1306 SDA |
| GPIO 9 | I2C SCL | ADS1115 SCL, SSD1306 SCL |
| GPIO 10 | Heater PWM | 1K resistor to IRLZ44N #1 gate |
| GPIO 11 | Conductivity switch | 1K resistor to IRLZ44N #2 gate |
| GPIO 4 | 1-Wire data | DS18B20 data (4.7K pullup to 3.3V) |
| GPIO 5 | Button input | Tactile button to GND (internal pullup) |
| GPIO 6 | Buzzer output | Passive piezo buzzer to GND |
| 5V | Heater supply | 33 ohm heater resistor |
| 3.3V | Logic supply | NTC divider, conductivity circuit, DS18B20 VCC, I2C pullups |
| GND | Common ground | All ground connections |

## Voltage Domains

- **5V rail**: USB VBUS, heater resistor supply, buzzer (if 5V type)
- **3.3V rail**: ESP32 on-board regulator output, all logic and sensor circuits
- **I2C bus**: 3.3V logic levels; most breakout boards include pullup resistors

---

## Circuit A: NTC Thermal Measurement

This is the core measurement circuit. The NTC thermistor in the probe tip forms a voltage divider with a precision 100K reference resistor. The ADS1115 reads the midpoint voltage at 16-bit resolution.

### How It Works

When the probe tip contacts a gemstone, heat flows from the heated tip into the stone. The NTC thermistor cools down, its resistance rises, and the voltage at AIN0 changes. The rate and shape of this voltage curve reveals the stone's thermal conductivity.

### Connections

```
  3.3V
   |
   +--[100K 1% ref resistor]--+-- ADS1115 AIN0
                               |
                          [NTC 100K B3950]
                          (in probe tip)
                               |
                              GND
```

### Details

- At 25C, the NTC reads ~100K, so the divider midpoint is ~1.65V (half of 3.3V)
- As the NTC heats up, its resistance drops, voltage at AIN0 drops
- As the NTC cools (touching stone), resistance rises, voltage at AIN0 rises
- ADS1115 in single-ended mode, gain = 1x (FSR = 4.096V), 128 SPS for fast sampling
- 16-bit resolution gives ~0.125mV per LSB, plenty for curve analysis
- Use short wires from the NTC to ADS1115 to minimize noise pickup

---

## Circuit B: Heater Drive

The heater is a 33 ohm resistor wrapped around the copper probe body. An IRLZ44N MOSFET switches 5V through it under PWM control from the ESP32.

### How It Works

GPIO 10 drives the MOSFET gate through a 1K resistor. When HIGH (3.3V), the IRLZ44N turns on fully (Rds_on < 0.1 ohm at 3.3V Vgs). Current flows through the 33 ohm heater resistor, warming the probe tip. The firmware uses PWM to control temperature precisely.

### Connections

```
  5V (USB bus)
   |
   +--[33 ohm 1/2W heater resistor]--+
                                      |
                               IRLZ44N #1
                                Drain (D)
                                      |
                                Gate (G)----[1K]---- GPIO 10
                                      |
                               Source (S)
                                      |
                                     GND
```

### Details

- Heater current at 5V: I = 5V / 33 ohm = ~150mA
- Heater power: P = 5V x 150mA = ~0.75W
- This is within USB 2.0 spec (500mA total) with headroom for the rest of the circuit
- The 1K gate resistor limits inrush current and damps ringing
- PWM frequency: 1kHz is fine (not audible, fast enough for thermal control)
- The IRLZ44N is massive overkill for 150mA, but it is cheap and easy to get
- Heat sink on the MOSFET is NOT needed at this power level

---

## Circuit C: Electrical Conductivity

This circuit measures the stone's electrical conductivity by passing a tiny current through it and measuring the resulting voltage. This differentiates diamond from moissanite (moissanite is electrically conductive, diamond is not).

### How It Works

When MOSFET #2 turns on, 3.3V is applied through a 10M ohm series resistor. The probe tip contacts one side of the stone, and the ground plate contacts the other side. Any current flowing through the stone creates a voltage at AIN1. Diamond reads near zero; moissanite reads a measurable voltage.

### Connections

```
  3.3V
   |
   +--[10M ohm series resistor]--+-- ADS1115 AIN1
                                  |
                           IRLZ44N #2
                            Drain (D)
                                  |
                            Gate (G)----[1K]---- GPIO 11
                                  |
                            Source (S)
                                  |
                           [wire to probe tip]
                                  
                           (stone sits between
                            probe tip and plate)

                           [ground plate]
                                  |
                                 GND
```

### Details

- The 10M resistor limits current to microamps (safe for the stone and the user)
- At 3.3V with 10M series resistance, max current is 0.33 uA
- Conductive stones (moissanite, silicon carbide) show measurable voltage at AIN1
- Non-conductive stones (diamond, cubic zirconia, glass) read near zero
- ADS1115 gain = 16x (FSR = 0.256V) for maximum sensitivity on this channel
- The MOSFET switch prevents the conductivity circuit from interfering with thermal readings
- Important: the probe tip serves double duty (thermal contact AND electrical contact)

---

## Circuit D: I2C Bus

Both the ADS1115 ADC and the SSD1306 OLED share a single I2C bus.

### Connections

```
  ESP32 GPIO 8 (SDA) ----+---- ADS1115 SDA ----+---- SSD1306 SDA
                          |                      |
  ESP32 GPIO 9 (SCL) ----+---- ADS1115 SCL ----+---- SSD1306 SCL
                          
  3.3V ---- ADS1115 VDD          3.3V ---- SSD1306 VCC
  GND  ---- ADS1115 GND          GND  ---- SSD1306 GND
  
  ADS1115 ADDR ---- GND  (sets address to 0x48)
```

### Details

- ADS1115 default address: 0x48 (ADDR pin to GND)
- SSD1306 default address: 0x3C
- Most breakout boards include 4.7K to 10K pullup resistors on SDA/SCL
- If using bare modules without pullups, add 4.7K resistors from SDA to 3.3V and SCL to 3.3V
- I2C clock speed: 400kHz (fast mode) works for both devices
- Keep I2C wires short (under 15cm) and away from the heater power lines
- ADS1115 ALERT/RDY pin is unused (leave floating or tie to 3.3V via 10K)

---

## Circuit E: Power Distribution

```
  USB-C connector (on ESP32-S3 DevKitC)
       |
       +-- 5V (VBUS) ----+---- Heater circuit (through MOSFET, up to 150mA)
       |                  +---- ESP32 on-board 3.3V regulator input
       |                  +---- Buzzer VCC (if 5V type)
       |
       +-- GND -----------+---- Common ground for everything
       
  ESP32 3.3V pin ----+---- ADS1115 VDD
                      +---- SSD1306 VCC
                      +---- DS18B20 VCC
                      +---- NTC voltage divider top (100K ref)
                      +---- Conductivity circuit top (10M series)
                      +---- 4.7K pullup for DS18B20
```

### Current Budget

| Circuit | Typical Draw | Peak Draw |
|---------|-------------|-----------|
| ESP32-S3 (WiFi/BLE active) | 80mA | 240mA |
| ADS1115 | 0.2mA | 0.2mA |
| SSD1306 OLED | 10mA | 20mA |
| DS18B20 | 1mA | 1.5mA |
| Heater (33 ohm at 5V) | 0 (off) | 150mA |
| NeoPixel (optional, full white) | 0 | 60mA |
| Buzzer | 0 (off) | 30mA |
| **Total** | **~90mA idle** | **~500mA peak** |

Peak draw stays within USB 2.0's 500mA limit. If using a USB 3.x port or a phone charger, you have even more headroom.

---

## DS18B20 Ambient Temperature Sensor

```
  3.3V ----+---- DS18B20 VCC (red wire)
            |
            +--[4.7K pullup]--+---- DS18B20 DATA (yellow wire) ---- GPIO 4
                              
  GND ---- DS18B20 GND (black wire)
```

- Standard 1-Wire protocol, parasitic power NOT used (VCC connected)
- 4.7K pullup is required on the data line
- Reads ambient temperature for compensation (probe readings are relative to ambient)
- 12-bit resolution (0.0625C), conversion time ~750ms
- Read once per measurement cycle, not during fast ADC sampling

---

## Button and Buzzer

```
  GPIO 5 ----[tactile button]---- GND
  (internal pullup enabled in firmware)
  
  GPIO 6 ----[passive piezo buzzer]---- GND
```

- Button uses ESP32 internal pullup (~45K), reads LOW when pressed
- Buzzer driven with PWM at various frequencies for different tones
- Short beep = measurement started, rising tone = diamond detected, falling tone = not diamond

---

## Full System ASCII Schematic

```
                              +------------------+
                              |   ESP32-S3       |
                              |   DevKitC-1      |
                              |                  |
              +---[4.7K]--3.3V|  3.3V       5V  |----+----[33R 1/2W]----+
              |               |                  |    |                   |
  DS18B20 ----+------GPIO 4  |                  |    |            IRLZ44N #1 D
  (ambient)         |        |                  |    |                   |
                    |        |  GPIO 10  -------|----|--[1K]--- Gate    |
                    |        |                  |    |           Source--GND
                    |        |                  |    |
                    |   3.3V-|--[100K 1%]--+    |    +--- Buzzer(+)
                    |        |             |    |         |
                    |        |         ADS1115  |    GPIO 6---Buzzer(-)---GND
                    |        |          AIN0    |
                    |        |             |    |
                    |        |         [NTC 100K]
                    |        |         (probe)  |
                    |        |             |    |
                    |        |            GND   |
                    |        |                  |
                    |   3.3V-|--[10M]------+    |
                    |        |             |    |
                    |        |         ADS1115  |
                    |        |          AIN1    |
                    |        |             |    |
                    |        |        IRLZ44N #2 D
                    |        |             |    |
                    |        |  GPIO 11 ---|--[1K]--- Gate
                    |        |             |
                    |        |          Source--[wire to probe tip]
                    |        |                  
                    |        |           [stone on ground plate]
                    |        |                  |
                    |        |                 GND
                    |        |                  |
                    |        |  GPIO 8 (SDA)----+---- ADS1115 SDA
                    |        |                  +---- SSD1306 SDA
                    |        |                  |
                    |        |  GPIO 9 (SCL)----+---- ADS1115 SCL
                    |        |                  +---- SSD1306 SCL
                    |        |                  |
                    |        |  GPIO 5 ---[Button]--- GND
                    |        |                  |
                    |        |  GND  -----------+---- Common GND
                    |        +------------------+
```

---

## Important Notes

1. **Ground continuity**: All GND connections must be common. A poor ground connection is the #1 cause of noisy readings.
2. **Wire routing**: Keep the analog wires (NTC, conductivity) away from the heater power line and the I2C bus. Cross them at right angles if they must cross.
3. **Decoupling**: The ADS1115 and SSD1306 breakout boards typically include decoupling capacitors. If using bare chips, add 100nF ceramic caps close to each VCC pin.
4. **Probe cable**: Keep the cable from probe to board under 30cm. Longer runs pick up more noise. Use shielded cable or twisted pairs for the NTC lines if going longer.
5. **MOSFET orientation**: The IRLZ44N in TO-220 package has Gate-Drain-Source pinout (left to right, facing the label). Double-check with the datasheet.
6. **ADS1115 ADDR pin**: Must be connected to GND for address 0x48. Do not leave it floating.
