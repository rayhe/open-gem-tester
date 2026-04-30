# OpenGemTester - Assembly Guide

> Step-by-step assembly instructions. Start with breadboard prototyping, verify each subsystem, then optionally move to a permanent perfboard build.

## Tools Needed

- Soldering iron (for probe construction and optional perfboard)
- Wire strippers
- Small Phillips screwdriver
- Multimeter (essential for testing)
- Breadboard (for initial prototyping)
- Flush cutters / side cutters
- Third-hand tool or PCB holder (helpful)

---

## Step 1: Parts Checklist

Before starting, lay out and verify all components. Check against the [BOM](BOM.md).

**Must have before starting:**
- [ ] ESP32-S3 DevKitC-1
- [ ] ADS1115 breakout board
- [ ] SSD1306 0.96" OLED
- [ ] DS18B20 temperature sensor
- [ ] 2x IRLZ44N MOSFETs
- [ ] Resistors: 100K 1%, 33 ohm 1/2W, 10M, 2x 1K, 4.7K
- [ ] Tactile push button
- [ ] Passive piezo buzzer
- [ ] USB-C data cable
- [ ] Breadboard and jumper wires

**Needed for probe (can build later):**
- [ ] 2mm copper rod
- [ ] NTC 100K B3950 glass bead thermistor
- [ ] Thermal epoxy
- [ ] Magnet wire (30-34 AWG)
- [ ] Heat shrink tubing

**Needed for enclosure (can print later):**
- [ ] 3D printed enclosure parts (see `enclosure/` directory)

---

## Step 2: Breadboard Layout

Place the ESP32-S3 DevKitC across the center channel of a half-size breadboard. The USB-C port should face the edge of the breadboard for easy cable access.

```
  Breadboard layout (top view):
  
  +--------------------------------------------------+
  |  [DS18B20]    [Button]   [Buzzer]                 |
  |     |            |          |                     |
  |  ---+---+--------+----------+---  row for GND     |
  |     |   |        |          |                     |
  |  +--+---+-ESP32-S3-DevKitC--+--+                  |
  |  |  L   |  [USB-C this end] |  R                  |
  |  |  e   |                   |  i                  |
  |  |  f   |                   |  g                  |
  |  |  t   |                   |  h                  |
  |  |      |                   |  t                  |
  |  +------+-------------------+--+                  |
  |     |                       |                     |
  |  [ADS1115]            [SSD1306 OLED]              |
  |                                                   |
  |  [MOSFET #1]  [MOSFET #2]                         |
  |  (heater)     (conductivity)                      |
  +--------------------------------------------------+
  
  Power rails: 3.3V on top rail, GND on bottom rail
  5V tapped from ESP32 5V pin to a separate rail section
```

**Tip**: Use red wires for 3.3V, orange for 5V, black for GND, and other colors for signal lines. This makes debugging much easier.

---

## Step 3: I2C Bus Wiring (ADS1115 + OLED)

Wire these first because they are easy to test.

### Connections

1. **ADS1115 breakout:**
   - VDD to 3.3V rail
   - GND to GND rail
   - SDA to ESP32 GPIO 8
   - SCL to ESP32 GPIO 9
   - ADDR to GND rail (sets address 0x48)
   - A0, A1, A2, A3: leave unconnected for now

2. **SSD1306 OLED:**
   - VCC to 3.3V rail
   - GND to GND rail
   - SDA to ESP32 GPIO 8 (same bus as ADS1115)
   - SCL to ESP32 GPIO 9 (same bus as ADS1115)

### Test

1. Connect USB-C cable to ESP32
2. Upload a simple I2C scanner sketch (available in Arduino IDE under Examples, Wire, i2c_scanner)
3. Open serial monitor at 115200 baud
4. You should see two devices found:
   - 0x3C (OLED)
   - 0x48 (ADS1115)
5. If not found: check wiring, verify breakout boards have power (LED indicators)

---

## Step 4: DS18B20 Ambient Sensor Wiring

### Connections

The DS18B20 waterproof version has three wires:
- **Red**: VCC to 3.3V rail
- **Black**: GND to GND rail  
- **Yellow** (or white): Data to ESP32 GPIO 4

Add the pullup resistor:
- **4.7K resistor** between the data line (GPIO 4) and 3.3V rail

### Test

1. Upload a DS18B20 test sketch (use the DallasTemperature library)
2. Serial monitor should show ambient temperature (should be reasonable, e.g. 20-25C indoors)
3. Pinch the sensor between your fingers; temperature should rise slowly
4. If reading -127C or similar: check pullup resistor, verify data pin assignment

---

## Step 5: Heater MOSFET Circuit

This drives the 33 ohm heater resistor on the probe with 5V through an IRLZ44N MOSFET.

