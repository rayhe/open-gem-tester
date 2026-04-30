/*
 * OpenGemTester Enclosure
 * Parametric 3D-printable enclosure for the OpenGemTester project.
 *
 * Designed for:
 *   - ESP32-S3 DevKitC-1 (approx 70mm x 25mm)
 *   - Small perfboard for supporting circuitry (30mm x 50mm)
 *   - SSD1306 0.96" OLED display (27mm x 13mm visible)
 *   - 6mm tactile button
 *   - Piezo buzzer (12mm diameter)
 *   - Copper probe tip exiting from one end
 *   - USB-C access on the opposite end
 *
 * Print settings:
 *   Layer height: 0.2mm
 *   Perimeters: 3
 *   Infill: 20%
 *   Supports: None needed in recommended orientation
 *   Material: PLA or PETG
 *
 * License: MIT
 */

// ==========================================
// Parameters (adjust to fit your components)
// ==========================================

// Overall body dimensions
body_length = 160;
body_width  = 40;
body_height = 25;
wall_thickness = 2.0;
corner_radius = 4;

// Lid
lid_thickness = 2.0;
lid_lip = 1.5;        // Overlap lip for snap fit
snap_tolerance = 0.3;  // Clearance for snap fit

// ESP32-S3 DevKitC-1 mounting
esp32_length = 70;
esp32_width  = 25.5;
esp32_offset_x = 50;  // Distance from probe end
esp32_offset_y = 0;   // Centered
esp32_standoff_height = 3;
esp32_screw_diameter = 2.2;  // M2 screw holes

// Perfboard bay
perf_length = 50;
perf_width  = 30;
perf_offset_x = 5;   // Near probe end

// OLED display window
oled_window_width  = 27;
oled_window_height = 15;
oled_offset_x = 85;  // From probe end
oled_offset_y = 0;    // Centered on width

// Button hole
button_diameter = 7;
button_offset_x = 25;  // From probe end
button_offset_y = 10;  // Offset from center

// Buzzer sound port
buzzer_diameter = 12;
buzzer_offset_x = 45;
buzzer_offset_y = -10;
buzzer_hole_count = 7;
buzzer_hole_diameter = 1.5;

// Probe exit hole (narrow end)
probe_hole_diameter = 4;

// USB-C port (opposite end)
usbc_width  = 10;
usbc_height = 4;

// Ground plate wire exit
wire_hole_diameter = 3;
wire_hole_offset_x = 130;

// ==========================================
// Modules
// ==========================================

module rounded_box(l, w, h, r) {
    hull() {
        for (x = [r, l - r])
            for (y = [r, w - r])
                translate([x, y, 0])
                    cylinder(h = h, r = r, $fn = 32);
    }
}

module body_shell() {
    difference() {
        // Outer shell
        rounded_box(body_length, body_width, body_height, corner_radius);

        // Inner cavity
        translate([wall_thickness, wall_thickness, wall_thickness])
            rounded_box(
                body_length - 2 * wall_thickness,
                body_width - 2 * wall_thickness,
                body_height - wall_thickness,  // Open top for lid
                corner_radius - wall_thickness
            );
    }
}

module probe_hole() {
    // Hole on the narrow end (x = 0) for the copper probe tip
    translate([- 1, body_width / 2, body_height / 2])
        rotate([0, 90, 0])
            cylinder(h = wall_thickness + 2, d = probe_hole_diameter, $fn = 32);
}

module usbc_port() {
    // USB-C opening on the opposite narrow end (x = body_length)
    translate([body_length - wall_thickness - 1, body_width / 2 - usbc_width / 2, body_height / 2 - usbc_height / 2])
        cube([wall_thickness + 2, usbc_width, usbc_height]);
}

module oled_window() {
    // Rectangular window on the top face for the OLED display
    translate([
        oled_offset_x - oled_window_width / 2,
        body_width / 2 + oled_offset_y - oled_window_height / 2,
        body_height - wall_thickness - 1
    ])
        cube([oled_window_width, oled_window_height, wall_thickness + 2]);
}

module button_hole() {
    // Round hole on the top face for the tactile button
    translate([
        button_offset_x,
        body_width / 2 + button_offset_y,
        body_height - wall_thickness - 1
    ])
        cylinder(h = wall_thickness + 2, d = button_diameter, $fn = 32);
}

module buzzer_holes() {
    // Array of small holes for sound from the piezo buzzer
    translate([buzzer_offset_x, body_width / 2 + buzzer_offset_y, body_height - wall_thickness - 1]) {
        // Center hole
        cylinder(h = wall_thickness + 2, d = buzzer_hole_diameter, $fn = 16);
        // Ring of holes
        for (i = [0 : buzzer_hole_count - 1]) {
            angle = i * 360 / buzzer_hole_count;
            translate([cos(angle) * 4, sin(angle) * 4, 0])
                cylinder(h = wall_thickness + 2, d = buzzer_hole_diameter, $fn = 16);
        }
    }
}

