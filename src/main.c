#include "pico/stdlib.h"
#include "tusb.h"

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "controller/persist.h"
#include "device/lamp.h"
#include "hid/vendor/report.h"

controller_t ctrl;

int main()
{
    stdio_init_all();

    ctrl_init(&ctrl);
    ctrl_persist_init();

    // TODO(bkeyes): remove or flag this debug call
    ctrl_persist_dump();

    lamp_init();
    tusb_init();

    // Set default animations for all lamps
    for (uint8_t id = 0; id <= MAX_LAMP_ID; id++) {
        struct Vendor12VRGBAnimationReport *report = ctrl_persist_find_report(id);
        if (report != NULL) {
            ctrl_set_animation_from_report(&ctrl, report);
        }
    }

    while (true) {
        ctrl_task(&ctrl);
        tud_task();
    }

    return 0;
}
