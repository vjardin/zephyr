/**************************************************************************************************
 * Copyright (c) 2024-2025 Renesas Electronics Corporation
 * Copyright (c) 2024-2025 Vincent Jardin, Free Mobile
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Combined I2C Master and Slave Example for EK-RA4M2
 *
 * This example demonstrates simultaneous use of IIC0 as master and IIC1 as slave.
 * The application acts as an I2C bridge/proxy between a host controller and I2C sensors.
 *
 * Wiring:
 * Master (IIC0):
 * - SCL0: P400 (Arduino D15) - Connect to I2C sensor SCL
 * - SDA0: P401 (Arduino D14) - Connect to I2C sensor SDA
 * 
 * Slave (IIC1):
 * - SCL1: P402 - Connect to host controller SCL
 * - SDA1: P403 - Connect to host controller SDA
 *
 * Both buses require 4.7kΩ pull-up resistors on SCL and SDA lines.
 *
 * Testing:
 * 1. Connect an I2C sensor (e.g., BME280) to IIC0 (master)
 * 2. Connect host (Raspberry Pi, etc.) to IIC1 (slave)
 * 3. Host can send commands to read sensor data through the RA4M2 bridge
 *
 * Expected Output:
 * [INF] I2C Bridge Example - RA4M2 acting as master and slave
 * [INF] Master device ready (IIC0)
 * [INF] Slave device ready (IIC1) at address 0x50
 * [INF] Bridge ready - waiting for host commands...
 * [INF] Received command from host: [0x10, 0x01]
 * [INF] Forwarding to sensor...
 * [INF] Sensor response: [0xAA, 0xBB, 0xCC, 0xDD]
 * [INF] Sent response to host
 **************************************************************************************************/

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_combined_example, LOG_LEVEL_INF);

// Command buffer for host communication
define COMMAND_BUFFER_SIZE 32
static uint8_t command_buffer[COMMAND_BUFFER_SIZE];
static size_t command_length = 0;

// Response buffer for sensor data
define RESPONSE_BUFFER_SIZE 32
static uint8_t response_buffer[RESPONSE_BUFFER_SIZE];
static size_t response_length = 0;

/**
 * @brief I2C Slave Callback Function
 *
 * Handles commands from the host and forwards them to sensors.
 *
 * @param dev Pointer to the I2C slave device
 * @param event I2C slave event type
 */
static void i2c_slave_callback(const struct device *dev, uint32_t event)
{
    switch (event) {
        case I2C_SLAVE_EVENT_WRITE_RECEIVED: {
            LOG_INF("Received command from host");
            
            // Read the command from host
            command_length = i2c_slave_read(dev, command_buffer, COMMAND_BUFFER_SIZE);
            LOG_INF("Command (%d bytes): [%.*H]", command_length, command_length, command_buffer);
            
            // First byte is typically the sensor address
            if (command_length > 0) {
                uint8_t sensor_addr = command_buffer[0];
                
                // Forward command to sensor (skip address byte for sensor)
                LOG_INF("Forwarding to sensor at 0x%02X...", sensor_addr);
                
                int ret = i2c_write(i2c_master_dev, &command_buffer[1], command_length - 1, sensor_addr);
                if (ret != 0) {
                    LOG_ERR("Failed to write to sensor: %d", ret);
                    break;
                }
                
                // Read response from sensor
                response_length = i2c_read(i2c_master_dev, response_buffer, RESPONSE_BUFFER_SIZE, sensor_addr);
                if (response_length > 0) {
                    LOG_INF("Sensor response (%d bytes): [%.*H]", 
                           response_length, response_length, response_buffer);
                    
                    // Send response back to host
                    i2c_slave_write(dev, response_buffer, response_length);
                    LOG_INF("Sent response to host");
                } else {
                    LOG_WRN("No response from sensor");
                }
            }
            break;
        }
        
        case I2C_SLAVE_EVENT_READ_REQUESTED: {
            LOG_INF("Host requested read - sending status");
            
            // Send bridge status information
            uint8_t status[] = "RA4M2 I2C Bridge Ready";
            i2c_slave_write(dev, status, sizeof(status) - 1);
            break;
        }
        
        case I2C_SLAVE_EVENT_STOP: {
            LOG_DBG("Stop condition received");
            break;
        }
        
        case I2C_SLAVE_EVENT_ADDRESS_NACK: {
            LOG_WRN("Address NACK - host may be scanning bus");
            break;
        }
        
        default: {
            LOG_DBG("Unknown slave event: %d", event);
            break;
        }
    }
}

/**
 * @brief Main Function
 *
 * Initializes both I2C master and slave interfaces.
 */
void main(void)
{
    int ret;

    LOG_INF("I2C Bridge Example - RA4M2 acting as master and slave");

    /* Initialize I2C Master (IIC0) */
    i2c_master_dev = DEVICE_DT_GET(DT_NODELABEL(iic0));
    
    if (!device_is_ready(i2c_master_dev)) {
        LOG_ERR("I2C master device not ready");
        return;
    }
    LOG_INF("Master device ready (IIC0)");

    /* Initialize I2C Slave (IIC1) */
    i2c_slave_dev = DEVICE_DT_GET(DT_NODELABEL(iic1));
    
    if (!device_is_ready(i2c_slave_dev)) {
        LOG_ERR("I2C slave device not ready");
        return;
    }
    LOG_INF("Slave device ready (IIC1) at address 0x50");

    /* Configure the I2C slave with our callback */
    ret = i2c_slave_configure(i2c_slave_dev, i2c_slave_callback);
    if (ret != 0) {
        LOG_ERR("Failed to configure I2C slave: %d", ret);
        return;
    }

    LOG_INF("Bridge ready - waiting for host commands...");

    /* Main loop - just wait for interrupts */
    while (1) {
        k_sleep(K_SECONDS(1));
        LOG_DBG("Bridge running...");
    }
}