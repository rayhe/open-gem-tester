# Probe Construction Guide

The thermal probe is the single most important component of the OpenGemTester. Its construction quality directly determines measurement accuracy and repeatability. Take your time here.

## Overview

The probe is a small copper rod with an embedded NTC thermistor near the tip and a heater resistor on the body. When heated and pressed against a gemstone, the thermistor measures how fast heat flows out of the copper and into the stone. That rate of heat loss is what we use to identify the material.

## Materials Needed

| Item | Specification | Source | Approx. Cost |
|------|--------------|--------|-------------|
| Copper rod | 2mm diameter, solid copper welding/brazing rod | Hardware store / Amazon | $5 for a pack |
| NTC thermistor | 100K ohm, B=3950, glass bead type, ~1.5mm diameter | Amazon / AliExpress (search "100K NTC 3950 glass bead") | $3 for 10-pack |
| Heater resistor | 33 ohm, 1/2W or 1W, metal film, axial leads | Amazon / Mouser / Digikey | $1 for assortment |
| Thermal epoxy | Arctic Silver Thermal Adhesive or JB Weld (standard, NOT thermal compound) | Amazon | $7 |
| Magnet wire | 30-32 AWG enamel-coated copper wire | Amazon (search "magnet wire 30 AWG") | $5 for a spool |
| Heat shrink tubing | 3mm diameter, black | Amazon | $3 for assortment |
| Solder | Standard rosin-core 60/40 or lead-free | Any electronics supplier | (you have this) |

## Tools Needed

- Pin vise or small drill press with 1.5mm drill bit
- Soldering iron with fine tip
- Wire strippers
- Fine-grit sandpaper (400+ grit)
- Small file (needle file)
- Flush cutters
- Multimeter (for testing)
- Third-hand / helping hands tool (very helpful)
- Heat gun or lighter (for heat shrink)

## Step-by-Step Construction

### Step 1: Cut the Copper Rod

Cut a 15mm length of 2mm copper rod using flush cutters or a small pipe cutter. Deburr the cut end with sandpaper.

**Why 15mm?** Long enough to have thermal mass for the heater, short enough to keep the probe compact. The exposed tip will be about 3mm; the rest is inside the heat shrink and enclosure.

### Step 2: Drill the Thermistor Hole

Using a pin vise with a 1.5mm drill bit, drill a hole into the **end** of the copper rod (axially), approximately 3mm deep. This hole will hold the NTC thermistor bead.

Tips:
- Clamp the rod securely. A helping-hands tool works well.
- Drill slowly. Copper is soft and the bit can grab.
- Clear chips frequently by withdrawing the bit.
- Test-fit the thermistor. The glass bead (~1.5mm) should slip in with a little clearance for epoxy.

If drilling axially is too difficult, you can drill a radial hole about 2mm from the tip instead. This is easier but slightly less optimal since the thermistor is not exactly at the contact surface.

### Step 3: Prepare the Thermistor

The NTC thermistor has two thin wire leads. You need to:

1. Test it with a multimeter in resistance mode. At room temperature (~25C), it should read approximately 100K ohm.
2. Carefully bend the leads so they run parallel along the copper rod body.
3. If the leads are too short (less than ~20mm), solder extensions using 30 AWG magnet wire. Strip the enamel coating from ~3mm at each end using sandpaper or a flame.

### Step 4: Install the Thermistor

1. Mix a small amount of thermal epoxy (Arctic Silver Thermal Adhesive works well, or JB Weld original).
2. Apply a thin coating of epoxy inside the drilled hole using a toothpick.
3. Gently insert the thermistor glass bead into the hole, leads running back along the rod.
4. Add a small fillet of epoxy around the hole entrance to seal it.
5. Let cure per the epoxy instructions (Arctic Silver: 5 min working time, 24h full cure).

**Critical:** Do NOT use thermal paste/compound (like Arctic MX-4). You need actual adhesive that sets hard. Thermal compound will ooze out.

### Step 5: Install the Heater Resistor

The 33 ohm resistor goes on the rod body, about 8mm back from the tip.

1. Bend one lead of the 33 ohm resistor tightly around the copper rod body.
2. Solder the lead to the copper rod for good thermal contact. Use plenty of flux.
3. The other lead extends away for wiring.
4. The resistor body should be in firm contact with the copper rod.

