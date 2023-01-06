import hid

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
