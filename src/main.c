#include "pico/stdlib.h"
#include "tusb.h"

#include "controller/controller.h"

controller_t ctrl;

int main()
{
    ctrl_init(&ctrl);
    tusb_init();

    while (true) {
        tud_task();
    }

    return 0;
}
