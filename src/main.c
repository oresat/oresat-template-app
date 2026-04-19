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
#include <zephyr/device.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>

#include <CANopen.h>
#include <canopennode.h>

//#include <oresat.h>

LOG_MODULE_REGISTER(oresat_zephyr_template, LOG_LEVEL_DBG);

#define CAN_INTERFACE (DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus)))
#define CAN_BITRATE                                                                        \
    (DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bitrate,                                         \
     DT_PROP_OR(DT_CHOSEN(zephyr_canbus), bus_speed, CONFIG_CAN_DEFAULT_BITRATE)         / \
     1000))

static bool run_self_tests(void)
{
    LOG_INF("Running system self-tests...");
    
    // Placeholder for board/app-specific checks
    bool tests_passed = true; 

    return tests_passed;
}

int main(void)
{
    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;
    CO_ReturnError_t err;
    struct canopen_context can = {
        .dev = CAN_INTERFACE,
    };

    uint16_t timeout;
    uint32_t elapsed;
    int64_t timestamp;
    const uint8_t node_id = 0x2A;

    LOG_INF("Oresat template starting on board: %s", CONFIG_BOARD_TARGET);

    // Confirm a newly booted MCUboot image if self-tests pass
    if (!boot_is_img_confirmed()) {
        LOG_INF("New firmware detected. Pending confirmation.");
        
        if (run_self_tests()) {
            LOG_INF("Self-tests passed. Confirming image.");
            int rc = boot_write_img_confirmed();
            if (rc < 0) {
                LOG_ERR("Failed to confirm MCUboot image: %d", rc);
            }
        } else {
            LOG_ERR("Self-tests failed. Rebooting to revert firmware...");
            k_msleep(500); // Let logs flush
            sys_reboot(SYS_REBOOT_COLD);
        }
    } else {
        LOG_INF("Firmware already confirmed.");
    }

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

        // Main CANopen processing loop
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
