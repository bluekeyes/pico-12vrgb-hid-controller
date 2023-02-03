#include "pico/stdlib.h"
#include "tusb.h"

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "controller/persist.h"
#include "device/lamp.h"
#include "device/temperature.h"
#include "hid/vendor/report.h"

controller_t ctrl;

int main()
{
    stdio_init_all();

    lamp_init();
    temperature_init();

    ctrl_init(&ctrl);
    ctrl_persist_init();

    tusb_init();

    // Set default animations for all lamps
    for (uint8_t id = 0; id <= MAX_LAMP_ID; id++) {
        struct Vendor12VRGBAnimationReport *report = ctrl_persist_find_report(id);
        if (report != NULL) {
            ctrl_set_animation_from_report(&ctrl, report);
        }
    }

    while (true) {
        tud_task();
        ctrl_task(&ctrl);
    }

    return 0;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    ctrl_suspend(&ctrl);
}

void tud_resume_cb()
{
    ctrl_resume(&ctrl);
}
