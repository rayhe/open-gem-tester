# OpenGemTester - Probe Construction Guide

> The probe is the most critical component. Take your time building it. A well-made probe gives repeatable, accurate measurements. A sloppy probe gives noise and frustration.

## Overview

The probe consists of a copper rod with an embedded NTC thermistor near the tip and a heater resistor wrapped around the body. The copper rod provides excellent thermal coupling to the gemstone. When heated and pressed against a stone, heat flows from the rod into the stone at a rate determined by the stone's thermal conductivity. The NTC measures the resulting temperature decay curve.

```
  Cross-section (not to scale):
  
  |<---------- 15mm ---------->|
  
  +============================+
  |    heat shrink tubing       |  <-- insulation over body
  |  +------------------------+|
  |  | [33R heater wrapped]   ||
  |  |  around copper rod     ||
  |  +----+-------------------+|
  |       |   copper rod core  |====> tip (exposed ~3mm)
  |  +----+-------------------+|      filed to slight dome
  |  | [NTC bead in hole]     ||
  |  +------------------------+|
  +============================+
  |<-8mm->|              |<2mm>|
  heater position    NTC position
  
  Wires exit from the back end (left side):
  - 2 magnet wires for NTC thermistor
  - 2 wires for heater resistor
  - 1 wire for copper tip (conductivity)
```

---

## Materials Needed

| Material | Specification | Quantity | Notes |
|----------|-------------|----------|-------|
| Copper rod | 2mm diameter, minimum 20mm length | 1 | Cut from copper welding rod (hardware store, Amazon). Get extra for practice. |
| NTC thermistor | 100K, B=3950, glass bead, ~1.5mm dia | 1 | Must be glass bead type for small form factor. Larger disk/chip types will not fit. |
| Heater resistor | 33 ohm, 1/2W, metal film | 1 | Axial through-hole. Trim leads to ~5mm each. |
| Thermal epoxy | 2-part, thermally conductive | Small amount | Arctic Silver thermal adhesive, or JB Weld. Standard 5-minute epoxy works but is thermally slower. |
| Magnet wire | 30-34 AWG, enamel coated | ~30cm | For extending NTC thermistor leads through the probe body |
| Heat shrink tubing | 3mm ID (for rod body), 5mm ID (over heater area) | ~20cm total | Must shrink tight. Dual-wall adhesive-lined heat shrink is ideal. |
| Hookup wire | 22-26 AWG, stranded | ~40cm | For heater leads and conductivity connection |
| Solder | Standard rosin-core, 60/40 or lead-free | Small amount | For electrical connections |

---

## Tools Needed

| Tool | Purpose | Notes |
|------|---------|-------|
| Pin vise or Dremel with 1.5mm drill bit | Drilling hole for NTC in copper rod | A pin vise is safer and more controlled |
| Soldering iron | Soldering heater leads, magnet wire connections | Fine tip preferred |
| Small files (needle files) | Shaping the tip, cleaning drill hole | A set of jeweler's files is ideal |
| 220-grit sandpaper | Sanding tip to dome shape | |
| Wire strippers | Stripping magnet wire ends | Or use sandpaper to remove enamel |
| Flush cutters | Cutting rod to length, trimming leads | |
| Helping hands / third hand tool | Holding rod during assembly | Almost essential for this build |
| Heat gun or lighter | Shrinking heat shrink tubing | A lighter works, just keep it moving |
| Small clamp or vise | Holding rod for drilling | Wrap rod in tape to prevent marring |
| Magnifying glass or loupe | Inspecting thermistor placement | Optional but very helpful |
| Multimeter | Testing resistance values | Essential |
| Calipers (optional) | Measuring drill depth, rod length | Helpful for precision |

---

## Step-by-Step Construction

### Step 1: Cut the Copper Rod

1. Mark 15mm on the copper rod
2. Score with a file, then snap or cut with flush cutters
3. File both ends flat and square
4. Lightly sand the entire surface with 220-grit to clean off oxidation
5. Wipe with isopropyl alcohol

**Result**: A clean 15mm copper rod, 2mm diameter, flat on both ends.

### Step 2: Drill the NTC Hole

This is the trickiest step. You are drilling a 1.5mm hole in a 2mm rod. Go slow.

1. Secure the rod in a small vise or clamp (wrap with tape to protect the surface)
2. Mark a spot **2mm from one end** (this end will be the tip)
3. Using a pin vise with a **1.5mm drill bit**, drill perpendicular to the rod axis
4. Drill approximately **3mm deep** (through the center and slightly past it)
5. Go very slowly. Copper is soft but the rod is thin. Pressing too hard will bend it.
6. Clean out copper shavings with compressed air or a thin wire

