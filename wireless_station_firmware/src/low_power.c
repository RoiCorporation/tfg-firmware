#include "low_power.h"
#include "hardware/powman.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "pico/time.h"


powman_power_state off_state;
powman_power_state on_state;


void init_powman_states(void) {
    powman_timer_start();
    powman_timer_set_1khz_tick_source_lposc();

    off_state = POWMAN_POWER_STATE_NONE;
    off_state = powman_power_state_with_domain_on(off_state, POWMAN_POWER_DOMAIN_SRAM_BANK0);
    off_state = powman_power_state_with_domain_on(off_state, POWMAN_POWER_DOMAIN_SRAM_BANK1);
    
    on_state = POWMAN_POWER_STATE_NONE;
    on_state = powman_power_state_with_domain_on(on_state, POWMAN_POWER_DOMAIN_SWITCHED_CORE);
    on_state = powman_power_state_with_domain_on(on_state, POWMAN_POWER_DOMAIN_XIP_CACHE);
    on_state = powman_power_state_with_domain_on(on_state, POWMAN_POWER_DOMAIN_SRAM_BANK0);
    on_state = powman_power_state_with_domain_on(on_state, POWMAN_POWER_DOMAIN_SRAM_BANK1);
}


int8_t turn_low_power_on(void) {

    init_powman_states();
    multicore_reset_core1();
    set_sys_clock_khz(24000, true);
    clock_stop(clk_usb);
    clock_stop(clk_adc);

    for (int i = 0; i < NUM_BANK0_GPIOS; i++) {
        gpio_set_function(i, GPIO_FUNC_SIO);
        gpio_disable_pulls(i);
        gpio_set_input_enabled(i, false);
    }

    gpio_init(TOUCH_BUTTON_PIN);
    gpio_set_dir(TOUCH_BUTTON_PIN, GPIO_IN);
    gpio_pull_down(TOUCH_BUTTON_PIN);
    gpio_set_input_enabled(TOUCH_BUTTON_PIN, true);

    // Turn off Pico 2 W wireless power signal
    gpio_init(23);
    gpio_set_dir(23, GPIO_OUT);
    gpio_put(23, 0);

    sleep_ms(100);

    gpio_disable_pulls(TOUCH_BUTTON_PIN);
    powman_enable_gpio_wakeup(0, TOUCH_BUTTON_PIN, false, true);

    powman_enable_alarm_wakeup_at_ms(powman_timer_get_ms() + 15000);

    if (!powman_configure_wakeup_state(off_state, on_state)) {
        while (true)
            tight_loop_contents();
    }

    powman_hw->boot[0] = 0;
    powman_hw->boot[1] = 0;
    powman_hw->boot[2] = 0;
    powman_hw->boot[3] = 0;

    powman_set_power_state(off_state);
}