### Connections

1. Place IRLZ44N #1 on the breadboard (Gate-Drain-Source, left to right when facing label)
2. **Gate**: Connect through a **1K resistor** to ESP32 GPIO 10
3. **Drain**: Connect to one lead of the **33 ohm 1/2W resistor**; the other lead of the 33 ohm resistor connects to the **5V rail**
4. **Source**: Connect to **GND rail**

**For initial testing, the 33 ohm resistor stands in for the probe heater. Once the probe is built, the heater resistor will be on the probe body.**

### Test

1. Upload a sketch that toggles GPIO 10 HIGH/LOW every 2 seconds
2. Measure voltage across the 33 ohm resistor:
   - GPIO 10 HIGH: Should read ~5V (MOSFET on, current flowing)
   - GPIO 10 LOW: Should read ~0V (MOSFET off)
3. The 33 ohm resistor should get noticeably warm when the MOSFET is on (0.75W dissipation)
4. If the resistor does not get warm: check MOSFET orientation, verify gate signal with multimeter

---

## Step 6: NTC Measurement Circuit

This is the precision analog circuit. Take care with wiring.

### Connections

1. **100K 1% reference resistor**: One end to 3.3V rail, other end to a junction node
2. **Junction node**: Connect to ADS1115 AIN0 input
3. **NTC thermistor** (or temporary 100K resistor for testing): One end to the junction node, other end to GND

```
  3.3V ---[100K 1%]---+--- ADS1115 AIN0
                       |
                  [NTC 100K]
                  (or test resistor)
                       |
                      GND
```

### Test

1. Upload a sketch that reads ADS1115 channel 0 in single-ended mode (gain = 1, 128 SPS)
2. With a 100K test resistor: should read approximately 1.65V (half of 3.3V)
3. With the NTC at room temperature (~25C): should also read approximately 1.65V (NTC is ~100K at 25C)
4. Warm the NTC with your finger: voltage should decrease (resistance drops as temperature rises)
5. If readings are noisy: shorten wires, check ground connections, add 100nF cap from AIN0 to GND

---

## Step 7: Electrical Conductivity Circuit

### Connections

1. Place IRLZ44N #2 on the breadboard
2. **Gate**: Connect through a **1K resistor** to ESP32 GPIO 11
3. **Source**: This will connect to the **probe tip** (for now, leave a jumper wire ready)
4. **Drain**: Connect to one end of the **10M ohm resistor**; the other end to **3.3V rail**
5. **ADS1115 AIN1**: Connect to the junction of MOSFET #2 drain and the 10M resistor

```
  3.3V ---[10M]---+--- ADS1115 AIN1
                   |
             IRLZ44N #2 Drain
                   |
             Gate --[1K]-- GPIO 11
                   |
             Source --- [wire to probe tip, eventually]
```

For the ground plate side:
- The **ground plate** connects to GND
- The stone sits between the probe tip and the ground plate

### Test

1. Turn on GPIO 11 (MOSFET on)
2. Touch the two test leads (probe side and ground plate side) together:
   - Should read near 0V at AIN1 (short circuit to ground through MOSFET)
3. Separate the leads (open circuit):
   - Should read near 3.3V at AIN1 (no current path)
4. Hold a resistor between the leads to simulate different conductivities:
   - 1M ohm: readable voltage
   - 100M ohm: very small voltage
   - Open: full 3.3V

---

## Step 8: Button and Buzzer Wiring

### Button

1. One leg of the **tactile button** to ESP32 GPIO 5
2. Other leg to GND
3. Firmware enables the internal pullup resistor on GPIO 5

### Buzzer

1. **Positive** lead of passive piezo buzzer to ESP32 GPIO 6
2. **Negative** lead (or other lead) to GND

### Test

1. Upload test sketch:
   - Read GPIO 5 with INPUT_PULLUP; serial print the state
   - Press button: should toggle from HIGH to LOW
2. Drive GPIO 6 with tone(6, 2000) for a 2kHz beep
3. If buzzer is very quiet: try driving from 5V through a transistor (the 3.3V GPIO may not drive some buzzers loudly enough)

---

## Step 9: Probe Connection

Once the probe is built (see [probe-construction.md](probe-construction.md)), connect it to the breadboard.

### Wiring the Probe

The probe has 4 wires coming out of it:
1. **NTC wire A** (from thermistor) to the junction of 100K ref resistor and ADS1115 AIN0
2. **NTC wire B** (from thermistor) to GND
3. **Heater wire A** (from 33 ohm resistor) to the 5V side (disconnect the standalone 33 ohm from Step 5 since the heater is now on the probe)
4. **Heater wire B** (from 33 ohm resistor) to MOSFET #1 drain

