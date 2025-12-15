/**************************************************************************************************
 * Copyright (c) 2024-2025 Renesas Electronics Corporation
 * Copyright (c) 2024-2025 Vincent Jardin, Free Mobile
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Simple I2C Slave Example for EK-RA4M2
 *
 * This example demonstrates how to use the I2C slave functionality on the RA4M2 board.
 * The board will respond to I2C requests at address 0x50.
 *
 * Wiring:
 * - Connect SCL1 (P402) to your I2C master's SCL line
 * - Connect SDA1 (P403) to your I2C master's SDA line
 * - Add 4.7kΩ pull-up resistors on both SCL and SDA lines
 *
 * Testing:
 * Use i2c-tools on Linux:
 *   i2cdetect -y 1  # Should show device at 0x50
 *   i2cget -y 1 0x50  # Read from slave
 *   i2cset -y 1 0x50 0x01  # Write to slave
 *
 * Expected Output:
 * [INF] I2C slave device ready at address 0x50
 * [INF] Read requested from master
 * [INF] Stop condition received
 **************************************************************************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_slave_example, LOG_LEVEL_INF);

/* Simple data buffer for demonstration */
static uint8_t data_buffer[32] = {0};
static size_t data_length = 0;

/**
 * @brief I2C Slave Callback Function
 *
 * This function handles I2C slave events from the master.
 *
 * @param dev Pointer to the I2C device
 * @param event I2C slave event type
 */
static void i2c_slave_callback(const struct device *dev, uint32_t event)
{
    switch (event) {
        case I2C_SLAVE_EVENT_READ_REQUESTED: {
            LOG_INF("Read requested from master");
            
            // Prepare some sample data to send
            uint8_t sample_data[] = "Hello from RA4M2 I2C Slave!";
            data_length = sizeof(sample_data) - 1;
            memcpy(data_buffer, sample_data, data_length);
            
            // Send the data
            i2c_slave_write(dev, data_buffer, data_length);
            break;
        }
        
        case I2C_SLAVE_EVENT_WRITE_RECEIVED: {
            uint8_t received_data[32];
            size_t received_length = i2c_slave_read(dev, received_data, sizeof(received_data));
            
            LOG_INF("Received %d bytes from master: %.*s", 
                   received_length, received_length, received_data);
            break;
        }
        
        case I2C_SLAVE_EVENT_STOP: {
            LOG_INF("Stop condition received");
            break;
        }
        
        case I2C_SLAVE_EVENT_ADDRESS_NACK: {
            LOG_WRN("Address NACK received");
            break;
        }
        
        default: {
            LOG_INF("Unknown event: %d", event);
            break;
        }
    }
}

/**
 * @brief Main Function
 *
 * Initializes the I2C slave and starts the example.
 */
void main(void)
{
    const struct device *i2c_slave_dev;
    int ret;

    LOG_INF("Starting I2C Slave Example for EK-RA4M2");

    /* Get the I2C slave device */
    i2c_slave_dev = DEVICE_DT_GET(DT_NODELABEL(iic1));
    
    if (!device_is_ready(i2c_slave_dev)) {
        LOG_ERR("I2C slave device not ready");
        return;
    }

    LOG_INF("I2C slave device ready at address 0x50");

    /* Configure the I2C slave with our callback */
    ret = i2c_slave_configure(i2c_slave_dev, i2c_slave_callback);
    if (ret != 0) {
        LOG_ERR("Failed to configure I2C slave: %d", ret);
        return;
    }

    LOG_INF("I2C slave configured and waiting for master requests...");

    /* Main loop - just wait for interrupts */
    while (1) {
        k_sleep(K_SECONDS(1));
        LOG_DBG("I2C slave running...");
    }
}