module wire_exit() {
    // Hole on the side for the ground plate wire
    translate([
        wire_hole_offset_x,
        -1,
        body_height / 2
    ])
        rotate([-90, 0, 0])
            cylinder(h = wall_thickness + 2, d = wire_hole_diameter, $fn = 32);
}

module esp32_standoffs() {
    // Four standoff posts for M2 screws to mount the ESP32 board
    standoff_positions = [
        [esp32_offset_x, body_width / 2 - esp32_width / 2 + 2],
        [esp32_offset_x, body_width / 2 + esp32_width / 2 - 2],
        [esp32_offset_x + esp32_length - 4, body_width / 2 - esp32_width / 2 + 2],
        [esp32_offset_x + esp32_length - 4, body_width / 2 + esp32_width / 2 - 2]
    ];

    for (pos = standoff_positions) {
        translate([pos[0], pos[1], wall_thickness])
            difference() {
                cylinder(h = esp32_standoff_height, d = 5, $fn = 32);
                cylinder(h = esp32_standoff_height + 1, d = esp32_screw_diameter, $fn = 32);
            }
    }
}

module lid() {
    // Snap-fit lid
    difference() {
        rounded_box(
            body_length - 2 * wall_thickness + 2 * snap_tolerance,
            body_width - 2 * wall_thickness + 2 * snap_tolerance,
            lid_thickness,
            corner_radius - wall_thickness
        );

        // OLED window passthrough
        translate([
            oled_offset_x - wall_thickness - oled_window_width / 2,
            body_width / 2 + oled_offset_y - wall_thickness - oled_window_height / 2,
            -1
        ])
            cube([oled_window_width, oled_window_height, lid_thickness + 2]);

        // Button passthrough
        translate([
            button_offset_x - wall_thickness,
            body_width / 2 + button_offset_y - wall_thickness,
            -1
        ])
            cylinder(h = lid_thickness + 2, d = button_diameter, $fn = 32);

        // Buzzer holes passthrough
        translate([buzzer_offset_x - wall_thickness, body_width / 2 + buzzer_offset_y - wall_thickness, -1]) {
            cylinder(h = lid_thickness + 2, d = buzzer_hole_diameter, $fn = 16);
            for (i = [0 : buzzer_hole_count - 1]) {
                angle = i * 360 / buzzer_hole_count;
                translate([cos(angle) * 4, sin(angle) * 4, 0])
                    cylinder(h = lid_thickness + 2, d = buzzer_hole_diameter, $fn = 16);
            }
        }
    }

    // Snap lip around the inside edge
    translate([snap_tolerance, snap_tolerance, lid_thickness])
        difference() {
            rounded_box(
                body_length - 2 * wall_thickness,
                body_width - 2 * wall_thickness,
                lid_lip,
                corner_radius - wall_thickness - snap_tolerance
            );
            translate([snap_tolerance + 0.5, snap_tolerance + 0.5, -1])
                rounded_box(
                    body_length - 2 * wall_thickness - 2 * snap_tolerance - 1,
                    body_width - 2 * wall_thickness - 2 * snap_tolerance - 1,
                    lid_lip + 2,
                    max(1, corner_radius - wall_thickness - 2 * snap_tolerance)
                );
        }
}

module ground_plate_stand() {
    // Small stand for the copper ground plate
    plate_size = 25;
    base_size = 35;
    base_height = 5;
    plate_recess = 1;

    difference() {
        // Base
        rounded_box(base_size, base_size, base_height, 3);

        // Recess for the copper plate
        translate([(base_size - plate_size) / 2, (base_size - plate_size) / 2, base_height - plate_recess])
            cube([plate_size, plate_size, plate_recess + 1]);

        // Wire channel
        translate([base_size / 2, -1, 2])
            rotate([-90, 0, 0])
                cylinder(h = (base_size - plate_size) / 2 + 2, d = wire_hole_diameter, $fn = 32);
    }
}

// ==========================================
// Assembly
// ==========================================

// Main body with all cutouts
module main_body() {
    difference() {
        body_shell();
        probe_hole();
        usbc_port();
        oled_window();
        button_hole();
        buzzer_holes();
        wire_exit();
    }
    esp32_standoffs();
}

// Render everything (separated for printing)
// Comment/uncomment to export individual parts

// Main body
main_body();

// Lid (offset for viewing, move to origin for export)
translate([0, body_width + 10, 0])
    lid();

// Ground plate stand (offset for viewing)
translate([0, 2 * body_width + 20, 0])
    ground_plate_stand();