Additionally:
5. The **copper tip itself** connects to MOSFET #2 source (for conductivity measurements)

**This means 5 wires total from probe to circuit.** Use color-coded wires:
- Red + Black for NTC thermistor
- Orange + Brown for heater resistor
- Green for copper tip (conductivity)

### Test

1. Measure NTC resistance with multimeter: should be ~100K at room temp
2. Measure heater resistance: should be ~33 ohm
3. Check for shorts between NTC leads and the copper rod (should be open circuit if epoxy insulation is good)

---

## Step 10: Ground Plate Setup

The ground plate provides the electrical return path for conductivity measurements.

### Construction

1. Cut a piece of copper sheet to approximately 25x25mm (1" x 1")
2. Sand one face lightly with 220-grit sandpaper for good contact
3. Solder a wire to the back of the plate
4. Connect the wire to GND on the breadboard
5. Optionally mount the plate on a small piece of wood or plastic so it sits flat

### Usage

- Place the gemstone on the ground plate
- Touch the probe tip to the top of the stone
- The stone completes the circuit between probe tip and ground plate

### Tips

- Keep the plate clean; oils and dirt reduce electrical contact
- A light press of the stone onto the plate is sufficient
- For very small stones, a smaller plate or a pointed ground contact works better
- The plate also acts as a heat sink for the bottom of the stone (minor effect, but helps consistency)

---

## Step 11: System Testing

Before final assembly, test the complete system on the breadboard.

### Subsystem Verification Checklist

- [ ] **I2C**: Both ADS1115 (0x48) and OLED (0x3C) detected
- [ ] **OLED**: Displays text correctly
- [ ] **DS18B20**: Reads reasonable ambient temperature
- [ ] **ADS1115 Ch0**: NTC voltage divider reads ~1.65V at room temp
- [ ] **ADS1115 Ch1**: Conductivity circuit reads 0V when shorted, 3.3V when open
- [ ] **Heater MOSFET**: 33 ohm resistor/probe heater gets warm when GPIO 10 is HIGH
- [ ] **Conductivity MOSFET**: Switches on/off cleanly with GPIO 11
- [ ] **Button**: Reads LOW when pressed, HIGH when released
- [ ] **Buzzer**: Produces audible tones at various frequencies
- [ ] **Probe NTC**: Resistance changes when probe tip is warmed/cooled
- [ ] **Probe heater**: Tip gets warm (not hot) within a few seconds
- [ ] **Probe conductivity**: Touching tip to ground plate reads near 0V

### Full Measurement Test

1. Place a known material on the ground plate (glass, metal coin, or an actual gemstone)
2. Heat the probe for 2-3 seconds using the heater
3. Touch the probe to the material
4. Watch the ADS1115 Ch0 readings: the voltage should change as heat transfers
5. The rate of change is the thermal signature
6. Toggle MOSFET #2 and read Ch1 for conductivity
7. If all subsystems work, the firmware can do the rest

---

## Step 12: Breadboard to Perfboard (Optional)

If you want a permanent, enclosed build:

### Planning

1. Sketch your perfboard layout on paper first
2. The ESP32-S3 DevKitC uses 2.54mm pin headers, so it mounts directly on perfboard
3. Group components by circuit:
   - NTC circuit near ADS1115
   - MOSFETs near edge (for heat dissipation, though not really needed at this power)
   - Button and buzzer near their GPIO pins

### Layout Suggestion (5x7cm perfboard)

```
  +------------------------------------------+
  |  [Button]  [Buzzer]                       |
  |                                           |
  |  +---ESP32-S3 DevKitC (on headers)---+    |
  |  |   [soldered into perfboard]       |    |
  |  +-----------------------------------+    |
  |                                           |
  |  [ADS1115]  [MOSFET1] [MOSFET2]          |
  |             [resistors underneath]         |
  |                                           |
  |  [DS18B20 wire exit]   [Probe wire exit]  |
  +------------------------------------------+
```

### Soldering Tips

1. Use socket headers for the ESP32 and ADS1115 (so you can remove them if needed)
2. Solder resistors on the underside of the perfboard to save space
3. Use a ground bus along one edge; connect all GND points to it
4. Double-check I2C connections before powering on
5. Test continuity with a multimeter after soldering each section
6. Apply hot glue to secure connectors and wire exits
7. Label your wires (masking tape + marker works fine)

### Wire Management

- Bundle the probe wires and ground plate wire separately
- Use small zip ties or wire loom for a clean look
- Leave enough slack for the probe to reach stones without straining

---

## What's Next

Once the hardware is assembled and tested:
1. Flash the OpenGemTester firmware to the ESP32-S3
2. Run the calibration routine (uses known reference materials)
3. Print and assemble the enclosure (see `enclosure/README.md`)
4. Start testing gemstones!
