# OpenGemTester - Bill of Materials

> Open-source ESP32-based gemstone/diamond tester using thermal decay curve analysis and electrical conductivity.

## Key Components

| # | Component | Specs | Qty | Est. Price | Source | Alternatives |
|---|-----------|-------|-----|-----------|--------|-------------|
| 1 | ESP32-S3 DevKitC-1 | USB-C, N16R8 preferred (16MB flash, 8MB PSRAM) | 1 | $8-10 | [Amazon B0B6HT7V7M](https://amazon.com/dp/B0B6HT7V7M), [Adafruit #5364](https://www.adafruit.com/product/5364) | Any ESP32-S3 DevKit with USB-C; Seeed XIAO ESP32-S3 works but pinout differs |
| 2 | ADS1115 16-bit ADC breakout | I2C, 4-channel, default addr 0x48 | 1 | $4-6 | [Adafruit #1085](https://www.adafruit.com/product/1085), [Amazon B07VPFLSMH](https://amazon.com/dp/B07VPFLSMH) | ADS1015 (12-bit) works but lower resolution; any ADS1115 breakout board is fine |
| 3 | SSD1306 OLED display | 0.96", 128x64, I2C, addr 0x3C | 1 | $3-5 | [Amazon B09C5K91H7](https://amazon.com/dp/B09C5K91H7), [Adafruit #326](https://www.adafruit.com/product/326) | Any SSD1306 0.96" I2C OLED; 1.3" SH1110 works with code changes |
| 4 | DS18B20 temp sensor | Waterproof, digital, 1-Wire | 1 | $3-4 | [Amazon B012C597T0](https://amazon.com/dp/B012C597T0), [SparkFun SEN-11050](https://www.sparkfun.com/products/11050) | Non-waterproof TO-92 version works fine; any DS18B20 variant |
| 5 | NTC thermistor (probe) | 100K ohm, B=3950, glass bead, ~1.5mm dia | 1 | $1-2 | [Amazon B07WFHSR4M](https://amazon.com/dp/B07WFHSR4M) (pack of 5-10), [Mouser 871-B57540G0104J](https://www.mouser.com/ProductDetail/871-B57540G0104J) | Must be glass bead type for small size; 100K/3950 spec is important for calibration accuracy |
| 6 | 33 ohm 1/2W resistor | Metal film, heater element | 1 | $0.10 | [Mouser 603-MFR-25FBF52-33R](https://www.mouser.com/ProductDetail/603-MFR-25FBF52-33R), [DigiKey 33QBK-ND](https://www.digikey.com/en/products/detail/33QBK-ND) | 27 to 47 ohm range works; 1/2W minimum rating required; 1W preferred for durability |
| 7 | IRLZ44N N-channel MOSFET | Logic-level gate, TO-220 | 2 | $1-2 | [Amazon B0B937XNMC](https://amazon.com/dp/B0B937XNMC) (pack of 5-10), [Mouser 942-IRLZ44NPBF](https://www.mouser.com/ProductDetail/942-IRLZ44NPBF) | IRL540N, IRLZ34N, or any logic-level N-ch MOSFET rated for 3.3V gate drive |
| 8 | 100K ohm 1% resistor | Metal film, NTC voltage divider reference | 1 | $0.10 | [DigiKey CF14JT100K](https://www.digikey.com/en/products/detail/CF14JT100K), [Mouser 603-MFR-25FBF52-100K](https://www.mouser.com/ProductDetail/603-MFR-25FBF52-100K) | Must be 1% tolerance for measurement accuracy; 0.1% even better |
| 9 | 10M ohm resistor | 1/4W, for conductivity measurement | 1 | $0.10 | [DigiKey 10MQBK-ND](https://www.digikey.com/en/products/detail/10MQBK-ND), [Mouser 603-MFR-25FBF5210M0](https://www.mouser.com/ProductDetail/603-MFR-25FBF5210M0) | 5.6M to 15M works; value mainly sets sensitivity range |
| 10 | 1K ohm resistors | 1/4W, MOSFET gate resistors | 2 | $0.10 | [DigiKey CF14JT1K00](https://www.digikey.com/en/products/detail/CF14JT1K00) | 470 ohm to 2.2K works fine for gate drive |
| 11 | 4.7K ohm resistor | 1/4W, DS18B20 pullup | 1 | $0.05 | [DigiKey CF14JT4K70](https://www.digikey.com/en/products/detail/CF14JT4K70) | 4.7K is standard for 1-Wire; 3.3K to 10K generally works |
| 12 | 6mm tactile push button | Through-hole, momentary | 1 | $0.10 | [Amazon B01MRP025V](https://amazon.com/dp/B01MRP025V) (assorted pack), [SparkFun COM-00097](https://www.sparkfun.com/products/97) | Any momentary push button; panel-mount type also works for the enclosure |
| 13 | Passive piezo buzzer | 5V, through-hole | 1 | $1 | [Amazon B07RGDDYMP](https://amazon.com/dp/B07RGDDYMP), [Adafruit #160](https://www.adafruit.com/product/160) | Active buzzer works (fixed tone only); must be 3.3V or 5V compatible |
| 14 | WS2812B NeoPixel | Single, through-hole or breakout (optional) | 1 | $1 | [Adafruit #1734](https://www.adafruit.com/product/1734), [Amazon B088BTYJH6](https://amazon.com/dp/B088BTYJH6) | Optional - many ESP32-S3 DevKitC boards have one built in on GPIO 48 |

## Probe Materials

| # | Component | Specs | Qty | Est. Price | Source | Alternatives |
|---|-----------|-------|-----|-----------|--------|-------------|
| 15 | Copper rod | 2mm diameter, cut to ~15mm | 1 | $3-5 | [Amazon B07W6TWHWV](https://amazon.com/dp/B07W6TWHWV) (copper welding rod, 36"), hardware store copper rod | Brass rod works but has lower thermal conductivity; 1.5mm to 2.5mm diameter OK |
| 16 | Ground plate | Copper or brass, ~25x25mm | 1 | $2-3 | Cut from copper sheet [Amazon B0812HJ49V](https://amazon.com/dp/B0812HJ49V), or use a copper PCB blank | Any conductive flat surface; a penny filed flat works in a pinch |
| 17 | Thermal epoxy | Small tube, 2-part | 1 | $5-8 | [Amazon Arctic Silver Thermal Adhesive](https://amazon.com/dp/B0087X725S), JB Weld | Must be thermally conductive; standard epoxy works but gives slower thermal response |
| 18 | Enamel-coated magnet wire | 30-34 AWG, ~30cm needed | 1 | $3-5 | [Amazon B07FMPX13X](https://amazon.com/dp/B07FMPX13X) (spool), [SparkFun PRT-11363](https://www.sparkfun.com/products/11363) | Thin insulated hookup wire (30AWG) works; key is small diameter to fit in probe body |
| 19 | Heat shrink tubing | 3mm and 5mm diameter, assorted | 1 | $2-3 | [Amazon B084GDLSCK](https://amazon.com/dp/B084GDLSCK) (assorted kit) | Electrical tape works in a pinch but heat shrink is much more durable |

## Wiring and Prototyping

| # | Component | Specs | Qty | Est. Price | Source | Notes |
|---|-----------|-------|-----|-----------|--------|-------|
| 20 | Dupont jumper wires | M-M and M-F, assorted | 1 set | $3-5 | [Amazon B01EV70C78](https://amazon.com/dp/B01EV70C78) | You probably already have these |
| 21 | Half-size breadboard | 400 tie points | 1 | $2-3 | [Amazon B07DL13RZH](https://amazon.com/dp/B07DL13RZH) | You probably already have this |
| 22 | Perfboard (optional) | 5x7cm or 7x9cm, for permanent build | 1 | $1-2 | [Amazon B07NM68FXK](https://amazon.com/dp/B07NM68FXK) | Only needed if moving beyond breadboard prototype |
| 23 | USB-C cable | For power and programming | 1 | $0 | You almost certainly have one | Any USB-C data cable (not charge-only) |
| 24 | Small hookup wire | 22-26 AWG, assorted colors | 1 set | $3-5 | [Amazon B07TX6BX47](https://amazon.com/dp/B07TX6BX47) | You probably already have this |

## Cost Summary

| Category | Estimated Cost |
|----------|---------------|
| MCU + Sensors + Display (items 1-4) | $18 to $25 |
| Passive components (items 5-14) | $4 to $8 |
| Probe materials (items 15-19) | $15 to $24 |
| Wiring and prototyping (items 20-24) | $9 to $15 |
| **Total (buying everything new)** | **$46 to $72** |
| **If you already have breadboard, wires, USB cable** | **$37 to $57** |

## Notes

- **Resistor packs**: If you don't have a resistor kit, consider a 1/4W assortment kit (~$10, Amazon B08FHPJ5G8) that covers most values you need.
- **Bulk savings**: The NTC thermistors, MOSFETs, and buttons usually come in multi-packs. Great for spares or building multiple units.
- **ADS1115 vs ADS1015**: The ADS1115 (16-bit) is strongly recommended. The ADS1015 (12-bit) works but reduces measurement precision, which matters for curve fitting.
- **ESP32-S3 variant**: The DevKitC-1 with N16R8 is ideal. N8R2 or N8R8 variants also work. Avoid boards without USB-C (older micro-USB versions exist).
- **Power**: The entire circuit runs from USB power (5V bus, 3.3V regulator on-board). No external power supply needed. The heater draws ~150mA at 5V through the 33 ohm resistor, well within USB spec.
- **3D printing**: Enclosure STL files are in the `enclosure/` directory. Prints fine in PLA or PETG, no supports needed for most parts.
