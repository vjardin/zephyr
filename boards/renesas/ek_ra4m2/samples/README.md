# I2C Slave Example for EK-RA4M2

This directory contains a simple I2C slave example for the Renesas EK-RA4M2 development board.

## Overview

This example demonstrates how to use the I2C slave functionality on the RA4M2 board. The board will respond to I2C requests at address 0x50, allowing you to test communication with I2C master devices.

## Hardware Requirements

- EK-RA4M2 development board
- I2C master device (e.g., Raspberry Pi, another microcontroller, or I2C USB adapter)
- 2 × 4.7kΩ resistors (for I2C pull-ups)
- Breadboard and jumper wires

## Wiring Instructions

Connect the I2C lines as follows:

```
EK-RA4M2          I2C Master
--------          ----------
P402 (SCL1)  ↔   SCL
P403 (SDA1)  ↔   SDA
GND          ↔   GND
```

Add 4.7kΩ pull-up resistors between:
- SCL and 3.3V
- SDA and 3.3V

## Building and Running

### Prerequisites

1. Install the Zephyr SDK and development environment
2. Set up the RA4M2 board support as described in the main documentation

### Building

```bash
# Navigate to your Zephyr workspace
cd <zephyr-workspace>

# Create a build directory
mkdir -p build && cd build

# Configure for EK-RA4M2 with I2C slave support
west build -b ek_ra4m2 samples/renesas/ek_ra4m2/i2c_slave_example
```

### Configuration

Ensure your `prj.conf` or `board.conf` includes:

```kconfig
CONFIG_I2C_RENESAS_RA_IIC_SLAVE=y
CONFIG_USE_RA_FSP_I2C_IIC_SLAVE=y
```

### Flashing

```bash
west flash -r jlink
```

## Testing

### Using Linux I2C Tools

If you're using a Raspberry Pi or other Linux-based I2C master:

```bash
# Install i2c-tools if not already installed
sudo apt-get install i2c-tools

# Detect the I2C slave (should show 0x50)
sudo i2cdetect -y 1

# Read from the slave
i2cget -y 1 0x50

# Write to the slave
i2cset -y 1 0x50 0x01
```

### Expected Output

On the RA4M2 console, you should see:

```
[INF] Starting I2C Slave Example for EK-RA4M2
[INF] I2C slave device ready at address 0x50
[INF] I2C slave configured and waiting for master requests...
[INF] Read requested from master
[INF] Stop condition received
```

## Troubleshooting

### Device Not Detected

- **Check wiring**: Ensure SCL and SDA are connected correctly
- **Verify pull-up resistors**: Both SCL and SDA need 4.7kΩ pull-ups to 3.3V
- **Check voltage levels**: Ensure both devices are using 3.3V logic levels
- **Verify address**: Make sure no other device is using address 0x50

### Communication Errors

- **Signal integrity**: Use short wires and proper breadboard connections
- **Noise reduction**: Keep I2C lines away from power lines and noisy signals
- **Speed matching**: Ensure both master and slave are configured for the same speed

### Interrupt Issues

- **Interrupt conflicts**: Verify that interrupts 91-94 are not used by other peripherals
- **Priority settings**: Check that interrupt priorities are set appropriately

## Advanced Usage

### Changing the Slave Address

To change the I2C slave address, modify the device tree overlay or board configuration:

```dts
&iic1 {
    slave-address = <0x27>;  // Your desired address
};
```

### Handling Different Data Types

The example shows basic string handling. For binary data:

```c
// For binary data
case I2C_SLAVE_EVENT_WRITE_RECEIVED: {
    uint8_t binary_data[32];
    size_t length = i2c_slave_read(dev, binary_data, sizeof(binary_data));
    
    // Process binary data
    for (int i = 0; i < length; i++) {
        LOG_INF("Byte %d: 0x%02X", i, binary_data[i]);
    }
    break;
}
```

### Multiple Slave Addresses

The RA4M2 I2C peripheral supports multiple slave addresses. You can configure additional addresses in the device tree.

## Example Applications

### Simple I2C Slave Example

1. **Sensor Emulator**: Simulate temperature, humidity, or other sensors
2. **Configuration Storage**: Store and retrieve configuration data via I2C
3. **Inter-MCU Communication**: Communicate between multiple microcontrollers
4. **Debug Interface**: Provide debug information over I2C
5. **Bootloader**: Implement I2C-based firmware updates

### Combined Master/Slave Example

1. **I2C Bridge/Proxy**: Forward commands between host and sensors
2. **Sensor Hub**: Aggregate data from multiple sensors for host
3. **Protocol Translator**: Convert between different I2C protocols
4. **Dual-Bus Gateway**: Connect two separate I2C networks
5. **Remote Sensor Access**: Access sensors from a distant host

## Available Examples

### Simple I2C Slave Example

**File**: `i2c_slave_example.c`

A basic I2C slave implementation that responds to read/write requests from a master.

**Features**:
- Simple callback-based event handling
- Basic read/write operations
- Status reporting
- Error handling

### Combined Master/Slave Example

**File**: `i2c_combined_example.c`

An advanced example demonstrating simultaneous use of both I2C interfaces:
- IIC0 as master (communicates with sensors)
- IIC1 as slave (communicates with host)

**Features**:
- I2C bridge/proxy functionality
- Command forwarding between host and sensors
- Bidirectional data transfer
- Comprehensive logging
- Error recovery

## Technical Details

### Simple Slave Example

- **I2C Peripheral**: IIC1
- **Slave Address**: 0x50 (configurable)
- **Supported Speeds**: Standard (100 kbps), Fast (400 kbps)
- **Interrupts**: RXI, TXI, TEI, ERI
- **Buffer Size**: 32 bytes (configurable)

### Combined Example

- **Master Interface**: IIC0 (P400/SCL0, P401/SDA0)
- **Slave Interface**: IIC1 (P402/SCL1, P403/SDA1)
- **Master Speed**: Up to 1 MHz
- **Slave Speed**: Up to 400 kHz
- **Buffer Sizes**: 32 bytes each direction (configurable)

## References

- [RA4M2 Group User's Manual](https://www.renesas.com/us/en/document/man/ra4m2-group-users-manual-hardware)
- [EK-RA4M2 User's Manual](https://www.renesas.com/us/en/document/mat/ek-ra4m2-v1-users-manual)
- [Zephyr I2C API Documentation](https://docs.zephyrproject.org/latest/services/device_model/i2c.html)
- [Renesas FSP Documentation](https://github.com/renesas/fsp)

## License

This example is licensed under the Apache 2.0 license. See the LICENSE file for details.