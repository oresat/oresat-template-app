/**
 * adc.c
 *
 * Original code came from zephyr/samples/driver/adc_sequence.
 *
 * Modified slightly to run as a thread and use logging.
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h> 
#include <zephyr/sys/__assert.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dsp/print_format.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(oresat_adc_demo, LOG_LEVEL_DBG);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* 1000 msec = 1 sec */
#define ADC_SLEEP_TIME_MS 100 /* Use 5 to generate more data to graph samples that follow the DAC output */

/* ADC node from the devicetree. */
#define ADC_NODE DT_ALIAS(adc0)

/* Auxiliary macro to obtain channel vref, if available. */
#define CHANNEL_VREF(node_id) DT_PROP_OR(node_id, zephyr_vref_mv, 0)

/* Data of ADC device specified in devicetree. */
static const struct device *adc = DEVICE_DT_GET(ADC_NODE);

/* Data array of ADC channels for the specified ADC. */
static const struct adc_channel_cfg channel_cfgs[] = {
	DT_FOREACH_CHILD_SEP(ADC_NODE, ADC_CHANNEL_CFG_DT, (,))};

/* Data array of ADC channel voltage references. */
static uint32_t vrefs_mv[] = {DT_FOREACH_CHILD_SEP(ADC_NODE, CHANNEL_VREF, (,))};

/* Get the number of channels defined on the DTS. */
#define CHANNEL_COUNT ARRAY_SIZE(channel_cfgs)

static int handle_adc(void)
{
	int err;
#ifdef CONFIG_SEQUENCE_32BITS_REGISTERS
	uint32_t channel_reading[CONFIG_SEQUENCE_SAMPLES][CHANNEL_COUNT];
#else
	uint16_t channel_reading[CONFIG_SEQUENCE_SAMPLES][CHANNEL_COUNT];
#endif

	/* Options for the sequence sampling. */
	const struct adc_sequence_options options = {
		.extra_samplings = CONFIG_SEQUENCE_SAMPLES - 1,
		.interval_us = 0,
	};

	/* Configure the sampling sequence to be made. */
	struct adc_sequence sequence = {
		.buffer = channel_reading,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(channel_reading),
		.resolution = CONFIG_SEQUENCE_RESOLUTION,
		.options = &options,
	};

	LOG_INF("Starting ADC demo");

	if (!device_is_ready(adc)) {
		LOG_ERR("ADC controller device %s not ready", adc->name);
		return 0;
	}

	LOG_INF("Channels: %d, sequence samples: %d, resolution: %d",
		   CHANNEL_COUNT, CONFIG_SEQUENCE_SAMPLES, CONFIG_SEQUENCE_RESOLUTION);

	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < CHANNEL_COUNT; i++) {
		sequence.channels |= BIT(channel_cfgs[i].channel_id);
		err = adc_channel_setup(adc, &channel_cfgs[i]);
		if (err < 0) {
			LOG_ERR("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
		if ((vrefs_mv[i] == 0) && (channel_cfgs[i].reference == ADC_REF_INTERNAL)) {
			vrefs_mv[i] = adc_ref_internal(adc);
		}
		LOG_INF("Channel: %u, vref_mv: %u, gain: %u, acq time: %u, diff: %u, inp_pos: %u, inp_neg: %u",
			   channel_cfgs[i].channel_id, vrefs_mv[i], channel_cfgs[i].gain, channel_cfgs[i].acquisition_time,
			   channel_cfgs[i].differential, channel_cfgs[i].input_positive, channel_cfgs[i].input_negative);
	}

	while (1) {
		k_msleep(ADC_SLEEP_TIME_MS);

		err = adc_read(adc, &sequence);
		if (err < 0) {
			LOG_ERR("Could not read (%d)", err);
			continue;
		}

		for (size_t channel_index = 0U; channel_index < CHANNEL_COUNT; channel_index++) {
			int32_t raw;
			int32_t val_mv;

			printk("Channel %" PRId32 ":", channel_index);

			for (size_t sample_index = 0U; sample_index < CONFIG_SEQUENCE_SAMPLES;
			     sample_index++) {

				raw = channel_reading[sample_index][channel_index];
				val_mv = raw;

				err = adc_raw_to_millivolts(vrefs_mv[channel_index],
							    channel_cfgs[channel_index].gain,
							    CONFIG_SEQUENCE_RESOLUTION, &val_mv);

				/* conversion to mV may not be supported, skip if not */
				if ((err < 0) || vrefs_mv[channel_index] == 0) {
					printk("% 4" PRId32 ",", raw);
				} else {
					printk("% 4" PRId32 ",", val_mv);
				}
			}
			printk("\n");
		}
	}

	return 0;
}

K_THREAD_DEFINE(adc_id, STACKSIZE, handle_adc, NULL, NULL, NULL, PRIORITY, 0, 0);
