#include "pico/stdlib.h"
#include "tusb.h"

#include "color/color.h"
#include "controller/animations/fade.h"
#include "controller/controller.h"
#include "device/lamp.h"

controller_t ctrl;

int main()
{
    ctrl_init(&ctrl);
    lamp_init();
    tusb_init();

    // struct RGBi color = {0x00, 0xdb, 0xa8};
    // struct AnimationFade fade = anim_fade_breathe(color, 2000000);
    struct RGBi color1 = {0x0a, 0xbf, 0xa1};
    struct RGBi color2 = {0xde, 0x8e, 0x0d};
    struct AnimationFade *fade = anim_fade_new_cross(color1, color2, 4000000);

    ctrl_set_animation(&ctrl, 0, anim_fade, (void *) fade);

    while (true) {
        ctrl_task(&ctrl);
        tud_task();
    }

    return 0;
}