**Check**: The NTC glass bead (~1.5mm diameter) should fit snugly into the hole. It should sit at or just below the rod surface.

**If you mess up**: Cut a new piece and try again. This is why you bought extra copper rod.

### Step 3: Prepare the NTC Thermistor

1. Examine the NTC thermistor. It should be a small glass bead (~1.5mm) with two thin wire leads
2. Measure the resistance with a multimeter: should read approximately **100K ohm at 25C** (room temperature)
3. If the leads are too thick to fit in the hole, trim them to ~3mm beyond the bead
4. Solder ~15cm of **magnet wire** to each NTC lead (use a magnifying glass)
5. Insulate each solder joint with a tiny piece of heat shrink or a dab of epoxy
6. Test resistance again through the magnet wire extensions: still ~100K

**Important**: Note which magnet wire goes to which NTC lead. Mark them (different colored wire, or a small knot in one).

### Step 4: Install the NTC in the Copper Rod

1. Mix a small amount of **thermal epoxy** (follow the product's mixing instructions)
2. Apply a thin layer of epoxy inside the drilled hole using a toothpick
3. Carefully insert the NTC glass bead into the hole, leads pointing toward the back of the rod
4. The bead should sit flush with or slightly recessed below the rod surface
5. Route the magnet wire leads along the rod body toward the back end
6. Apply a tiny bit more epoxy over the bead to seal it in place
7. Let the epoxy cure fully per the manufacturer's instructions (typically 2 to 24 hours depending on type)

**Critical**: The NTC must have good thermal contact with the copper rod. Air gaps between the bead and the copper reduce measurement accuracy. Thermal epoxy fills these gaps.

**Do NOT use**: Super glue (cyanoacrylate) -- it is thermally insulating and becomes brittle with heat cycling.

### Step 5: Attach the Heater Resistor

1. Trim the **33 ohm resistor** leads to ~5mm on each side
2. Position the resistor body around the copper rod at approximately **8mm from the tip**
3. Wrap one lead tightly around the rod, making 1 to 2 turns
4. Wrap the other lead around the rod on the opposite side
5. Solder the leads to the rod (use flux, copper can be stubborn to wet)
6. Alternatively, if soldering to the rod is difficult:
   - Wrap the resistor leads tight and secure with fine copper wire
   - Use a dab of thermal epoxy to bond
7. Solder ~10cm of **hookup wire** (22-26 AWG) to each resistor lead for connection back to the circuit

**Thermal coupling matters**: The resistor must have good thermal contact with the copper rod. It heats the rod, which heats the tip, which heats the stone.

### Step 6: Attach the Conductivity Wire

1. Solder a ~15cm piece of **hookup wire** directly to the copper rod, near the back end
2. This wire connects to MOSFET #2 source in the circuit
3. The copper rod tip itself is the conductivity electrode
4. Make sure this solder joint is solid; it carries microamps but a bad connection adds noise

### Step 7: Apply Heat Shrink Insulation

1. Cut a piece of **5mm heat shrink** long enough to cover the heater resistor area (~15mm)
2. Slide it over the heater area
3. Cut a piece of **3mm heat shrink** to cover the rod body from behind the heater to the back end
4. Thread all wires (2 NTC magnet wires, 2 heater hookup wires, 1 conductivity wire) out the back
5. **Leave the tip exposed**: approximately **3mm of bare copper** should protrude from the front of the heat shrink
6. Shrink the tubing with a heat gun or lighter (keep the flame moving to avoid scorching)
7. The heat shrink should grip the rod firmly

```
  Final probe (side view):
  
  ====[heat shrink]============[heat shrink]====||>  tip (3mm exposed copper)
       ^                ^                        ^
       |                |                        |
  wires exit       heater area             NTC location
  (back end)       (5mm shrink)          (inside rod)
```

### Step 8: Shape the Tip

1. File the exposed copper tip to a **slight dome shape** (not a sharp point)
2. Sand smooth with 220-grit, then 400-grit if available
3. A rounded tip provides consistent contact area when pressed against stones
4. Too sharp a point concentrates contact and gives variable readings
5. Too flat requires excessive pressure and risks scratching softer stones

**Ideal tip profile**: A gentle dome, like the end of a ballpoint pen.

---

## Testing the Completed Probe

### Test 1: NTC Resistance

1. Set multimeter to 200K ohm range
2. Measure between the two NTC magnet wires
3. At room temperature (~25C): should read **80K to 120K** (nominal 100K)
4. Pinch the probe tip between your fingers: resistance should drop noticeably within 10 seconds
5. If reading open circuit: NTC lead is broken (likely at the solder joint to magnet wire)
6. If reading very low (under 10K): possible short between NTC leads through the copper rod

### Test 2: Heater Resistance

1. Measure between the two heater hookup wires
2. Should read approximately **33 ohm** (30 to 36 ohm is acceptable)
3. If open circuit: a solder joint failed
4. If very low (under 5 ohm): heater lead is shorting to the copper rod

### Test 3: Insulation Check

1. Measure between each NTC wire and the copper rod conductivity wire
2. Should be **open circuit** (infinite resistance) or very high (over 10M)
3. If low resistance: epoxy insulation failed, or a lead is touching the rod
4. Measure between each heater wire and the NTC wires
5. Should also be open circuit or very high
6. If shorts exist: reapply heat shrink or epoxy to isolate

### Test 4: Heater Function

1. Connect heater wires to a 5V supply through a switch or jumper
2. Current draw should be approximately 150mA (measure with multimeter in series)
3. The probe tip should feel noticeably warm within 3 to 5 seconds
4. After 10 seconds, it should be clearly warm to the touch but not painfully hot
5. Disconnect power; the tip should cool to ambient within 15 to 20 seconds
6. If it does not heat: check heater resistance again; verify connections

### Test 5: Thermal Response

1. Connect the NTC to the circuit (or just monitor resistance with a multimeter)
2. Heat the probe for 5 seconds
3. Touch the tip to a piece of glass or metal
4. The NTC resistance should change rapidly as heat transfers into the material
5. Glass (low thermal conductivity): slow resistance change
6. Metal (high thermal conductivity): fast resistance change
7. This differential response is exactly what the firmware uses to identify gemstones

---

## Troubleshooting

### Problem: Noisy or jumpy NTC readings
- **Cause**: Poor thermal contact between NTC bead and copper rod
- **Fix**: Reapply thermal epoxy, ensure no air gaps
- **Cause**: Long unshielded magnet wire picking up noise
- **Fix**: Twist the two NTC wires together, keep them short, route away from heater wires

### Problem: NTC reads open circuit intermittently
- **Cause**: Cracked solder joint on magnet wire extension
- **Fix**: Resolder the joint; add a strain relief (hot glue at the joint)

### Problem: Heater takes a long time to warm the tip
- **Cause**: Poor thermal contact between resistor and copper rod
- **Fix**: Ensure resistor leads are tight against the rod; reapply thermal epoxy
- **Cause**: Heat shrink creating insulation between heater and rod
- **Fix**: The resistor leads should contact the copper directly; heat shrink goes over the top

### Problem: Conductivity readings are always zero
- **Cause**: Oxidation on the copper tip
- **Fix**: Lightly sand the tip with fine sandpaper before testing
- **Cause**: Poor contact with the ground plate
- **Fix**: Ensure stone is firmly on the plate and probe presses against the stone's top surface

### Problem: Readings vary between tests on the same stone
- **Cause**: Inconsistent contact pressure
- **Fix**: Practice applying consistent, moderate pressure; the domed tip helps
- **Cause**: Tip not reaching full preheat temperature
- **Fix**: Ensure heater runs for the full preheat duration before touching the stone
- **Cause**: Ambient temperature changing between tests
- **Fix**: The DS18B20 compensates for this, but rapid ambient changes (AC vent, sunlight) cause drift; let the system stabilize

---

## Tips for Better Performance

1. **Preheat consistency**: Always heat for the same duration before touching a stone. The firmware handles this automatically, but if testing manually, use a timer.

2. **Contact angle**: Hold the probe perpendicular to the stone surface. Angled contact changes the thermal coupling area.

3. **Clean the tip**: Wipe the tip with a dry cloth between tests. Oils from your skin affect thermal contact.

4. **Let the probe cool**: Wait for the probe to return to ambient temperature between tests. The DS18B20 reading helps verify this.

5. **Mounting stones**: For small stones, a dab of thermal paste on the ground plate can improve thermal and electrical contact with the plate. Wipe it off the stone when done.

6. **Spare probes**: Build two or three probes. The construction process has a learning curve, and having a spare saves frustration if one fails.

7. **Copper oxidation**: Over time, the exposed copper tip will oxidize (darken). This slightly reduces thermal conductivity. Periodically sand the tip back to bright copper with 400-grit sandpaper.

8. **Storage**: Keep the probe in a dry place. Moisture accelerates copper oxidation. A small zip-lock bag with a desiccant packet works.
