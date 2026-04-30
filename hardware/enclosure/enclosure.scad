// OpenGemTester - Parametric Enclosure
// OpenSCAD source file
// Pen-style handheld form factor for ESP32-S3 based gemstone tester
//
// Usage:
//   1. Open this file in OpenSCAD (https://openscad.org)
//   2. Adjust parameters below to fit your specific components
//   3. Render with F6, export STL with F7
//   4. Print body and lid separately (no supports needed)
//
// License: MIT

// ============================================================
// PARAMETERS - Adjust these to customize your enclosure
// ============================================================

// --- Overall body dimensions ---
body_length = 160;       // mm, total length
body_width  = 40;        // mm, total width
body_height = 25;        // mm, total height (body + lid together)
wall_thickness = 2;      // mm, shell wall thickness
corner_radius = 3;       // mm, rounded edge radius

// --- Body/lid split ---
// The body is the bottom part, lid is the top part
body_split = 15;         // mm, height of the bottom body piece
lid_height = body_height - body_split; // mm, height of the lid

// --- ESP32-S3 DevKitC-1 dimensions ---
// Typical board: ~70mm x 25.4mm
esp32_length = 70;       // mm
esp32_width  = 25.5;     // mm
esp32_offset_x = 8;      // mm, distance from back wall to ESP32 front edge
esp32_offset_y = (body_width - esp32_width) / 2; // centered
esp32_standoff_h = 3;    // mm, height of standoffs above floor
esp32_standoff_d = 4;    // mm, standoff outer diameter
esp32_screw_d = 2.2;     // mm, hole for M2 screw

// --- ESP32 mounting hole positions (relative to board corner) ---
// Approximate positions for DevKitC-1; measure your board and adjust
esp32_hole_inset_x = 2.5;  // mm from board edge
esp32_hole_inset_y = 2.5;  // mm from board edge

// --- Perfboard area ---
// Sits next to the ESP32, holds ADS1115, MOSFETs, resistors
perfboard_length = 50;   // mm
perfboard_width  = 30;   // mm
perfboard_offset_x = esp32_offset_x + esp32_length + 3; // 3mm gap after ESP32
perfboard_standoff_h = 3;
perfboard_standoff_d = 4;

// --- OLED display window (SSD1306 0.96" 128x64) ---
// Active area is roughly 22mm x 11mm; cutout slightly larger
oled_window_w = 27;      // mm, window width
oled_window_h = 15;      // mm, window height
oled_offset_x = esp32_offset_x + 20; // mm from back of enclosure
oled_offset_y = (body_width - oled_window_w) / 2; // centered

// --- Button hole ---
button_hole_d = 7;       // mm, diameter for 6mm tactile button
button_offset_x = oled_offset_x + oled_window_h + 15; // mm from back
button_offset_y = body_width / 2; // centered

// --- Buzzer sound holes ---
buzzer_hole_d = 1.2;     // mm, individual hole diameter
buzzer_grid_rows = 3;
buzzer_grid_cols = 3;
buzzer_grid_spacing = 3; // mm between hole centers
buzzer_offset_x = button_offset_x + 15; // mm from back
buzzer_offset_y = body_width / 2; // centered

// --- USB-C port cutout (at the back end) ---
usbc_width  = 12;        // mm
usbc_height = 7;         // mm
usbc_z_offset = esp32_standoff_h + 1.6 + 1; // standoff + PCB thickness + center offset

// --- Probe exit hole (at the front end) ---
probe_hole_d = 6;        // mm, diameter
probe_hole_z = body_split / 2; // centered in body height

// --- Ground plate wire exit (side hole) ---
wire_exit_d = 4;         // mm, diameter for wire pass-through
wire_exit_x = body_length - 30; // mm from back, on the side
wire_exit_z = body_split / 2;

// --- Lid attachment ---
// Option 1: Screw holes (M2)
use_screws = true;
screw_hole_d = 2.2;      // mm, for M2 screws
screw_boss_d = 6;         // mm, boss diameter around screw hole
num_screws = 4;
// Screw positions: corners, inset from edges
screw_inset_x = 10;      // mm from each end
screw_inset_y = 6;        // mm from each side

// --- Lid lip (keeps lid aligned) ---
lid_lip_width = 1.5;     // mm, width of the alignment lip
lid_lip_height = 2;      // mm, how far the lip extends down into body

// --- Tolerances ---
tolerance = 0.3;         // mm, clearance for fitting parts

