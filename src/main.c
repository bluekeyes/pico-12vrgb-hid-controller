#include "pico/stdlib.h"
#include "tusb.h"

#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "rgb/rgb.h"

controller_t ctrl;

int main()
{
    ctrl_init(&ctrl);
    rgb_init();
    tusb_init();

    rgb_tuple_t color = {0x00, 0xdb, 0xa8, 0x01};
    struct AnimationFade fade = anim_fade_breathe(color, 2000000);
    ctrl_set_animation(&ctrl, anim_fade, (void *) &fade);

    while (true) {
        ctrl_task(&ctrl);
        tud_task();
    }

    return 0;
}
