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
#include <CO_OD.h>
#include <board_sensors.h>
#include <oresat.h>

LOG_MODULE_REGISTER(oresat_zephyr_template, LOG_LEVEL_DBG);

#define CAN_INTERFACE (DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus)))
#define CAN_BITRATE (DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bitrate, \
					 DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bus_speed, \
								CONFIG_CAN_DEFAULT_BITRATE) / 1000))

int main(void)
{
	uint16_t timeout;
	uint32_t elapsed;
	int64_t timestamp;
	CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
	struct canopen_context can = {.dev = CAN_INTERFACE};
	uint8_t node_id = oresat_get_node_id();
	int err;

	LOG_INF("Oresat app template starting up on board: %s, node: %u", CONFIG_BOARD_TARGET, (unsigned int)node_id);

	oresat_fix_pdo_cob_ids(node_id);

	LOG_DBG("Opening CAN");
	//canopennode_init(CAN_INTERFACE, CAN_BITRATE, node_id);
	if (!device_is_ready(can.dev)) {
		LOG_ERR("CAN interface is not ready");
		return 0;
	}
	err = CO_init(&can, node_id, CAN_BITRATE);
	if (err != CO_ERROR_NO) {
		LOG_ERR("CO_init failed (err = %d)", err);
		return 0;
	}
	LOG_INF("CANopen stack initialized");

	LOG_DBG("Initializing sensors");
	board_sensors_init();

	LOG_DBG("Starting main loop");
	elapsed = 0U;

	// CO_OD_configure();
	CO_CANsetNormalMode(CO->CANmodule[0]);

	while (true) {
		timeout = 1000U;
		timestamp = k_uptime_get();
		reset = CO_process(CO, (uint16_t)elapsed, &timeout);
		if (reset != CO_RESET_NOT) {
			break;
		}

		if (timeout > 0) {
			board_sensors_fill_od();
			k_sleep(K_MSEC(timeout));
			elapsed = (uint32_t)k_uptime_delta;
		} else {
			elapsed = 0U;
		}
	}

	CO_delete(&can);
	LOG_DBG("Done.");
	sys_reboot(SYS_REBOOT_COLD);

	return 0;
}