// ============================================================
// MODULES
// ============================================================

// Rounded box primitive
// Creates a box with rounded vertical edges
module rounded_box(l, w, h, r) {
    hull() {
        translate([r, r, 0])
            cylinder(h=h, r=r, $fn=30);
        translate([l-r, r, 0])
            cylinder(h=h, r=r, $fn=30);
        translate([r, w-r, 0])
            cylinder(h=h, r=r, $fn=30);
        translate([l-r, w-r, 0])
            cylinder(h=h, r=r, $fn=30);
    }
}

// Standoff post with screw hole
module standoff(h, od, id) {
    difference() {
        cylinder(h=h, d=od, $fn=20);
        translate([0, 0, -0.1])
            cylinder(h=h+0.2, d=id, $fn=20);
    }
}

// ============================================================
// BODY (bottom piece)
// ============================================================

module body() {
    difference() {
        // Outer shell
        rounded_box(body_length, body_width, body_split, corner_radius);

        // Hollow out the interior
        translate([wall_thickness, wall_thickness, wall_thickness])
            rounded_box(
                body_length - 2*wall_thickness,
                body_width - 2*wall_thickness,
                body_split, // cut through the top (lid will cap it)
                corner_radius - wall_thickness
            );

        // USB-C port cutout (back end, centered)
        translate([
            -0.1,
            (body_width - usbc_width) / 2,
            usbc_z_offset - usbc_height/2
        ])
            cube([wall_thickness + 0.2, usbc_width, usbc_height]);

        // Probe exit hole (front end, centered)
        translate([body_length, body_width/2, probe_hole_z])
            rotate([0, -90, 0])
                cylinder(h=wall_thickness+0.2, d=probe_hole_d, $fn=30);

        // Ground plate wire exit (right side)
        translate([wire_exit_x, body_width, wire_exit_z])
            rotate([90, 0, 0])
                cylinder(h=wall_thickness+0.2, d=wire_exit_d, $fn=20);

        // Screw holes in body (if using screws)
        if (use_screws) {
            for (pos = screw_positions()) {
                translate([pos[0], pos[1], -0.1])
                    cylinder(h=body_split+0.2, d=screw_hole_d, $fn=20);
            }
        }
    }

    // ESP32 standoffs (4 corners of the board)
    for (dx = [0, esp32_length - 2*esp32_hole_inset_x])
        for (dy = [0, esp32_width - 2*esp32_hole_inset_y])
            translate([
                esp32_offset_x + esp32_hole_inset_x + dx,
                esp32_offset_y + esp32_hole_inset_y + dy,
                wall_thickness
            ])
                standoff(esp32_standoff_h, esp32_standoff_d, esp32_screw_d);

    // Perfboard standoffs (4 corners)
    for (dx = [3, perfboard_length - 3])
        for (dy = [3, min(perfboard_width, body_width - 2*wall_thickness - 6) - 3])
            translate([
                perfboard_offset_x + dx,
                wall_thickness + 3 + dy,
                wall_thickness
            ])
                standoff(perfboard_standoff_h, perfboard_standoff_d, esp32_screw_d);

    // Screw bosses in body corners
    if (use_screws) {
        for (pos = screw_positions()) {
            translate([pos[0], pos[1], wall_thickness])
                difference() {
                    cylinder(h=body_split - wall_thickness, d=screw_boss_d, $fn=20);
                    translate([0, 0, -0.1])
                        cylinder(h=body_split, d=screw_hole_d, $fn=20);
                }
        }
    }
}

// ============================================================
// LID (top piece)
// ============================================================

