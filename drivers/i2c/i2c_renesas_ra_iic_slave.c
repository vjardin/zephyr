/*
 * Copyright (c) 2024-2025 Renesas Electronics Corporation
 * Copyright (c) 2024-2025 Vincent Jardin, Free Mobile
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT renesas_ra_iic_slave

#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/irq.h>
#include <zephyr/sys/util.h>

#include "r_iic_slave.h"
#include <errno.h>
#include <soc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(renesas_ra_iic_slave, CONFIG_I2C_LOG_LEVEL);

/* Declare the FSP ISR functions */
extern void iic_slave_rxi_isr(void);
extern void iic_slave_txi_isr(void);
extern void iic_slave_tei_isr(void);
extern void iic_slave_eri_isr(void);

struct i2c_ra_iic_slave_config {
	void (*irq_config_func)(const struct device *dev);
	const struct pinctrl_dev_config *pcfg;
	uint8_t channel;
	uint32_t slave_address;
	uint8_t rxi_irq;
	uint8_t txi_irq;
	uint8_t tei_irq;
	uint8_t eri_irq;
};

struct i2c_ra_iic_slave_data {
	iic_slave_instance_ctrl_t ctrl;
	i2c_slave_cfg_t fsp_config;
	iic_slave_extended_cfg_t fsp_ext_config;
	struct i2c_target_config *target_config;
	const struct device *dev;
};

/* FSP callback handler */
static void iic_slave_callback(i2c_slave_callback_args_t *p_args)
{
	struct i2c_ra_iic_slave_data *data =
		CONTAINER_OF(p_args->p_context, struct i2c_ra_iic_slave_data, ctrl);

	if (data->target_config == NULL || data->target_config->callbacks == NULL) {
		return;
	}

	const struct i2c_target_callbacks *cbs = data->target_config->callbacks;

	switch (p_args->event) {
	case I2C_SLAVE_EVENT_RX_REQUEST:
	case I2C_SLAVE_EVENT_GENERAL_CALL:
		if (cbs->write_requested) {
			cbs->write_requested(data->target_config);
		}
		break;
	case I2C_SLAVE_EVENT_TX_REQUEST:
		if (cbs->read_requested) {
			uint8_t val;
			cbs->read_requested(data->target_config, &val);
		}
		break;
	case I2C_SLAVE_EVENT_RX_COMPLETE:
		if (cbs->stop) {
			cbs->stop(data->target_config);
		}
		break;
	case I2C_SLAVE_EVENT_TX_COMPLETE:
		if (cbs->stop) {
			cbs->stop(data->target_config);
		}
		break;
	case I2C_SLAVE_EVENT_ABORTED:
		if (cbs->stop) {
			cbs->stop(data->target_config);
		}
		break;
	default:
		break;
	}
}

static int i2c_ra_iic_slave_target_register(const struct device *dev,
					    struct i2c_target_config *cfg)
{
	struct i2c_ra_iic_slave_data *data = dev->data;
	fsp_err_t err;

	if (data->target_config != NULL) {
		return -EBUSY;
	}

	data->target_config = cfg;

	/* Update callback */
	err = R_IIC_SLAVE_CallbackSet(&data->ctrl, iic_slave_callback,
				      &data->ctrl, NULL);
	if (err != FSP_SUCCESS) {
		data->target_config = NULL;
		return -EIO;
	}

	return 0;
}

static int i2c_ra_iic_slave_target_unregister(const struct device *dev,
					      struct i2c_target_config *cfg)
{
	struct i2c_ra_iic_slave_data *data = dev->data;

	if (data->target_config != cfg) {
		return -EINVAL;
	}

	data->target_config = NULL;
	return 0;
}

