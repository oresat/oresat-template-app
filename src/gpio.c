/**
 * gpio.c
 *
 * Show how to access GPIOs on a board.
 *
 */
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(oresat_gpio_demo, LOG_LEVEL_DBG);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY_GPIO 6

#define GPIO_SLEEP_TIME_MS 5000

/**************************************************/
#if defined(CONFIG_BOARD_MCXN947_PROTOCARD)
#define BP_NODE DT_NODELABEL(protogpios)

/**
 * Use X Macros to define board-specific GPIO definitions
 * (GPIO_DEFS).
 *
 * Define the raw data that lists test point number, port
 * number, and bit number in the port. For example, the first
 * line corresponds to TP84, which is connected to P0_3.
 *
 * Use that data in the next section to generate an enumeration
 * of all the test point names (e.g, TP84) and an array of
 * structures for storing the device tree gpio access objects as
 * well as ints for the three columns.
 *
 * This array of structs can be indexed with the enumeration
 * values to provide easy access to any specific test point
 * GPIO.
 **/
#define GPIO_DEFS \
	X(84, 0,  3) \
	X(85, 0,  4) \
	X(86, 0,  5) \
	X(78, 0, 18) \
	X(79, 0, 19) \
	X(80, 0, 20) \
	X(81, 0, 21) \
	X(82, 0, 22) \
	X(83, 0, 23) \
	X(68, 1,  0) \
	X(69, 1,  1) \
	X(70, 1,  2) \
	X(71, 1,  3) \
	X(72, 1,  4) \
	X(73, 1,  5) \
	X(74, 1,  6) \
	X(75, 1,  7) \
	X( 5, 1, 12) \
	X( 7, 1, 14) \
	X( 8, 1, 15) \
	X( 9, 2,  0) \
	X(10, 2,  1) \
	X(52, 3,  1) \
	X(60, 3, 12) \
	X(59, 3, 13) \
	X(58, 3, 14) \
	X(57, 3, 15) \
	X(56, 3, 16) \
	X(55, 3, 17) \
	X(62, 3, 20) \
	X(63, 3, 21) \
	X(29, 4,  1) \
	X(27, 4,  3) \
	X(26, 4,  4) \
	X(25, 4,  5) \
	X(22, 4,  6) \
	X(20, 4,  7) \
	X(41, 4, 19) \
	X(64, 5,  0) \
	X(65, 5,  1) \
	X(66, 5,  2) \
	X(67, 5,  3)

/**
 * Instantiate the X macro to use the ## token pasting operator
 * to generate enums for each test point name.
 *
 * Use this X macro by invoking GPIO_DEFS. This automatically
 * fills in a comma-separated list of enumeration names.
 *
 * We also automatically get a count of the number of test
 * points defined as NUM_GPIO_TPS.
 *
 * After this X macro is used, undefine it so another X macro
 * can be created to use the same raw data above.
 *
 * The resulting enums will be named, for example:
 *   TP84 for port 0 bit 3
 *   TP85 for port 0 bit 4
 * You can then use these names in your custom source code.
 *
 **/
#define X(tpnum, port, bit) TP ## tpnum,
typedef enum {
	GPIO_DEFS
	NUM_GPIO_TPS
} GPIO_TPS;
#undef X

/**
 * Define a structure to store the gpio_dt_spec, and integers to
 * store the tpnum, port, and bit.
 */
typedef struct gpio_tp {
	const struct gpio_dt_spec dt;
	int tpnum;
	int port;
	int bit;
} gpio_tp;

/**
 * Instantiate a new X macro to generate build-time
 * initialization data for each GPIO. It must be done at
 * build-time due to the way that the Zephyr device tree access
 * from C is done using macros.
 *
 * Use the X macro to generate initialization data for an array
 * of these structures, by invoking GPIO_DEFS again.
 *
 * As recommended, undefine X so it could be redefined later.
 *
 * An example initialization entry for the array is:
 *
 *   {GPIO_DT_SPEC_GET(BP_NODE, tp84_p0_3_gpios), 84, 0, 3},
 *
 * You can then access a specific gpio_tp structure in the
 * gpio_tp_array by enum name, such as:
 *
 *   gpio_tp *my_new_led = &gpio_tp_array[TP84];
 *
 */
#define X(tpnum, port, bit) {GPIO_DT_SPEC_GET(BP_NODE, tp ## tpnum ## _p ## port ## _ ## bit ## _gpios), tpnum, port, bit},
static const gpio_tp gpio_tp_array[NUM_GPIO_TPS] = {
	GPIO_DEFS
};
#undef X

static int gpios_init(void)
{
	/**
	 * The GPIO test points are configured by default as inputs with
	 * pulldowns. As an example of changing the configuration at
	 * runtime, rather than in the device tree at startup only, here
	 * we configure TP84, which is routed to Port 0 bit 3, as an
	 * output which is active.
	 * By setting TP84 to be an output, as shown below, you could
	 * hook up a new LED + resistor to TP84 and be able to turn it
	 * on and off.
	 */
#if 0
	const gpio_tp *my_new_led = &gpio_tp_array[TP84];
	int ret = gpio_pin_configure_dt(&my_new_led->dt, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);

	if (ret) {
	    return ret;
	}
#endif

#if 0
	// modify the handle_gpios() function below to also toggle the new LED

	const gpio_tp *my_new_led = &gpio_tp_array[TP84]; // <-- add this line

	for (;;) {
		gpios_log();
		k_msleep(1000);
		gpio_pin_toggle_dt(&my_new_led->dt); // <-- add this line
	}
#endif
	return 0;
}

static int gpios_log(void)
{
	/* enumerate all GPIOs and print their definitions as well as their currently read hardware value. */
	for (int i = 0; i < NUM_GPIO_TPS; i++) {
		LOG_INF("TP%d P%d_%d = %u", gpio_tp_array[i].tpnum, gpio_tp_array[i].port, gpio_tp_array[i].bit, gpio_pin_get_dt(&gpio_tp_array[i].dt));
	}
	return 0;
}

#else

static int gpios_init(void)
{
	LOG_INF("No GPIOs defined.");
	return 0;
}

static int gpios_log(void)
{
	return 0;
}

#endif


static int handle_gpios(void)
{
	int ret;

	LOG_INF("Starting GPIO demo");

	/* Can we use the GPIOs? */
	ret = gpios_init();

	if (ret != 0) {
		LOG_ERR("GPIO device(s) is not ready: %d", ret);
		return -1;
	}

	for (;;) {
		gpios_log();
		k_msleep(GPIO_SLEEP_TIME_MS);
	}

	return 0;
}

K_THREAD_DEFINE(gpio_id, STACKSIZE, handle_gpios, NULL, NULL, NULL, PRIORITY_GPIO, 0, 0);
