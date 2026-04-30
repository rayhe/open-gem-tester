# OpenGemTester - Enclosure Design

## Overview

The enclosure is a pen-style handheld device inspired by the familiar Diamond Selector II form factor. The probe extends from one end, the USB-C port is accessible from the other, and the OLED display faces the user through a window cutout on top.

## Form Factor

- **Style**: Pen/wand-style handheld, similar to commercial diamond testers
- **Approximate dimensions**: 160mm long, 40mm wide, 25mm tall
- **Material**: 3D printed PLA or PETG (no supports needed for most parts)
- **Two-piece design**: Main body (bottom) and lid (top), joined by snap-fit clips or M2 screws

## Internal Layout

```
  Side view (cross section):
  
  USB-C port                                      Probe exit
  <==]  +-------------------------------------------[==>
        | ESP32-S3 DevKitC  | Perfboard w/        | Probe
        | (on standoffs)    | ADS1115, MOSFETs,   | body
        |                   | resistors            |
        +-------------------------------------------+
        |<--- ~70mm ------->|<----- ~60mm -------->|<20>|
  
  Top view:
  
  +---------------------------------------------------+
  |  [USB-C]  [OLED window]  [Button] [Buzzer holes]  |===> probe tip
  +---------------------------------------------------+
```

## Design Features

- **OLED display window**: Rectangular cutout in the lid, 27mm x 15mm, positioned over the OLED module
- **Button hole**: 7mm circular hole in the lid, aligned with the tactile button
- **Buzzer holes**: Array of small holes (1mm each) for sound to escape
- **USB-C port slot**: Rectangular opening at the back end, 12mm x 7mm
- **Probe exit**: 6mm circular hole at the front end, with a strain relief groove
- **Wire exit**: Small hole on the side or back for the ground plate cable
- **Internal standoffs**: 4 posts matching ESP32-S3 DevKitC mounting holes (M2 screws)
- **Perfboard shelf**: Ledge or posts to support the component perfboard
- **Snap-fit or screw closure**: Lid attaches with 4x M2 screw holes or snap-fit clips along the long edges

## Separate Ground Plate

The ground plate is a separate piece connected by a wire:
- Small copper plate (~25x25mm) mounted on a 3D printed base/pedestal
- Wire runs from the plate through the enclosure wire exit to GND on the board
- Pedestal keeps the plate flat and stable on a desk

## Files

- `enclosure.scad` - OpenSCAD parametric source file (edit this to customize dimensions)
- To generate STL files: Open in OpenSCAD, render (F6), then export STL (F7)
- Print settings: 0.2mm layer height, 20% infill, no supports needed

## Customization

All key dimensions are parameters at the top of `enclosure.scad`. Common adjustments:
- `body_length` - overall length (default 160mm)
- `body_width` - overall width (default 40mm)
- `body_height` - overall height (default 25mm)
- `wall_thickness` - shell thickness (default 2mm)
- `corner_radius` - edge rounding (default 3mm)
- `esp32_offset` - position of the ESP32 inside the body
- Various cutout positions and sizes
