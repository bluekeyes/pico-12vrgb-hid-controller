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


def write_feature_report(d, report):
    h = hid.device()
    try:
        h.open_path(d['path'])
        h.send_feature_report(report)
    finally:
        h.close()


def reset():
    d = find_vendor_device()
    write_feature_report(d, bytes([0x07, 0b00000000]))


def reset_bootsel():
    d = find_vendor_device()
    write_feature_report(d, bytes([0x07, 0b00000001]))


def reset_clear_flash():
    d = find_vendor_device()
    write_feature_report(d, bytes([0x07, 0b00000010]))


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


def set_animation(lamp_id, animation_type, params, colors, set_default=False):
    report_id = 0x09 if set_default else 0x08

    d = find_vendor_device()
    data = bytearray([report_id, lamp_id, animation_type])

    for param in params + [0] * (8 - len(params)):
        data.extend(struct.pack('<l', param))

    for color in colors + [(0, 0, 0)] * (8 - len(colors)):
        data.extend(struct.pack('<BBB', *color))

    if set_default:
        write_feature_report(d, data)
    else:
        write_output_report(d, data)


def set_fade_animation_lamp0(fade_time, hold_time, colors, set_default=False):
    set_animation(0, 0x02, [len(colors), fade_time, hold_time], colors, set_default=set_default)


def set_breathe_animation_lamp0(fade_time, color, set_default=False):
    set_animation(0, 0x01, [fade_time], [color], set_default=set_default)


def set_none_animation_lamp0(set_default=False):
    set_animation(0, 0x00, [], [], set_default=set_default)