module lid() {
    difference() {
        union() {
            // Outer shell of lid
            rounded_box(body_length, body_width, lid_height, corner_radius);

            // Alignment lip (extends downward, will fit inside body)
            translate([
                wall_thickness + tolerance,
                wall_thickness + tolerance,
                -lid_lip_height
            ])
                rounded_box(
                    body_length - 2*(wall_thickness + tolerance),
                    body_width - 2*(wall_thickness + tolerance),
                    lid_lip_height,
                    max(1, corner_radius - wall_thickness - tolerance)
                );
        }

        // Hollow out (thin shell)
        translate([wall_thickness, wall_thickness, -lid_lip_height - 0.1])
            rounded_box(
                body_length - 2*wall_thickness,
                body_width - 2*wall_thickness,
                lid_height - wall_thickness + 0.1,
                max(1, corner_radius - wall_thickness)
            );

        // OLED display window cutout
        translate([oled_offset_x, oled_offset_y, -0.1])
            cube([oled_window_h, oled_window_w, lid_height + 0.2]);

        // Button hole
        translate([button_offset_x, button_offset_y, -0.1])
            cylinder(h=lid_height + 0.2, d=button_hole_d, $fn=30);

        // Buzzer sound holes (grid pattern)
        for (r = [0:buzzer_grid_rows-1])
            for (c = [0:buzzer_grid_cols-1])
                translate([
                    buzzer_offset_x + r * buzzer_grid_spacing - (buzzer_grid_rows-1)*buzzer_grid_spacing/2,
                    buzzer_offset_y + c * buzzer_grid_spacing - (buzzer_grid_cols-1)*buzzer_grid_spacing/2,
                    -0.1
                ])
                    cylinder(h=lid_height + 0.2, d=buzzer_hole_d, $fn=15);

        // Probe exit hole in lid (matching body)
        translate([body_length, body_width/2, lid_height/2])
            rotate([0, -90, 0])
                cylinder(h=wall_thickness+0.2, d=probe_hole_d, $fn=30);

        // USB-C cutout in lid (if port is tall enough to reach lid)
        translate([
            -0.1,
            (body_width - usbc_width) / 2,
            -0.1
        ])
            cube([wall_thickness + 0.2, usbc_width, lid_height + 0.2]);

        // Screw holes through lid
        if (use_screws) {
            for (pos = screw_positions()) {
                translate([pos[0], pos[1], -lid_lip_height - 0.1])
                    cylinder(h=lid_height + lid_lip_height + 0.2, d=screw_hole_d, $fn=20);
            }
        }
    }
}

// ============================================================
// GROUND PLATE PEDESTAL (separate piece)
// ============================================================

module ground_plate_pedestal() {
    pedestal_w = 35;    // mm, base width
    pedestal_d = 35;    // mm, base depth
    pedestal_h = 8;     // mm, base height
    plate_recess_w = 26; // mm, recess for copper plate
    plate_recess_d = 26;
    plate_recess_depth = 1.5; // mm
    wire_channel_d = 3; // mm

    difference() {
        // Base block with rounded corners
        hull() {
            for (dx = [3, pedestal_w-3])
                for (dy = [3, pedestal_d-3])
                    translate([dx, dy, 0])
                        cylinder(h=pedestal_h, r=3, $fn=20);
        }

        // Recess for copper plate (centered on top)
        translate([
            (pedestal_w - plate_recess_w) / 2,
            (pedestal_d - plate_recess_d) / 2,
            pedestal_h - plate_recess_depth
        ])
            cube([plate_recess_w, plate_recess_d, plate_recess_depth + 0.1]);

        // Wire channel (exits from the side)
        translate([pedestal_w/2, -0.1, pedestal_h/2])
            rotate([-90, 0, 0])
                cylinder(h=pedestal_d/2 + 0.1, d=wire_channel_d, $fn=20);

        // Wire channel meets the plate recess from below
        translate([pedestal_w/2, pedestal_d/4, pedestal_h - plate_recess_depth - 0.1])
            cylinder(h=plate_recess_depth + 0.2, d=wire_channel_d, $fn=20);
    }
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

function screw_positions() = [
    [screw_inset_x, screw_inset_y],
    [body_length - screw_inset_x, screw_inset_y],
    [screw_inset_x, body_width - screw_inset_y],
    [body_length - screw_inset_x, body_width - screw_inset_y]
];

// ============================================================
// RENDER
// ============================================================
// Uncomment the part you want to render/export:
//   - For printing, render one part at a time
//   - For preview, show all parts in exploded view

// --- Exploded preview (default) ---
// Shows body, lid (raised), and ground plate pedestal side by side

// Body (bottom)
color("SteelBlue", 0.8)
    body();

// Lid (top) - raised for visibility
color("LightSteelBlue", 0.8)
    translate([0, 0, body_split + 10]) // 10mm gap for exploded view
        lid();

// Ground plate pedestal (off to the side)
color("Goldenrod", 0.8)
    translate([body_length + 20, 0, 0])
        ground_plate_pedestal();

// --- Individual parts for STL export ---
// Uncomment ONE of these and comment out the exploded view above:

// body();             // Export as: OpenGemTester_body.stl
// lid();              // Export as: OpenGemTester_lid.stl (print upside down)
// ground_plate_pedestal(); // Export as: OpenGemTester_plate_pedestal.stl
