function in2mm(v) = 25.4*v;

$fa = 0.5;
$fs = 0.5;
eps = 1;

wall = 2;
standoff_wall = 1.6;

pcb_width = in2mm(1.25);
pcb_length = in2mm(4);
pcb_height = 1.6;

pcb_clearance = 0.4;
pin_clearance = 3;

rgb_offset_x = in2mm(0.825);
rgb_offset_y = in2mm(0.175);
rgb_spacing = in2mm(0.7);

rgb_cable_width = 5;
rgb_cable_height = 1.2;

usb_width = 12;
usb_height = 6;
usb_center_offset_z = 2.5;

case_width = 2*wall + 2*pcb_clearance + pcb_width;
case_length = wall + pcb_clearance + pcb_length;
case_floor = 2;
case_pcb_depth = 8;
case_height = case_floor + pin_clearance + pcb_height + case_pcb_depth;

screw_diameter = 2.5;
screw_offset = in2mm(0.5625);

clip_size = 1;

module mirror_x() {
    children(); mirror([1, 0, 0]) children();
}

module mirror_y() {
    children(); mirror([0, 1, 0]) children();
}

module rgb_cable_guide(h, angle=45) {
    translate([0, -eps/2, -h])
    union() {
        mirror_x()
        translate([rgb_cable_height/2, 0, 0])
        rotate([0, -angle, 0])
        translate([-rgb_cable_width, 0, 0])
        cube([rgb_cable_width, wall + eps, rgb_cable_height]);

        translate([-rgb_cable_height/2, 0, 0])
        cube([rgb_cable_height, wall + eps, h + eps]);
    }
}

module case_shell() {
    difference() {
        // body
        translate([-case_length/2, -case_width/2, 0])
        cube([case_length, case_width, case_height]);

        // main cutout 
        translate([
            -case_length/2 + wall,
            -pcb_width/2 - pcb_clearance,
            case_floor
        ])
        cube([
            pcb_clearance + pcb_length + eps,
            2*pcb_clearance + pcb_width,
            case_height - case_floor + eps,
        ]);

        // usb cutout
        let (
            dz = case_floor + pin_clearance + pcb_height + usb_center_offset_z
        ) {
            translate([
                -(case_length + eps)/2,
                -usb_width/2,
                dz - usb_height/2,
            ])
            cube([wall + eps, usb_width, usb_height]);
        }

        // rgb cable guides
        mirror_y() {
            for (i = [0, 1]) {
                translate([
                    case_length/2 - rgb_offset_x - i*rgb_spacing,
                    -case_width/2,
                    case_height,
                ])
                rgb_cable_guide(0.5 * case_pcb_depth, angle=35);
            }
        }
    }
}

module standoffs() {
    edge_width = in2mm(0.125);
    pico_length = 14;
    pico_width = 14;
    sata_length = in2mm(0.25);
    inner_w = pcb_width + 2*pcb_clearance;

    // edge supports
    mirror_y()
    translate([
        -case_length/2 + wall,
        (pcb_width/2 + pcb_clearance) - edge_width,
        0,
    ])
    cube([pcb_length + pcb_clearance, edge_width, pin_clearance]);

    // sata connector support
    translate([case_length/2 - sata_length, -inner_w/2, 0])
    cube([sata_length, inner_w, pin_clearance]);

    // pico support
    translate([-case_length/2 + wall, -pico_width/2, 0])
    cube([pico_length, pico_width, pin_clearance]);

    // screw standoff
    translate([case_length/2 - screw_offset, 0, 0])
    cylinder(d=screw_diameter + 2*standoff_wall, h=pin_clearance);
}

module clips(length) {
    dz = pcb_height - pcb_clearance;
    mirror_y()
    translate([
        0,
        pcb_width/2 + pcb_clearance,
        case_floor + pin_clearance + clip_size + dz,
    ])
    rotate([0, 90, 0])
    cylinder(r=clip_size, h=length, $fn=4);
}

module case() {
    difference() {
        union() {
            case_shell();
            translate([0, 0, case_floor]) standoffs();

            translate([-case_length/2 + wall, 0, 0])
            clips(10);
        }

        // screw hole
        translate([case_length/2 - screw_offset, 0, -eps/2])
        cylinder(d=screw_diameter, h=case_floor + pin_clearance + eps);
    }
}

case();