Alternatively, wrap a few turns of nichrome wire around the rod body (if you have nichrome from a broken toaster or vape coil). Aim for approximately 33 ohms total resistance.

### Step 6: Wire Extensions

Solder 30 AWG magnet wire extensions to:
- Both thermistor leads (if not already extended)
- Both heater resistor leads

Route all four wires along the rod body, keeping them neat. You can use a tiny dab of superglue or a wrap of thin Kapton tape to hold them in place.

Label or color-code the wires:
- Thermistor pair (goes to NTC voltage divider circuit, ADS1115 AIN0)
- Heater pair (goes to heater MOSFET drain and 5V rail)

### Step 7: Apply Heat Shrink

Slide a ~12mm piece of 3mm heat-shrink tubing over the rod body, covering everything except the final ~3mm of the tip. Shrink it with a heat gun or lighter (careful not to melt the thermistor wires).

The exposed copper tip is your contact surface. This is what touches the gemstone.

### Step 8: Finish the Tip

File and sand the exposed tip to a slight dome shape using fine sandpaper (400+ grit). You want:
- A smooth, slightly convex surface
- Consistent contact area when pressed against a flat stone
- No sharp edges that could scratch gems

Polish with 1000-grit if you want a mirror finish (purely aesthetic, but it reduces thermal interface resistance slightly).

## Testing the Completed Probe

### Thermistor Check
1. Measure resistance between the thermistor wires with a multimeter.
2. At room temperature (~25C), you should read approximately 100K ohm (80K to 120K is acceptable).
3. Pinch the copper tip between your fingers. The resistance should drop noticeably within a few seconds as your body heat warms the tip. This confirms the thermistor is in good thermal contact with the copper.

### Heater Check
1. Measure resistance between the heater wires. Should read approximately 33 ohm.
2. Briefly apply 5V across the heater wires (from a USB power bank or bench supply). The copper tip should get noticeably warm within 10 to 15 seconds. **Do not leave power on for more than 30 seconds during testing** or you risk overheating the epoxy.

### Insulation Check
1. Verify no continuity between thermistor wires and heater wires.
2. Verify no continuity between any wire and the copper rod body (unless you intentionally grounded it).

## Probe Geometry and Performance

The probe's thermal time constant depends on:

- **Tip diameter**: 2mm is a good balance. Thinner (1mm) responds faster but has less heat reservoir. Thicker (3mm) is more stable but slower and harder to position on small stones.
- **Thermistor depth**: Closer to the contact surface = faster response. 2mm from the end is optimal.
- **Heater placement**: 8mm back from the tip distributes heat into the body without overheating the thermistor directly.
- **Contact force**: Press firmly but not hard. Consistent pressure matters more than absolute pressure. Aim for ~1N (the weight of a 100g mass).

## Troubleshooting

| Problem | Likely Cause | Fix |
|---------|-------------|-----|
| Thermistor reads open circuit | Broken thermistor lead or bad solder joint | Reflow solder joints, check wire continuity |
| Thermistor reads ~0 ohm | Shorted leads inside the epoxy | Rebuild probe, ensure leads do not touch each other or the copper |
| Heater does not warm up | Bad solder joint to copper rod | Reflow the connection, ensure good thermal contact |
| Readings are noisy | Poor thermistor-to-copper thermal contact | Rebuild with more thermal epoxy, ensure bead is fully embedded |
| Readings drift | Ambient temperature changing, or heater bleed-through | Wait longer between heater-off and measurement, check ambient sensor |
| All stones read the same | Probe has too much thermal mass (tip too large) | Use thinner copper rod or drill deeper thermistor hole |

## Advanced: Multi-Tip Probes

If you want to test different stone sizes, consider building multiple probe tips:
- **Fine tip (1mm copper)**: For small mounted stones and earrings
- **Standard tip (2mm copper)**: General purpose, works for most loose stones
- **Wide tip (3mm copper)**: For large flat surfaces like watch crystals

Design the wiring so tips are interchangeable (use small 2-pin connectors like JST-PH).

## Photos

*TODO: Add photos of each construction step. Contributions welcome!*
