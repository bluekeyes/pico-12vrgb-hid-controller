#include "pico/stdlib.h"
#include "tusb.h"

#include "controller/controller.h"
#include "rgb/rgb.h"

controller_t ctrl;

int main()
{
    ctrl_init(&ctrl);
    rgb_init();

    tusb_init();

    while (true) {
        tud_task();
    }

    return 0;
}
