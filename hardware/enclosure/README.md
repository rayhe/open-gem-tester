# Enclosure

3D-printable enclosure for the OpenGemTester. Designed as a handheld pen-style form factor similar to commercial diamond testers, but sized to fit an ESP32-S3 DevKitC and supporting circuitry.

## Design

- **Form factor**: Rectangular pen with rounded edges, comfortable in one hand
- **Dimensions**: Approximately 160mm x 40mm x 25mm (L x W x H)
- **Two-piece construction**: Main body + snap-fit lid
- **Material**: PLA or PETG, 0.2mm layer height, 20% infill

## Cutouts and Features

- **OLED window**: 27mm x 15mm rectangular opening on the top face
- **Probe exit**: 4mm hole on the narrow end for the copper probe tip
- **USB-C port**: Opening on the opposite narrow end for the ESP32's USB-C connector
- **Button**: 7mm hole on the top face near the probe end
- **Buzzer**: Array of small holes (sound port) on the top face
- **Ground plate wire**: 3mm hole on the side for the wire leading to the ground plate
- **Internal standoffs**: Four M2 screw posts for mounting the ESP32 board
- **Perfboard bay**: Space for a ~30mm x 50mm perfboard holding the ADS1115, MOSFETs, and passive components

## Printing

- Print body upside-down (lid surface on the bed) for best surface finish on the visible side
- No supports needed if printed in the recommended orientation
- 3 perimeters minimum for strength
- PLA is fine for normal use; use PETG if you want better heat resistance

## Ground Plate

The ground plate is a separate piece: a ~25mm x 25mm copper or brass plate mounted on a small 3D-printed base. The stone sits on this plate during testing. A wire connects it to the tester's circuit ground.

A simple printed stand for the ground plate is included in the SCAD file.

## Files

- `enclosure.scad` - OpenSCAD parametric source file
- Export STLs by rendering in OpenSCAD (F6) then exporting (F7)
