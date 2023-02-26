#include "hardware/sync.h"
#include "pico/stdlib.h"
#include "tusb.h"

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "controller/persist.h"
#include "controller/sensor.h"
#include "device/lamp.h"
#include "device/temperature.h"
#include "hid/vendor/report.h"

controller_t ctrl;
sensor_controller_t sensectrl;

bool is_suspended = false;

int main()
{
    stdio_init_all();

    lamp_init();
    temperature_init();

    ctrl_init(&ctrl);
    ctrl_sensor_init(&sensectrl);
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

        // If suspended, block waiting for any event. On any wake-up (real or
        // spurius), restart the loopo and execute the USB task, because that's
        // what will clear the suspended flag. Skip any other tasks while suspended.
        if (is_suspended) {
            __wfe();
        } else {
            ctrl_task(&ctrl);
            ctrl_sensor_task(&sensectrl);
        }
    }

    return 0;
}

void suspend()
{
    if (!is_suspended) {
        ctrl_suspend(&ctrl);
        is_suspended = true;
    }
}

void resume()
{
    if (is_suspended) {
        ctrl_resume(&ctrl);
        is_suspended = false;
    }
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    suspend();
}

void tud_resume_cb()
{
    resume();
}

void tud_mount_cb(void)
{
    // For some reason (possibly because of the sensor usage), Windows is not
    // resuming the device when exiting sleep, but instead doing a bus reset,
    // which appears to tinyUSB as a mount event. Treat this as a normal resume
    // because we still get a suspend event on entering sleep.
    resume();
}
