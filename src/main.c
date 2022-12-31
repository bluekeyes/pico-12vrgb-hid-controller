#include "pico/stdlib.h"
#include "tusb.h"

int main() {
    tusb_init();

    while (true) {
        tud_task();
    }

    return 0;
}
