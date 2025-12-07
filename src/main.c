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
#include <zephyr/sys/reboot.h>

#include <canopennode.h>
#include <OD.h>
#include <board_sensors.h>
#include <oresat.h>

LOG_MODULE_REGISTER(oresat_zephyr_template, LOG_LEVEL_DBG);

#define CAN_INTERFACE (DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus)))
#define CAN_BITRATE (DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bitrate, \
					 DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bus_speed, \
								CONFIG_CAN_DEFAULT_BITRATE) / 1000))

int main(void)
{
	k_timepoint_t timepoint;
	uint8_t node_id = oresat_get_node_id();

	LOG_INF("Oresat app template starting up on board: %s, node: %u", CONFIG_BOARD_TARGET, (unsigned int)node_id);

	oresat_fix_pdo_cob_ids(node_id);

	LOG_DBG("Opening CAN");
	canopennode_init(CAN_INTERFACE, CAN_BITRATE, node_id);
	LOG_DBG("Initializing sensors");
	board_sensors_init();

	LOG_DBG("Starting main loop");
	while (canopennode_is_running()) {
		timepoint = sys_timepoint_calc(K_MSEC(1000));
		board_sensors_fill_od();
		k_sleep(sys_timepoint_timeout(timepoint));
	}

	canopennode_stop(CAN_INTERFACE);
	sys_reboot(SYS_REBOOT_COLD);
	LOG_DBG("Done.");

	return 0;
}
