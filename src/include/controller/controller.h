#ifndef CONTROLLER_CONTROLLER_H_
#define CONTROLLER_CONTROLLER_H_

#include "device/lamp.h"
#include "device/specs.h"
#include "hid/lights/report.h"

typedef struct Controller controller_t;

// -----
// Lamps
// -----

typedef struct {
    struct LampValue current;
    struct LampValue next;
    bool dirty;
} lamp_state;

// ---------
// Animation
// ---------

#define ANIM_FRAME_TIME_US (1000000 / CFG_RGB_ANIMATION_FRAME_RATE)

struct AnimationState {
    uint8_t  stage;         /* the current stage of the animation, as set by the frame callback */
    uint32_t frame;         /* the current frame in the full animation */
    uint32_t stage_frame;   /* the current frame in the current stage; resets to 0 on stage change */

    void *data;             /* arbitrary data used by the frame callback */
};

typedef uint8_t (*FrameCallback)(controller_t *ctrl, uint8_t lamp_id, struct AnimationState *state);

// ----------
// Controller
// ----------

struct Controller {
    bool is_autonomous;
    uint8_t next_lamp_id;

    bool do_update;
    lamp_state lamp_state[LAMP_COUNT];

    struct AnimationState animation[LAMP_COUNT];
    FrameCallback frame_cb[LAMP_COUNT];
    uint32_t last_frame_time_us;
};

void ctrl_init(controller_t *ctrl);
void ctrl_task(controller_t *ctrl);

void ctrl_set_next_lamp_attributes_id(controller_t *ctrl, uint8_t lamp_id);
void ctrl_get_lamp_attributes(controller_t *ctrl, struct LampAttributesResponseReport *report);

void ctrl_set_autonomous_mode(controller_t *ctrl, bool autonomous);
bool ctrl_get_autonomous_mode(controller_t *ctrl);

/**
 * @brief Sets the animation that plays in autonomous mode.
 *
 * When starting an animation, the controller takes full ownership of the
 * animation data. If the controller has an existing animation with non-null
 * data, setting a new animation automatically frees the existing animation
 * state. As a result, data must be allocated dynamically.
 *
 * Set a null frame callback and null data to disable animations.
 */
void ctrl_set_animation(controller_t *ctrl, uint8_t lamp_id, FrameCallback frame_cb, void *data);

void ctrl_update_lamp(controller_t *ctrl, uint8_t lamp_id, struct LampValue value, bool apply);
void ctrl_apply_lamp_updates(controller_t *ctrl);

#endif // CONTROLLER_CONTROLLER_H_
