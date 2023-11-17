import hid
import struct

def enumerate():
    for d in hid.enumerate(vendor_id=0x1209, product_id=0xB210):
        for k, v in d.items():
            print(f'{k}: {v}')

def find_device_by_usage(usage_page):
    devices = [d for d in hid.enumerate(vendor_id=0x1209, product_id=0xB210) if d['usage_page'] == usage_page]
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
    write_feature_report(d, bytes([0x30, 0b00000000]))


def reset_bootsel():
    d = find_vendor_device()
    write_feature_report(d, bytes([0x30, 0b00000001]))


def reset_clear_flash():
    d = find_vendor_device()
    write_feature_report(d, bytes([0x30, 0b00000010]))


def set_autonomous_mode(enabled):
    d = find_lighting_device()
    write_feature_report(d, bytes([0x06, 0x01 if enabled else 0x00]))


def update_lamp(r, g, b, lamp_id=0):
    d = find_lighting_device()

    report = bytearray([0x04, 0x01, 0x01])
    for lamp in [lamp_id, 0x00, 0x00, 0x00]:
        report.extend(struct.pack('<H', lamp))
    for c in [(r, g, b, 1)] + [(0, 0, 0, 0)] * 3:
        report.extend(struct.pack('<BBBB', *c))

    write_feature_report(d, report)


def set_animation(lamp_id, animation_type, data, set_default=False):
    report = bytearray([0x31, lamp_id, animation_type])
    report.extend(data)
    report.extend([0x00] * (60 - len(data)))

    d = find_vendor_device()
    if set_default:
        write_feature_report(d, report)
    else:
        write_output_report(d, report)


def set_none_animation(lamp_id=0, set_default=False):
    set_animation(0, 0x00, [], set_default=set_default)


def set_fade_animation(colors, fade_time_ms=2000, hold_time_ms=500, lamp_id=0, set_default=False):
    data = bytearray()
    data.extend(struct.pack('<B', len(colors)))
    for c in colors + [(0, 0, 0)] * (8 - len(colors)):
        data.extend(struct.pack('<BBB', *c))
    data.extend(struct.pack('<HH', fade_time_ms, hold_time_ms))
    set_animation(lamp_id, 0x02, data, set_default)


def set_breathe_animation(
        on_color, off_color=None,
        on_fade_time_ms=2000, on_time_ms=500,
        off_fade_time_ms=2000, off_time_ms=2000,
        lamp_id=0, set_default=False):
    data = bytearray()
    data.extend(struct.pack('<BBB', *on_color))
    data.extend(struct.pack('<BBB', *(off_color if off_color is not None else on_color)))
    data.extend(struct.pack('<HHHH', on_fade_time_ms, on_time_ms, off_fade_time_ms, off_fade_time_ms))
    set_animation(lamp_id, 0x01, data, set_default)
