#include "low_power.h"
#include <string.h>
#include "hardware/powman.h"
#include "hardware/flash.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "pico/time.h"


powman_power_state off_state;
powman_power_state on_state;


/**
 * Initialize POWMAN timer and sleep/wake power states.
 *
 * Keep SRAM banks 0 and 1 powered in sleep so retained data survives,
 * and restore core, XIP cache, and SRAM on wake.
 */
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


/**
 * @brief Use the Pico 2 W power manager to power off most of the board
 * ("hibernate") and configure the wake up timer and the touch button as
 * the only interruptions.
 */
void hibernate(void) {

    save_retained_data_to_flash();

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

    // Turn off Pico 2 W wireless power signal.
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


/**
 * @brief Save retained data to flash.
 *
 * Update the reserved flash sector with the current retained data, disabling
 * interrupts during erase/program operations.
 */
void save_retained_data_to_flash() {
    uint8_t sector_buffer[FLASH_SECTOR_SIZE];

    memcpy(sector_buffer, (const void *)FLASH_READ_ADDR, FLASH_SECTOR_SIZE);

    memcpy(
        sector_buffer,
        data_retained_in_hibernation,
        sizeof(retained_data_t)
    );

    // Start critical section.
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, sector_buffer, FLASH_SECTOR_SIZE);

    // End critical section.
    restore_interrupts(ints);
}
