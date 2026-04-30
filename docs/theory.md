# Theory of Operation

This document covers the physics behind OpenGemTester's thermal and electrical conductivity measurements, and explains why our approach outperforms single-threshold commercial testers.

---

## Table of Contents

1. [Thermal Conductivity Basics](#thermal-conductivity-basics)
2. [Why Diamond Is Special](#why-diamond-is-special)
3. [Thermal Conductivity of Target Materials](#thermal-conductivity-of-target-materials)
4. [How Thermal Probes Work](#how-thermal-probes-work)
5. [The Math: Exponential Decay Model](#the-math-exponential-decay-model)
6. [Why Single-Threshold Testing Fails](#why-single-threshold-testing-fails)
7. [Why Curve Fitting Is Better](#why-curve-fitting-is-better)
8. [Electrical Conductivity Testing](#electrical-conductivity-testing)
9. [Error Sources and Mitigation](#error-sources-and-mitigation)

---

## Thermal Conductivity Basics

Thermal conductivity (k) measures how well a material transfers heat. It is expressed in watts per meter-kelvin (W/mK). A material with high thermal conductivity moves heat quickly; a material with low thermal conductivity acts as an insulator.

For gemstone identification, thermal conductivity is extremely useful because different gem materials have widely varying thermal conductivity values, and those values are intrinsic to the crystal structure. A diamond feels "cold" to the touch because it pulls heat away from your finger rapidly. Glass feels "warm" because it does not.

### Units and Scale

- **Air**: ~0.025 W/mK (excellent insulator)
- **Glass**: ~1.0 W/mK
- **Most gemstones**: 1 to 50 W/mK
- **Moissanite (SiC)**: ~490 W/mK
- **Diamond**: ~2200 W/mK (one of the highest of any natural material)
- **Copper** (for reference): ~400 W/mK

The range spans nearly five orders of magnitude from air to diamond, which gives us excellent discrimination potential if we can measure accurately.

---

## Why Diamond Is Special

Diamond's extraordinary thermal conductivity comes from its crystal structure. Each carbon atom is bonded to four neighbors in a rigid tetrahedral arrangement (sp3 hybridization). This creates an extremely stiff lattice with light atoms packed tightly together.

Heat in a crystal travels primarily through **phonons**, which are quantized lattice vibrations. The speed and efficiency of phonon transport depend on:

1. **Bond stiffness** - Stiffer bonds mean higher phonon velocities. The C-C sp3 bond is one of the stiffest in nature.
2. **Atomic mass** - Lighter atoms vibrate at higher frequencies, carrying more energy per phonon. Carbon is very light (atomic mass 12).
3. **Crystal perfection** - Defects, impurities, and grain boundaries scatter phonons. Gem-quality diamond is a nearly perfect single crystal.
4. **Isotopic purity** - Natural diamond is ~99% carbon-12, with ~1% carbon-13. Even this small isotopic variation slightly reduces conductivity. Isotopically pure C-12 diamond reaches ~3300 W/mK.

This combination of stiff bonds, light atoms, and crystal perfection makes diamond the best thermal conductor among naturally occurring materials. No other gemstone comes close.

---

## Thermal Conductivity of Target Materials

| Material | Formula | Thermal Conductivity (W/mK) | Crystal System | Physical Explanation |
|---|---|---|---|---|
| Diamond | C | ~2200 | Cubic (Fd3m) | sp3 carbon, stiff lattice, light atoms, near-perfect crystal |
| Moissanite | SiC | ~490 | Hexagonal (6mm) | Strong Si-C bonds, relatively light atoms; lower than diamond due to heavier Si and polytypic stacking |
| Sapphire / Ruby | Al2O3 | ~35 | Trigonal (R3c) | Strong Al-O bonds but heavier atoms and more complex unit cell reduce phonon mean free path |
| Spinel | MgAl2O4 | ~15 | Cubic (Fd3m) | Mixed cation site reduces phonon coherence |
| Topaz | Al2SiO4(F,OH)2 | ~11 | Orthorhombic | Complex silicate framework, hydroxyl groups scatter phonons |
| YAG | Y3Al5O12 | ~11 | Cubic (Ia3d) | Heavy yttrium atoms (mass 89) severely limit phonon velocity |
| Quartz | SiO2 | ~1.4 (amorphous) to ~12 (c-axis) | Trigonal (crystalline) | Open framework with many possible phonon-scattering modes; strongly anisotropic in crystalline form |
| Cubic Zirconia | ZrO2 (+ Y2O3) | ~2.0 | Cubic (Fm3m) | Very heavy Zr atoms (mass 91), Y2O3 stabilizer introduces point defect scattering |
| Emerald | Be3Al2Si6O18 | ~1.4 | Hexagonal | Complex ring silicate with channel voids; inclusions common |
| Glass (soda-lime) | SiO2 + Na2O + CaO | ~1.0 | Amorphous | No long-range order, massive phonon scattering |
| Metals (Cu) | Cu | ~400 | Cubic (Fm3m) | Free electrons dominate heat transport (Wiedemann-Franz law) |
| Metals (Al) | Al | ~237 | Cubic (Fm3m) | Free electron transport |
| Metals (Steel) | Fe + C | ~50 | Various | Lower electron mobility, alloy scattering |

### Key Observations

- Diamond and moissanite are in a class of their own among gemstones, with thermal conductivities 10x to 2000x higher than other gems.
- Sapphire/ruby sit in a middle range (~35 W/mK) that is clearly separable from both the diamond/moissanite group and the glass/CZ group.
- CZ, glass, emerald, and quartz (amorphous) cluster in the 1 to 2 W/mK range, making them harder to distinguish from each other by thermal testing alone. However, they are trivially separable from diamond, moissanite, and sapphire.
- Metals have high thermal conductivity but are also excellent electrical conductors, making them easy to flag.

---

## How Thermal Probes Work

### Probe Construction

The OpenGemTester probe consists of:

1. **A copper tip** (~2 mm diameter) that makes contact with the stone
2. **A heater element** (resistive wire or small ceramic heater) that raises the tip to a known temperature above ambient
3. **An NTC 100K thermistor** embedded in or bonded to the copper tip, which measures the tip temperature with high sensitivity
4. **Thermal insulation** around the probe body to minimize heat loss to paths other than the stone

### Measurement Cycle

1. **Heating phase**: The heater raises the probe tip to approximately 60 to 80 degrees C above ambient (configurable). The exact temperature is measured by the thermistor and recorded.
2. **Contact**: The user touches the heated probe tip to the stone surface.
3. **Cooling phase**: Heat flows from the copper tip, through the contact interface, and into the stone. The thermistor records the temperature drop over time.
4. **Sampling**: The ADS1115 ADC samples the thermistor voltage at 475 SPS (samples per second) with 16-bit resolution, giving us approximately 500 data points per second.
5. **Analysis**: The firmware fits an exponential decay model to the sampled data and extracts the time constant (tau).

### Why Copper?

Copper's high thermal conductivity (~400 W/mK) ensures that the temperature across the probe tip is nearly uniform. This means the thermistor reading accurately represents the temperature at the contact point. A lower-conductivity tip material would create temperature gradients within the probe, introducing measurement error.

---

## The Math: Exponential Decay Model

### The Lumped-Capacitance Model

When the heated probe contacts a stone, we model the system as a thermal RC circuit:

- **C_probe**: the thermal capacitance of the probe tip (joules per kelvin)
- **R_contact**: the thermal resistance of the probe-to-stone interface (kelvin per watt)
- **k_stone**: the thermal conductivity of the stone

The probe temperature as a function of time follows Newton's law of cooling:

```
T(t) = T_amb + (T_0 - T_amb) * exp(-t / tau)
```

Where:
- `T(t)` is the probe temperature at time t
- `T_amb` is the ambient temperature (measured by the DS18B20)
- `T_0` is the initial probe temperature at the moment of contact
- `tau` is the thermal time constant
- `exp` is the exponential function

### The Time Constant

The time constant tau is:

```
tau = C_probe / (k_stone * A_contact / d_eff)
```

Or more simply:

```
tau = C_probe * R_thermal
```

Where:
- `C_probe` is the thermal mass of the probe (depends on probe geometry and material, approximately constant for a given build)
- `k_stone` is the thermal conductivity of the stone (what we want to measure)
- `A_contact` is the effective contact area between probe and stone
- `d_eff` is the effective thermal penetration depth
- `R_thermal` is the total thermal resistance from probe to stone bulk

### Extracting tau from Data

Given N sampled data points (t_i, T_i), we transform the data:

```
y_i = ln(T_i - T_amb)
```

This transforms the exponential decay into a linear relationship:

```
y_i = ln(T_0 - T_amb) - t_i / tau
```

We then use linear least-squares regression on (t_i, y_i) to find the slope (-1/tau) and intercept. This is computationally cheap enough to run on the ESP32 in real time.

### From tau to Thermal Conductivity

Since tau is inversely proportional to k_stone (for a given probe), and the proportionality constant depends on probe-specific parameters (C_probe, A_contact, d_eff), we use calibration to establish the mapping:

```
k = A / (tau^B)
```

Where A and B are calibration constants determined during the calibration procedure. For an ideal probe, B = 1, but in practice probe geometry effects may cause B to differ slightly from unity.

---

## Why Single-Threshold Testing Fails

Commercial testers like the Diamond Selector II work roughly as follows:

1. Heat the probe
2. Touch the stone
3. After a fixed time interval, check whether the temperature has dropped below a threshold
4. If yes, call it "diamond." If no, call it "not diamond."

### The Problems

**Problem 1: Moissanite false positives.** Moissanite has a thermal conductivity of ~490 W/mK. Diamond is ~2200 W/mK. Both will cause the probe to cool rapidly, and both will cross a threshold set to distinguish diamond from glass (~1 W/mK). The threshold would need to be set extremely high to reject moissanite, at which point it would also reject many real diamonds (especially small ones with less thermal mass).

**Problem 2: No confidence measure.** A single threshold gives a binary answer with no indication of how close to the boundary the measurement was. A reading just barely above threshold and a reading far above it both produce the same "diamond" LED.

**Problem 3: Noise sensitivity.** A single data point (temperature at one specific time) is highly susceptible to noise. Electrical noise in the thermistor circuit, mechanical vibration, or inconsistent contact pressure can easily push a reading across the threshold in either direction.

**Problem 4: No material identification.** Even if the threshold correctly separates "high conductivity" from "low conductivity," it tells you nothing about what the stone actually is. A sapphire, spinel, and topaz all register as "not diamond" with no further distinction.

**Problem 5: Ambient temperature dependence.** Without compensation, the initial probe-to-ambient temperature difference varies with room temperature. A cooler room means a larger initial delta, which means faster absolute cooling rate, which can shift readings relative to the threshold. Commercial testers address this with a manual calibration dial, which the user must adjust constantly.

---

## Why Curve Fitting Is Better

Instead of checking one threshold at one time point, OpenGemTester fits the **entire cooling curve** using hundreds of data points.

### Advantages

**Precision through statistics.** Fitting an exponential to 200+ data points dramatically reduces the effect of any single noisy sample. The standard error of the regression gives us a direct measure of fit quality, which we report as a confidence percentage.

**Continuous measurement.** Instead of a binary "above/below threshold," we get a continuous tau value that maps to a continuous thermal conductivity estimate. This lets us place the stone on a scale and identify it by matching against a table of known materials.

**Robustness to contact variation.** If the probe makes uneven contact and then settles, the curve-fitting algorithm can detect and exclude the initial transient, focusing on the stable portion of the decay.

**Self-consistency check.** A good exponential fit (high R-squared) indicates that the measurement is valid. A poor fit (low R-squared) indicates a problem: bad contact, stone too small, probe issue, etc. The device can flag unreliable readings instead of displaying a false result.

**Material resolution in the mid-range.** Single-threshold testers cannot distinguish sapphire (35 W/mK) from topaz (11 W/mK) from glass (1 W/mK). They are all "not diamond." Curve fitting with 16-bit precision can resolve these differences and provide specific identification.

---

## Electrical Conductivity Testing

### Why We Need It

Thermal testing alone cannot reliably distinguish diamond from moissanite in all conditions. While their thermal conductivities differ by about 4x (2200 vs. 490 W/mK), factors like contact pressure variation, stone size, and surface coatings can cause overlap in measured tau values, especially for small stones.

Electrical conductivity provides a completely independent measurement axis that definitively separates these two materials.

### The Physics

**Diamond (Type IIa):** Most gem-quality diamonds are Type IIa, meaning they have very low nitrogen content. They are excellent electrical insulators with resistivity greater than 10^13 ohm-cm. Even Type Ia diamonds (with nitrogen) are insulators, though Type IIb diamonds (with boron) are semiconductors. Type IIb diamonds are extremely rare in jewelry and are typically blue.

**Moissanite (SiC):** Silicon carbide is a wide-bandgap semiconductor (bandgap ~3.2 eV for 4H-SiC). At room temperature with a small applied voltage (3.3V from the ESP32), moissanite shows measurable conductivity. Typical resistance across a probe contact is in the range of kilohms to megohms, depending on crystal orientation and dopant levels.

**Other gems:** Sapphire, ruby, CZ, glass, quartz, topaz, and other common gems are all electrical insulators. They register as open circuit (resistance > 10 megohms).

**Metals:** Obviously conductive. Resistance in the single-digit ohm range. The firmware flags any reading below 100 ohms as "metal detected."

### Measurement Method

1. Two small contacts on the probe tip are pressed against the stone surface.
2. A voltage divider circuit with a known high-value reference resistor (10 megohms) measures the stone's resistance.
3. The ADS1115 reads the voltage at the midpoint.
4. For an insulator (diamond, sapphire, glass), essentially no current flows and the midpoint voltage equals the supply voltage.
5. For moissanite, measurable current flows, pulling the midpoint voltage down proportionally.
6. For metals, the midpoint drops to near zero.

### Decision Logic

| Material Class | Thermal Conductivity | Electrical Resistance | Identification |
|---|---|---|---|
| Diamond | Very high (tau < threshold_1) | Very high (> 10 megohms) | Diamond |
| Moissanite | High (tau < threshold_2) | Moderate (1K to 10M ohms) | Moissanite |
| Sapphire/Ruby | Medium (tau in range) | Very high | Sapphire or Ruby (distinguished by thermal k) |
| CZ, Glass, etc. | Low (tau > threshold_3) | Very high | Identified by specific thermal k |
| Metal | Very high | Very low (< 100 ohms) | Metal (flagged) |

The combination of thermal and electrical testing creates a two-dimensional identification space that is far more discriminating than either measurement alone.

---

## Error Sources and Mitigation

### 1. Contact Pressure Variation

**Problem:** Pressing harder increases the effective contact area, which reduces thermal resistance and speeds up cooling. This shifts tau and introduces measurement variability.

**Mitigation:**
- The curve-fitting approach is partially self-correcting: a larger contact area changes the magnitude of heat flow but the shape of the exponential remains governed by material properties.
- The probe tip has a fixed geometry (2 mm hemisphere) that limits how much contact area can vary.
- The firmware excludes the first ~50 ms of data after contact to allow pressure to stabilize.
- Confidence scoring penalizes fits with unusual residual patterns that suggest changing contact conditions.

### 2. Surface Contamination

**Problem:** Oil, dirt, or coatings on the stone surface act as a thermal insulation layer, slowing heat transfer and shifting tau toward lower apparent conductivity.

**Mitigation:**
- Users are instructed to clean the stone before testing.
- The firmware can detect anomalously slow initial heat transfer (indicating a surface layer) and warn the user.
- Electrical conductivity testing is less affected by thin surface films.

### 3. Stone Size (Thermal Mass)

**Problem:** Very small stones (under ~0.05 ct or about 2 mm diameter) have limited thermal mass. The stone heats up quickly, reducing the temperature gradient and shortening the useful portion of the decay curve.

**Mitigation:**
- The 16-bit ADC resolves small temperature differences that would be invisible to 8-bit or 10-bit converters.
- The firmware reduces the initial probe temperature for small-stone mode, preserving the useful portion of the curve.
- Confidence scoring is adjusted downward for short decay curves, and the user is warned when stone size may be limiting accuracy.

### 4. Ambient Temperature Effects

**Problem:** The initial temperature difference (T_0 - T_amb) affects the absolute cooling rate and the signal-to-noise ratio. In very warm environments, the smaller initial delta means less signal.

**Mitigation:**
- The DS18B20 continuously measures T_amb, and the firmware uses the actual measured value in all calculations.
- The heater power is automatically adjusted to maintain a consistent initial temperature delta regardless of ambient conditions.
- Self-calibration is performed at the current ambient temperature, so the calibration constants inherently account for the operating conditions.

### 5. Probe Thermal Mass Uncertainty

**Problem:** The exact thermal mass of the probe depends on manufacturing tolerances (amount of solder, thermistor placement, copper geometry). This varies between builds.

**Mitigation:**
- Self-calibration directly measures the probe's behavior on known materials, absorbing all probe-specific parameters into the calibration constants A and B.
- The calibration routine explicitly accounts for this by not assuming any specific probe thermal mass value.

### 6. Thermistor Nonlinearity

**Problem:** NTC thermistors have a nonlinear resistance-to-temperature relationship. Using a simple linear approximation introduces systematic errors, especially over wide temperature ranges.

**Mitigation:**
- The firmware uses the Steinhart-Hart equation (three-parameter model) for resistance-to-temperature conversion, which is accurate to within +/- 0.02 degrees C over the operating range.
- Steinhart-Hart coefficients are determined during manufacturing calibration of the thermistor (datasheet values), or can be fitted from three known temperature points.

### 7. ADC Noise and Drift

**Problem:** Even the ADS1115 has some noise (typically 5 to 10 uV RMS at 475 SPS) and offset drift with temperature.

**Mitigation:**
- The 16-bit resolution (7.8 uV per LSB at +/- 0.256V range) means noise is confined to the lowest 1 to 2 bits.
- The curve fit averages over hundreds of samples, reducing random noise by a factor of sqrt(N).
- The ADS1115's internal PGA (programmable gain amplifier) is set to maximize resolution for the expected signal range.
- Offset calibration is performed during the air baseline measurement.

---

## Further Reading

- Hoover, D. B., & Hoover, B. (1982). "Thermal conductivity probe for gem identification." *Gems & Gemology*, 18(2), 83-90.
- Slack, G. A. (1973). "Nonmetallic crystals with high thermal conductivity." *Journal of Physics and Chemistry of Solids*, 34(2), 321-335.
- Wei, L., et al. (1993). "Thermal conductivity of isotopically modified single crystal diamond." *Physical Review Letters*, 70(24), 3764.
- Berman, R. (1976). *Thermal Conduction in Solids*. Oxford University Press.
- Harris, D. C. (1999). *Materials for Infrared Windows and Domes*. SPIE Press. (Contains extensive thermal property data for optical/gem materials.)