static int i2c_ra_iic_slave_init(const struct device *dev)
{
	const struct i2c_ra_iic_slave_config *config = dev->config;
	struct i2c_ra_iic_slave_data *data = dev->data;
	fsp_err_t err;
	int ret;

	LOG_DBG("Initializing I2C slave device %s", dev->name);

	data->dev = dev;

	/* Configure pinctrl */
	ret = pinctrl_apply_state(config->pcfg, PINCTRL_STATE_DEFAULT);
	if (ret < 0) {
		LOG_ERR("Failed to apply pinctrl state: %d", ret);
		return ret;
	}

	/* Initialize FSP configuration */
	data->fsp_config.channel = config->channel;
	data->fsp_config.rate = I2C_SLAVE_RATE_STANDARD;
	data->fsp_config.slave = config->slave_address;
	data->fsp_config.addr_mode = I2C_SLAVE_ADDR_MODE_7BIT;
	data->fsp_config.general_call_enable = false;
	data->fsp_config.clock_stretching_enable = true;
	data->fsp_config.ipl = 12;
	data->fsp_config.eri_ipl = 12;
	data->fsp_config.rxi_irq = config->rxi_irq;
	data->fsp_config.txi_irq = config->txi_irq;
	data->fsp_config.tei_irq = config->tei_irq;
	data->fsp_config.eri_irq = config->eri_irq;
	data->fsp_config.p_callback = iic_slave_callback;
	data->fsp_config.p_context = &data->ctrl;
	data->fsp_config.p_transfer_tx = NULL;
	data->fsp_config.p_transfer_rx = NULL;
	data->fsp_config.p_extend = &data->fsp_ext_config;

	/* Initialize extended config */
	data->fsp_ext_config.clock_settings.cks_value = 0;
	data->fsp_ext_config.clock_settings.brl_value = 0x1F;
	data->fsp_ext_config.clock_settings.digital_filter_stages = 1;

	/* Configure IRQs first */
	if (config->irq_config_func) {
		config->irq_config_func(dev);
	}

	/* Open the I2C slave driver */
	err = R_IIC_SLAVE_Open(&data->ctrl, &data->fsp_config);
	if (err != FSP_SUCCESS) {
		LOG_ERR("Failed to open I2C slave driver: %d", err);
		return -EIO;
	}

	LOG_INF("I2C slave initialized on channel %d at address 0x%02x",
		config->channel, config->slave_address);

	return 0;
}

static const struct i2c_driver_api i2c_ra_iic_slave_api = {
	.target_register = i2c_ra_iic_slave_target_register,
	.target_unregister = i2c_ra_iic_slave_target_unregister,
};

#define IIC_SLAVE_INIT(index)                                                              \
	PINCTRL_DT_INST_DEFINE(index);                                                     \
	static void i2c_ra_iic_slave_irq_config_func##index(const struct device *dev);     \
                                                                                           \
	static const struct i2c_ra_iic_slave_config i2c_ra_iic_slave_config_##index = {    \
		.irq_config_func = i2c_ra_iic_slave_irq_config_func##index,                \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(index),                             \
		.channel = DT_INST_PROP(index, channel),                                   \
		.slave_address = DT_INST_PROP(index, slave_address),                       \
		.rxi_irq = DT_INST_IRQ_BY_NAME(index, rxi, irq),                           \
		.txi_irq = DT_INST_IRQ_BY_NAME(index, txi, irq),                           \
		.tei_irq = DT_INST_IRQ_BY_NAME(index, tei, irq),                           \
		.eri_irq = DT_INST_IRQ_BY_NAME(index, eri, irq),                           \
	};                                                                                 \
                                                                                           \
	static struct i2c_ra_iic_slave_data i2c_ra_iic_slave_data_##index;                 \
                                                                                           \
	DEVICE_DT_INST_DEFINE(index, i2c_ra_iic_slave_init, NULL,                          \
			      &i2c_ra_iic_slave_data_##index,                              \
			      &i2c_ra_iic_slave_config_##index,                            \
			      POST_KERNEL, CONFIG_I2C_INIT_PRIORITY,                       \
			      &i2c_ra_iic_slave_api);                                      \
                                                                                           \
	static void i2c_ra_iic_slave_irq_config_func##index(const struct device *dev)      \
	{                                                                                  \
		ARG_UNUSED(dev);                                                           \
		IRQ_CONNECT(DT_INST_IRQ_BY_NAME(index, rxi, irq),                          \
			    DT_INST_IRQ_BY_NAME(index, rxi, priority),                     \
			    iic_slave_rxi_isr, NULL, 0);                                   \
		IRQ_CONNECT(DT_INST_IRQ_BY_NAME(index, txi, irq),                          \
			    DT_INST_IRQ_BY_NAME(index, txi, priority),                     \
			    iic_slave_txi_isr, NULL, 0);                                   \
		IRQ_CONNECT(DT_INST_IRQ_BY_NAME(index, tei, irq),                          \
			    DT_INST_IRQ_BY_NAME(index, tei, priority),                     \
			    iic_slave_tei_isr, NULL, 0);                                   \
		IRQ_CONNECT(DT_INST_IRQ_BY_NAME(index, eri, irq),                          \
			    DT_INST_IRQ_BY_NAME(index, eri, priority),                     \
			    iic_slave_eri_isr, NULL, 0);                                   \
                                                                                           \
		irq_enable(DT_INST_IRQ_BY_NAME(index, rxi, irq));                          \
		irq_enable(DT_INST_IRQ_BY_NAME(index, txi, irq));                          \
		irq_enable(DT_INST_IRQ_BY_NAME(index, tei, irq));                          \
		irq_enable(DT_INST_IRQ_BY_NAME(index, eri, irq));                          \
	}

DT_INST_FOREACH_STATUS_OKAY(IIC_SLAVE_INIT)
