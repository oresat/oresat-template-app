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

int main(void)
{
    //const uint8_t node_id = oresat_get_node_id();
    const uint8_t node_id = 0x2A;

    LOG_INF("Oresat template starting on board: %s", CONFIG_BOARD_TARGET);
    LOG_INF("Starting CANopenNode (node_id=%u, bitrate=%u kbps)",
        (unsigned)node_id, (unsigned)CAN_BITRATE);

    if (!device_is_ready(CAN_INTERFACE)) {
        LOG_ERR("CAN device not ready");
        return 0;
    }

    //canopennode_init(CAN_INTERFACE, CAN_BITRATE, node_id);
    CO_ReturnError_t err = CO_init((void *)CAN_INTERFACE, node_id, CAN_BITRATE);
    if (err != CO_ERROR_NO) { ; }

    //canopen_program_download_attach(CO->NMT, CO->SDOserver, CO->em);
    canopen_program_download_attach(CO->NMT, CO->SDO[0], CO->em);

    printk("Template app running. Waiting for CANopen requests...\r\n");

    //while (canopennode_is_running()) {
    //    k_sleep(K_MSEC(1000));
    //}
    uint16_t next_ms = 50;
    while (1) {
        CO_NMT_reset_cmd_t r = CO_process(CO, 1, &next_ms);
        if (r != CO_RESET_NOT) {
            break;
        }
        k_sleep(K_MSEC(1));
    }

    LOG_WRN("CANopenNode stopped; rebooting");
    //canopennode_stop(CAN_INTERFACE);
    CO_delete((void *)CAN_INTERFACE);
    sys_reboot(SYS_REBOOT_COLD);

    return 0;
}


