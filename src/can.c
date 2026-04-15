#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <canopennode.h>
#include <CO_OD.h>
#include <board_sensors.h>
#include <oresat.h>

LOG_MODULE_REGISTER(can_thread, LOG_LEVEL_DBG);

#define CAN_INTERFACE (DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus)))
#define CAN_BITRATE (DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bitrate, \
					 DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bus_speed, \
								CONFIG_CAN_DEFAULT_BITRATE) / 1000))

#define CAN_THREAD_STACK_SIZE 2048
#define CAN_THREAD_PRIORITY 0
extern const k_tid_t can_id;

static void handle_can(void *p1, void *p2, void *p3)
{
	int err;
	uint16_t timeout;
	uint16_t wr_timeout_count;
	uint32_t elapsed = 0U;
	int64_t timestamp;
	CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
	struct canopen_context can = {.dev = CAN_INTERFACE};
	uint8_t node_id = oresat_get_node_id();

	k_thread_name_set(can_id, "can_thread");

	LOG_INF("Starting CAN thread");

	oresat_fix_pdo_cob_ids(node_id);

	LOG_DBG("Opening CAN device");
	if (!device_is_ready(can.dev)) {
		LOG_ERR("CAN interface is not ready");
		k_msleep(1000);
		__ASSERT(false, "Fatal error");
	}

	LOG_INF("Starting CANopenNode (node_id=%u, bitrate=%u kbps)",
			(unsigned)node_id, (unsigned)CAN_BITRATE);

	while (reset != CO_RESET_APP) {
		elapsed = 0U;
		wr_timeout_count = 0U;

		err = CO_init(&can, node_id, CAN_BITRATE);
		if (err != CO_ERROR_NO) {
			LOG_ERR("CO_init failed (err = %d)", err);
			k_msleep(1000);
			__ASSERT(false, "Fatal error");
		}

		canopen_program_download_attach(CO->NMT, CO->SDO[0], CO->em);
		CO_CANsetNormalMode(CO->CANmodule[0]);

		LOG_INF("Template app running. Waiting for CANopen requests...");

		while (true) {
			bool_t syncWas = false;

			timeout = 1U;
			timestamp = k_uptime_get();

			if (wr_timeout_count++ >= 1000U) {
				wr_timeout_count = 0U;

				CO_LOCK_OD();
				CO_OD_RAM.pack_1.cycles++;
				CO_OD_RAM.pack_1.current = 100;
				CO_OD_RAM.pack_1.full_capacity = 1000;
				CO_OD_RAM.pack_1.status = 13;
				CO_OD_RAM.pack_1.temperature = 21;
				CO_OD_RAM.pack_1.vbatt = 79;
				CO_OD_RAM.pack_2.cycles++;
				CO_OD_RAM.pack_2.current = 200;
				CO_OD_RAM.pack_2.full_capacity = 1500;
				CO_OD_RAM.pack_2.status = 17;
				CO_OD_RAM.pack_2.temperature = 23;
				CO_OD_RAM.pack_2.vbatt = 84;
				CO_UNLOCK_OD();

				/* Read inputs */
				CO_process_RPDO(CO, syncWas);

				/* Write outputs */
				CO_process_TPDO(CO, syncWas, timeout * 1000U * 1000U);
			}

			reset = CO_process(CO, (uint16_t)elapsed, &timeout);
			if (reset != CO_RESET_NOT) {
				break;
			}

			if (timeout > 0) {
				k_sleep(K_MSEC(timeout));
				elapsed = (uint32_t)k_uptime_delta(&timestamp);
			} else {
				elapsed = 0U;
			}

			if (reset == CO_RESET_COMM) {
				LOG_INF("Resetting communication");
			}
		}
	}

	CO_delete(&can);
	sys_reboot(SYS_REBOOT_COLD);
}


#if 0

#define CAN_INTERFACE (DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus)))
#define CAN_BITRATE 																	   \
	(DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bitrate,  									   \
	 DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bus_speed, CONFIG_CAN_DEFAULT_BITRATE)		 / \
	 1000))


int do_can(void)
{
	uint16_t timeout;
	uint32_t elapsed;
	int64_t timestamp;
	const uint8_t node_id = 0x2A;
	CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
	CO_ReturnError_t err;
	struct canopen_context can = {
		.dev = CAN_INTERFACE,
	};

	LOG_INF("Starting CANopenNode (node_id=%u, bitrate=%u kbps)",
			(unsigned)node_id, (unsigned)CAN_BITRATE);

	if (!device_is_ready(can.dev)) {
		LOG_ERR("CAN device not ready");
		return 0;
	}

	while (reset != CO_RESET_APP) {
		elapsed = 0;

		err = CO_init(&can, node_id, CAN_BITRATE);
		if (err != CO_ERROR_NO) {
			LOG_ERR("CO_init failed: %d", err);
			return 0;
		}

		canopen_program_download_attach(CO->NMT, CO->SDO[0], CO->em);

		CO_CANsetNormalMode(CO->CANmodule[0]);

		printk("Template app running. Waiting for CANopen requests...\r\n");

		while (true) {
			timeout = 1;
			timestamp = k_uptime_get();

			reset = CO_process(CO, (uint16_t)elapsed, &timeout);
			if (reset != CO_RESET_NOT) {
				break;
			}

			if (timeout > 0) {
				k_sleep(K_MSEC(timeout));
				elapsed = (uint32_t)k_uptime_delta(&timestamp);
			} else {
				elapsed = 0;
			}
		}

		if (reset == CO_RESET_COMM) {
			LOG_INF("Resetting communication");
		}
	}

	LOG_WRN("CANopenNode stopped. Rebooting");
	CO_delete(&can);
	sys_reboot(SYS_REBOOT_COLD);
	return 0;
}
#endif



K_THREAD_DEFINE(can_id, CAN_THREAD_STACK_SIZE, handle_can, NULL, NULL, NULL, CAN_THREAD_PRIORITY, 0, 0);
