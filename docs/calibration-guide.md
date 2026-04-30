# Calibration Guide

This guide walks you through the OpenGemTester self-calibration procedure. Calibration maps your specific probe's thermal time constants to known thermal conductivity values, compensating for variations in probe geometry, thermistor placement, and solder mass.

---

## Table of Contents

1. [Why Calibration Matters](#why-calibration-matters)
2. [What You Need](#what-you-need)
3. [Before You Begin](#before-you-begin)
4. [Step 1: Air Baseline](#step-1-air-baseline)
5. [Step 2: Glass Reference](#step-2-glass-reference)
6. [Step 3: Optional Diamond Point](#step-3-optional-diamond-point)
7. [How the Mapping Works](#how-the-mapping-works)
8. [Verifying Your Calibration](#verifying-your-calibration)
9. [When to Recalibrate](#when-to-recalibrate)
10. [Troubleshooting Calibration Issues](#troubleshooting-calibration-issues)

---

## Why Calibration Matters

Every OpenGemTester probe is slightly different. The exact amount of copper, the placement of the thermistor, the solder joints, and the insulation all affect how the probe stores and releases heat. These variations change the time constant (tau) measured for any given material.

Without calibration, a tau value is just a number. Calibration gives that number meaning by establishing the relationship between tau and thermal conductivity for *your specific probe*. Think of it like zeroing a kitchen scale before weighing ingredients.

### What Calibration Does

- Establishes a **baseline** (air = nearly infinite thermal resistance, slowest possible decay)
- Measures tau on a **known reference material** (glass, k = ~1.0 W/mK)
- Optionally adds a **high-conductivity reference** (diamond, k = ~2200 W/mK)
- Fits a mathematical relationship between tau and k that accounts for all probe-specific factors

After calibration, the device can convert any measured tau directly to a thermal conductivity value in W/mK.

---

## What You Need

### Required

- **A piece of window glass** (soda-lime glass): This is your primary calibration reference. Any flat piece of ordinary window glass works. It should be at least 10 mm x 10 mm and at least 3 mm thick. A microscope slide is ideal. Do not use tempered glass, Pyrex, or specialty glass, as these have different thermal conductivities.

### Optional but Recommended

- **A known diamond**: If you have a diamond that you are confident is genuine (tested by a jeweler or purchased from a reputable source), it provides an excellent high-conductivity calibration point. Even a small diamond (0.1 ct or larger) works.

- **A known sapphire**: Provides a mid-range calibration point (~35 W/mK). Natural or synthetic sapphire both work, as they have the same crystal structure and thermal conductivity.

### Environment

- Room temperature between 18 and 28 degrees C (65 to 82 degrees F)
- Stable temperature (avoid direct sunlight, heater vents, or drafty areas)
- Clean, dry work surface

---

## Before You Begin

1. **Power on the device** and let it stabilize for at least 2 minutes. The DS18B20 temperature sensor needs time to reach equilibrium with the ambient air.

2. **Clean the probe tip** with isopropyl alcohol and a lint-free cloth. Any contamination on the tip will affect readings.

3. **Clean your reference materials** (glass, diamond, etc.) with isopropyl alcohol. Fingerprint oils act as thermal insulation.

4. **Enter calibration mode** by holding the button for 3 seconds at the main screen. The OLED will display "CALIBRATION MODE" and guide you through each step.

---

## Step 1: Air Baseline

### What This Measures

With the probe heated and held in open air (not touching anything), heat dissipates very slowly through convection and radiation. This gives us the **maximum possible tau** for the probe, representing near-zero thermal conductivity at the tip.

### Procedure

1. The device heats the probe to the target temperature (displayed on screen).
2. When prompted, **hold the probe in open air**, pointing away from your body and any surfaces. Do not blow on it.
3. The device records the cooling curve in air for approximately 10 seconds.
4. The screen displays the measured tau_air value and "Air baseline OK" if the measurement is valid.

### What Can Go Wrong

- **Tau too short**: You may have been holding the probe near a surface, or there was a draft. Move to a calmer location and retry.
- **Noisy curve**: Electrical interference or a loose thermistor connection. Check your wiring.

---

## Step 2: Glass Reference

### What This Measures

Touching the heated probe to glass gives us a tau value for a material with a well-known thermal conductivity of approximately 1.0 W/mK. This is our primary calibration point.

### Procedure

1. The device heats the probe again.
2. When the screen displays "Touch probe to GLASS," **firmly but gently press the probe tip against the flat surface of the glass**.
3. Hold steady for approximately 5 seconds. Do not move or rock the probe.
4. The device records the cooling curve and fits an exponential decay.
5. The screen displays the measured tau_glass value, the fit quality (R-squared), and "Glass reference OK" if valid.

### Tips for a Good Measurement

- Use a flat area of the glass, not an edge.
- Apply consistent, moderate pressure. Do not press so hard that you flex the glass or slip.
- Keep your fingers away from the contact point. Body heat conducted through the glass can affect the measurement.
- If the glass is thin, place it on a wooden or plastic surface (not metal, which would act as a heat sink).

### Expected Values

Tau for glass will be significantly shorter than tau for air, but much longer than tau for diamond. If your glass tau is close to your air tau, something is wrong (bad contact, contaminated surface, or the "glass" is actually a different material).

---

## Step 3: Optional Diamond Point

### What This Measures

A known diamond provides a high-conductivity calibration point (~2200 W/mK). This dramatically improves accuracy in the high-conductivity range where diamond and moissanite reside.

### Procedure

1. The device heats the probe.
2. When prompted, **touch the probe to the table facet (flat top) of the diamond**.
3. Hold steady for approximately 3 seconds. Diamond cools the probe very quickly, so the useful data is captured fast.
4. The device records and fits the curve.
5. The screen displays tau_diamond and "Diamond reference OK."

### Why This Is Optional

Two calibration points (air + glass) are sufficient to establish a usable mapping. The air point anchors the low-conductivity end, and glass provides a known reference. The device assumes the tau-to-k relationship follows an inverse power law and can interpolate and extrapolate from two points.

However, adding a third point at the high-conductivity end:
- Validates the power-law assumption across the full range
- Reduces extrapolation error for diamond and moissanite identification
- Improves the fit of the B exponent (see below)

If you do not have a known diamond, the device will still work. It will just have slightly less certainty when distinguishing diamond from moissanite, and the confidence percentage for high-conductivity identifications may be lower.

---

## How the Mapping Works

### The Relationship Between tau and k

For a given probe, tau is inversely related to the thermal conductivity of the material being tested:

```
tau is proportional to 1 / k
```

This is because higher thermal conductivity means heat flows out of the probe faster, reducing the time constant.

In practice, the exact relationship is:

```
k = A / (tau ^ B)
```

Where:
- **k** is the thermal conductivity in W/mK
- **tau** is the measured time constant in seconds
- **A** is a probe-specific constant (depends on thermal mass, contact geometry, etc.)
- **B** is an exponent, ideally close to 1.0 but may vary due to probe geometry effects

### With Two Calibration Points (Air + Glass)

The air baseline provides a constraint (k approaches 0 as tau approaches tau_air), and the glass measurement provides a concrete (tau, k) pair:

```
Point 1: tau_air, k_air approximately 0.025 W/mK (convective loss only)
Point 2: tau_glass, k_glass = 1.0 W/mK
```

With two points, we solve for A and B.

### With Three Calibration Points (Air + Glass + Diamond)

```
Point 1: tau_air, k_air approximately 0.025 W/mK
Point 2: tau_glass, k_glass = 1.0 W/mK
Point 3: tau_diamond, k_diamond = 2200 W/mK
```

Three points allow us to fit A and B with a residual check. If the three points are not well-described by a single power law, the device warns you and suggests recalibrating.

### Storage

Calibration constants (A, B, tau_air, and the raw calibration data points) are stored in the ESP32's non-volatile storage (NVS). They persist across power cycles and firmware updates.

---

## Verifying Your Calibration

After calibration, test against your reference materials to verify accuracy:

1. **Test the glass again** in normal measurement mode. The device should report a thermal conductivity near 1.0 W/mK and identify it as "Glass" with high confidence.

2. **Test the diamond** (if available). The device should report a very high thermal conductivity and identify it as "Diamond."

3. **Test anything else you have** with a known identity: a sapphire, a CZ, a piece of quartz, etc. Check that the readings are plausible.

### Acceptable Accuracy

- Glass: 0.8 to 1.3 W/mK (within +/- 30%)
- Diamond: 1500 to 3000 W/mK (within +/- 35% due to contact variability at high k)
- Sapphire: 25 to 50 W/mK (within +/- 40%)

These ranges may seem wide, but remember that the goal is *identification*, not precision metrology. The materials we care about are separated by factors of 10x or more, so even 40% accuracy is sufficient for correct identification.

---

## When to Recalibrate

Recalibrate if any of the following occur:

- **Probe rebuild or repair**: Any change to the probe tip, thermistor, solder, or insulation changes the thermal mass and contact geometry.
- **Thermistor replacement**: Different thermistors have slightly different thermal characteristics even if they are the same part number.
- **Consistently inaccurate readings**: If known materials are being misidentified, recalibrating may fix probe drift.
- **Firmware update that changes ADC settings**: If sampling rate, gain, or timing parameters change, recalibration aligns the new data pipeline with known references.
- **Major ambient temperature change**: If you calibrated in a 20 degree C room and now operate in a 35 degree C environment, ambient compensation handles most of the difference, but recalibrating at the new temperature eliminates any residual offset.

### How Often?

For most users, calibrating once after building the device is sufficient. The calibration constants are physical properties of your probe and do not drift unless the probe physically changes.

If you use the tester professionally (gemological work, store counter testing), consider recalibrating monthly as a quality control measure.

---

## Troubleshooting Calibration Issues

### "Air baseline failed"

- **Cause**: Tau too short or curve fit too noisy.
- **Fix**: Ensure the probe is in open, still air. Move away from fans, vents, and surfaces. Wait for the probe to fully heat before starting.

### "Glass measurement failed"

- **Cause**: Poor contact, contaminated surface, or wrong material.
- **Fix**: Clean both probe and glass. Use flat, smooth glass at least 3 mm thick. Apply firm, steady pressure. Ensure you are using soda-lime glass, not Pyrex or specialty glass.

### "Calibration fit error too high"

- **Cause**: The three calibration points do not form a consistent power-law relationship. Usually caused by a bad measurement on one of the reference materials.
- **Fix**: Repeat the calibration from the beginning. Pay extra attention to cleanliness and contact quality.

### "Diamond tau too close to air tau"

- **Cause**: Poor contact with the diamond. The probe did not make thermal contact.
- **Fix**: Ensure the diamond is large enough (at least ~2 mm across the table facet). Press firmly. Clean both surfaces. If using a mounted diamond, test on the table facet where the probe can make flat contact.

### Readings drift over time

- **Cause**: Possible thermistor degradation, solder joint cracking, or probe insulation damage.
- **Fix**: Inspect the probe assembly. Reflow solder joints if needed. Recalibrate after repairs.

---

## Summary

| Step | Material | Purpose | Required? |
|---|---|---|---|
| 1 | Air | Establish maximum tau (baseline) | Yes |
| 2 | Window glass | Known low-k reference (1.0 W/mK) | Yes |
| 3 | Diamond | Known high-k reference (2200 W/mK) | No (but recommended) |

After calibration, test known materials to verify. Recalibrate if the probe changes or readings drift. Store calibration data in NVS.

Your OpenGemTester is now ready to identify stones with precision that surpasses commercial testers costing 10x more.
