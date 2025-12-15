/*
 * Copyright (c) 2024-2025 Renesas Electronics Corporation
 * Copyright (c) 2024-2025 Vincent Jardin, Free Mobile
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT renesas_ra_iic_b_slave

#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/irq.h>
#include <zephyr/sys/util.h>

#include "r_iic_b_slave.h"
#include <errno.h>
#include <soc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(renesas_ra_iic_b_slave, CONFIG_I2C_LOG_LEVEL);

struct i2c_ra_iic_b_slave_config {
	void (*irq_config_func)(const struct device *dev);
	const struct pinctrl_dev_config *pcfg;
};

struct i2c_ra_iic_b_slave_data {
	iic_b_slave_instance_ctrl_t ctrl;
	i2c_slave_cfg_t fsp_config;
};

static int i2c_ra_iic_b_slave_init(const struct device *dev)
{
	const struct i2c_ra_iic_b_slave_config *config = dev->config;
	struct i2c_ra_iic_b_slave_data *data = dev->data;
	fsp_err_t err;

	LOG_DBG("Initializing IIC-B slave device %s", dev->name);

	/* Initialize FSP configuration */
	data->fsp_config.p_callback = NULL;
	data->fsp_config.p_context = dev;
	data->fsp_config.p_transfer_tx = NULL;
	data->fsp_config.p_transfer_rx = NULL;

	/* Open the IIC-B slave driver */
	err = R_IIC_B_SLAVE_Open(&data->ctrl, &data->fsp_config);
	if (err != FSP_SUCCESS) {
		LOG_ERR("Failed to open IIC-B slave driver: %d", err);
		return -EIO;
	}

	/* Configure IRQ */
	if (config->irq_config_func) {
		config->irq_config_func(dev);
	}

	return 0;
}

static const struct i2c_driver_api i2c_ra_iic_b_slave_api = {
	/* Implement I2C slave API functions */
};

#define IIC_B_SLAVE_INIT(n) \
	PINCTRL_DT_INST_DEFINE(n); \
	static void irq_config_func_##n(const struct device *dev); \
	static const struct i2c_ra_iic_b_slave_config i2c_ra_iic_b_slave_config_##n = { \
		.irq_config_func = irq_config_func_##n, \
		.pcfg = PINCTRL_DT_INST_DEV_CONFIG_GET(n), \
	}; \
	static struct i2c_ra_iic_b_slave_data i2c_ra_iic_b_slave_data_##n; \
	DEVICE_DT_INST_DEFINE(n, i2c_ra_iic_b_slave_init, NULL, \
		&i2c_ra_iic_b_slave_data_##n, &i2c_ra_iic_b_slave_config_##n, \
		POST_KERNEL, CONFIG_I2C_INIT_PRIORITY, &i2c_ra_iic_b_slave_api); \
	static void irq_config_func_##n(const struct device *dev) \
	{ \
		IRQ_CONNECT(DT_INST_IRQN(n), DT_INST_IRQ(n, priority), \
			NULL, NULL, 0); \
		irq_enable(DT_INST_IRQN(n)); \
	}

DT_INST_FOREACH_STATUS_OKAY(IIC_B_SLAVE_INIT)