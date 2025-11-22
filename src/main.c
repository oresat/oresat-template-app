/**
 * main.c
 *
 * Simple main that only logs a bootup message. The remainder
 * of the demos are implemented as independent threads
 * in blink.c, dac.c, i2c_sensor.c, and adc.c.
 *
 * These can be disabled at compile time by adding:
 *   CONFIG_BLINK_DEMO=n
 * for example, to prj.conf. See Kconfig for the options or run
 * west build -t menuconfig for an interacive configuration
 * editor.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(oresat_zephyr_template, LOG_LEVEL_DBG);

int main(void)
{
    LOG_INF("Oresat app template starting up on board: %s", CONFIG_BOARD_TARGET);

    printk("Logging from printk\r\n");
    return 0;
}


