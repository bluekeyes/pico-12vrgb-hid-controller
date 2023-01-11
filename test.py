import hid
import struct

def find_device_by_usage(usage_page):
    devices = [d for d in hid.enumerate(vendor_id=0xcafe, product_id=0x4100) if d['usage_page'] == usage_page]
    if len(devices) == 0:
        raise ValueError('Device not found!')
    return devices[0]


def find_lighting_device():
    return find_device_by_usage(0x59)


def find_vendor_device():
    return find_device_by_usage(0xFF00)


def write_output_report(d, report):
    h = hid.device()
    try:
        h.open_path(d['path'])
        h.write(report)
    finally:
        h.close()


def bootsel():
    d = find_vendor_device()
    write_output_report(d, bytes([0x07, 0x01]))


def set_autonomous_mode(enabled):
    d = find_lighting_device()
    write_output_report(d, bytes([0x06, 0x01 if enabled else 0x00]))


def update_lamp0(r, g, b):
    d = find_lighting_device()
    write_output_report(d, bytes([
        0x04, # report id
        0x01, # lamp count
        0x00, # id
        0x00, # (empty id slot)
        r,    # rgbi tuple
        g,
        b,
        0x01,
        0x00, # (empty rgbi slot)
        0x00,
        0x00,
        0x00,
        0x01, # update flags
        0x00
    ]))


def set_fade_animation(fade_time, hold_time, colors):
    d = find_vendor_device()
    data = bytearray([
        0x08, # report id
        0x02, # animation type
    ])

    data.extend(struct.pack('<llll',
        len(colors),
        fade_time,
        hold_time,
        0,
    ))

    for color in colors:
        data.extend(struct.pack('<BBB', *color))
    for i in range(8 - len(colors)):
        data.extend(struct.pack('<BBB', 0, 0, 0))

    write_output_report(d, data)


def set_breathe_animation(fade_time, color):
    d = find_vendor_device()
    data = bytearray([
        0x08, # report id
        0x01, # animation type
    ])

    data.extend(struct.pack('<llll',
        fade_time,
        0,
        0,
        0,
    ))

    data.extend(struct.pack('<BBB', *color))
    for i in range(7):
        data.extend(struct.pack('<BBB', 0, 0, 0))

    write_output_report(d, data)


def set_none_animation():
    d = find_vendor_device()
    data = bytearray([
        0x08, # report id
        0x00, # animation type
    ])
    data.extend(b'\00' * 40)

    write_output_report(d, data)
