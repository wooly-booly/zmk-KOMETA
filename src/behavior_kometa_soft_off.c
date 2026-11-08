#define DT_DRV_COMPAT zmk_behavior_kometa_soft_off

#include <drivers/behavior.h>
#include <zephyr/kernel.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/matrix.h>
#include <zmk/pm.h>

#define IS_SPLIT_PERIPHERAL                                                                        \
    (IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))

BUILD_ASSERT(ZMK_KEYMAP_LEN <= 64);

static struct k_spinlock state_lock;
static uint64_t pressed_keys;
static bool shutdown_pending;

static void soft_off_work_handler(struct k_work *work) {
    ARG_UNUSED(work);

    k_spinlock_key_t key = k_spin_lock(&state_lock);
    bool ready = shutdown_pending && pressed_keys == 0;
    if (ready) {
        shutdown_pending = false;
    }
    k_spin_unlock(&state_lock, key);

    if (ready) {
        zmk_pm_soft_off();
    }
}

K_WORK_DELAYABLE_DEFINE(soft_off_work, soft_off_work_handler);

/* Called with state_lock held. Let the matrix settle after the last release. */
static void schedule_soft_off_if_ready(void) {
    if (shutdown_pending && pressed_keys == 0) {
        k_work_reschedule(&soft_off_work, K_MSEC(20));
    }
}

static int track_local_keys(const zmk_event_t *event) {
    const struct zmk_position_state_changed *position = as_zmk_position_state_changed(event);
    if (position == NULL || position->source != ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL ||
        position->position >= ZMK_KEYMAP_LEN) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    k_spinlock_key_t key = k_spin_lock(&state_lock);
    uint64_t mask = BIT64(position->position);
    if (position->state) {
        pressed_keys |= mask;
    } else {
        pressed_keys &= ~mask;
        schedule_soft_off_if_ready();
    }
    k_spin_unlock(&state_lock, key);

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kometa_soft_off, track_local_keys);
ZMK_SUBSCRIPTION(kometa_soft_off, zmk_position_state_changed);

static void request_soft_off(void) {
    k_spinlock_key_t key = k_spin_lock(&state_lock);
    shutdown_pending = true;
    schedule_soft_off_if_ready();
    k_spin_unlock(&state_lock, key);
}

static int binding_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    /* The peripheral receives its request early, before the central turns off. */
    if (IS_SPLIT_PERIPHERAL) {
        request_soft_off();
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int binding_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    ARG_UNUSED(binding);
    ARG_UNUSED(event);

    if (!IS_SPLIT_PERIPHERAL) {
        request_soft_off();
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api soft_off_api = {
    .locality = BEHAVIOR_LOCALITY_GLOBAL,
    .binding_pressed = binding_pressed,
    .binding_released = binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define KOMETA_SOFT_OFF_DEFINE(n)                                                                   \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                 \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &soft_off_api);
DT_INST_FOREACH_STATUS_OKAY(KOMETA_SOFT_OFF_DEFINE)
