.. zephyr:board:: ek_ra4m2

Overview
********

The Renesas RA4M2 group of 32-bit microcontrollers (MCUs) uses the high-performance Arm
Cortex®-M33 core. In concert with the secure crypto engine, it offers secure element
functionality. The RA4M2 is built on a highly efficient 40nm process, built on FreeRTOS—and
is expandable to use other RTOSes and middleware. The RA4M2 is suitable for IoT applications
requiring vast communication options, future proof security, large embedded RAM, and low
active power consumption down to 81µA/MHz running the CoreMark® algorithm from Flash.

The MCU in this series incorporates a high-performance Arm Cortex®-M33 core running up to
100 MHz with the following features:

**Renesas RA4M2 Microcontroller Group**

- R7FA4M2AD3CFP
- 100-pin LQFP package
- 100 MHz Arm® Cortex®-M33 core
- 512 kB Code Flash, 128 KB SRAM
- Native pin access through 4 x 28-pin male headers
- MCU current measurement points for precision current consumption measurement
- Multiple clock sources - RA MCU oscillator and sub-clock oscillator crystals, providing
  precision 24.000 MHz and 32,768 Hz reference clock. Additional low-precision clocks are
  available internal to the RA MCU

**System Control and Ecosystem Access**

- USB Full Speed Host and Device (micro AB connector)
- Three 5 V input sources

	- USB (Debug, Full Speed)
	- External power supply (using surface mount clamp test points and power input vias)

- Three Debug modes

	- Debug on-board (SWD)
	- Debug in (ETM, SWD, and JTAG)
	- Debug out (SWD)

- User LEDs and buttons

	- Three User LEDs (red, blue, green)
	- Power LED (white) indicating availability of regulated power
	- Debug LED (yellow) indicating the debug connection
	- Two User buttons
	- One Reset button

- Five most popular ecosystems expansions

	- 2 Seeed Grove® system (I2C/Analog) connectors
	- SparkFun® Qwiic® connector
	- 2 Digilent PmodTM (SPI and UART) connectors
	- ArduinoTM (Uno R3) connector
	- MikroElektronikaTM mikroBUS connector

- MCU boot configuration jumper

**Special Feature Access**

- 32 MB (256 Mb) External Quad-SPI Flash

Hardware
********
Detailed hardware features for the RA4M2 MCU group can be found at `RA4M2 Group User's Manual Hardware`_

.. figure:: ra4m2_block_diagram.webp
	:width: 442px
	:align: center
	:alt: RA4M2 MCU group feature

	RA4M2 Block diagram (Credit: Renesas Electronics Corporation)

Detailed hardware features for the EK-RA4M2 MCU can be found at `EK-RA4M2 - User's Manual`_


Supported Features
==================

.. zephyr:board-supported-hw::

I2C Master and Slave Support
============================

The EK-RA4M2 board provides comprehensive I2C support with both master and slave capabilities,
allowing for flexible I2C communication scenarios including multi-device networks and
debugging interfaces.

I2C Master Support
------------------

The board includes I2C master functionality through the IIC0 peripheral, which is ideal for
communicating with I2C sensors, EEPROMs, and other peripheral devices.

**Hardware Configuration**:

- **IIC0 Peripheral**: Configured as I2C master
- **Pins**:
  - SCL0: P400 (Port 4, Pin 0) - Arduino D15
  - SDA0: P401 (Port 4, Pin 1) - Arduino D14
- **Speed**: Configurable up to 1 MHz (Fast Mode Plus)
- **Interrupts**: RXI, TXI, TEI, ERI
- **Arduino Compatible**: Available on Arduino Uno R3 headers

**Getting Started with I2C Master**:

1. **Enable I2C Master in your configuration**:

   .. code-block:: kconfig

      CONFIG_I2C_RENESAS_RA_IIC=y
      CONFIG_USE_RA_FSP_I2C_IIC=y

2. **Access the I2C master device**:

   .. code-block:: c

      #include <zephyr/drivers/i2c.h>

      const struct device *i2c_master_dev = DEVICE_DT_GET(DT_NODELABEL(iic0));

      if (!device_is_ready(i2c_master_dev)) {
          printk("I2C master device not ready\n");
          return;
      }

3. **Basic I2C Master Operations**:

   .. code-block:: c

      // Write data to I2C slave device
      uint8_t write_data[] = {0x01, 0x02, 0x03};
      i2c_write(i2c_master_dev, write_data, sizeof(write_data), 0x50);

      // Read data from I2C slave device
      uint8_t read_data[10];
      i2c_read(i2c_master_dev, read_data, sizeof(read_data), 0x50);

4. **Advanced I2C Master Example**:

   .. code-block:: c

      // Write then read (common for sensor registers)
      uint8_t reg_addr = 0x10;  // Register address
      uint8_t reg_data[2];      // Data buffer

      // Write register address
      i2c_write(i2c_master_dev, &reg_addr, 1, 0x50);

      // Read register data
      i2c_read(i2c_master_dev, reg_data, sizeof(reg_data), 0x50);

**Common I2C Master Use Cases**:

1. **Sensor Communication**: Reading temperature, humidity, pressure sensors
2. **EEPROM Access**: Storing and retrieving configuration data
3. **RTC Configuration**: Setting and reading real-time clocks
4. **Display Control**: Driving OLED or LCD displays
5. **Multi-Device Networks**: Managing multiple I2C devices on the same bus

**I2C Master Wiring Example**:

.. code-block:: text

   EK-RA4M2 (Master)  ↔  I2C Sensor (Slave)
   ------------------    -------------------
   P400 (SCL0/D15)     ↔  SCL
   P401 (SDA0/D14)     ↔  SDA
   GND                 ↔  GND
   3.3V                ↔  VCC (if needed)

   Don't forget 4.7kΩ pull-up resistors on SCL and SDA!

I2C Slave Support
=================

The EK-RA4M2 board supports I2C slave functionality through the IIC1 peripheral. This allows
the board to act as an I2C slave device, responding to requests from I2C master devices.

Hardware Configuration
----------------------

- **IIC1 Peripheral**: Configured as I2C slave
- **Slave Address**: 0x50 (configurable)
- **Pins**:
  - SCL1: P402 (Port 4, Pin 2)
  - SDA1: P403 (Port 4, Pin 3)
- **Interrupts**: Configured for receive, transmit, transfer end, and error events

Getting Started with I2C Slave
------------------------------

To use the I2C slave functionality, follow these steps:

1. **Enable I2C Slave in your configuration**:

   Add these configuration options to your ``prj.conf`` or ``board.conf``:

   .. code-block:: kconfig

      CONFIG_I2C_RENESAS_RA_IIC_SLAVE=y
      CONFIG_USE_RA_FSP_I2C_IIC_SLAVE=y

2. **Access the I2C slave device in your application**:

   .. code-block:: c

      #include <zephyr/drivers/i2c.h>

      const struct device *i2c_slave_dev = DEVICE_DT_GET(DT_NODELABEL(iic1));

      if (!device_is_ready(i2c_slave_dev)) {
          printk("I2C slave device not ready\n");
          return;
      }

      printk("I2C slave device ready at address 0x50\n");

3. **Implement I2C slave callbacks** (example):

   .. code-block:: c

      static void i2c_slave_callback(const struct device *dev, uint32_t event)
      {
          switch (event) {
              case I2C_SLAVE_EVENT_READ_REQUESTED:
                  printk("Read requested from master\n");
                  // Prepare data to send
                  break;
              case I2C_SLAVE_EVENT_WRITE_RECEIVED:
                  printk("Data received from master\n");
                  // Process received data
                  break;
              case I2C_SLAVE_EVENT_STOP:
                  printk("Stop condition received\n");
                  break;
              default:
                  printk("Unknown event: %d\n", event);
                  break;
          }
      }

4. **Configure the slave device**:

   .. code-block:: c

      // Set up the slave with your callback
      i2c_slave_configure(i2c_slave_dev, i2c_slave_callback);

Wiring and Testing
------------------

To test the I2C slave functionality:

1. **Connect the I2C lines**:
   - Connect SCL1 (P402) to your I2C master's SCL line
   - Connect SDA1 (P403) to your I2C master's SDA line
   - Add pull-up resistors (4.7kΩ recommended) on both SCL and SDA lines

2. **Use an I2C master** to communicate with the slave:

   .. code-block:: bash

      # Using i2c-tools on Linux
      i2cdetect -y 1  # Should show device at address 0x50
      i2cget -y 1 0x50  # Read from slave
      i2cset -y 1 0x50 0x01  # Write to slave

3. **Expected output** on the RA4M2 console:

   .. code-block:: console

      I2C slave device ready at address 0x50
      Read requested from master
      Stop condition received

Advanced Configuration
---------------------

To change the slave address or other parameters, modify the device tree:

.. code-block:: dts

   &iic1 {
       pinctrl-0 = <&iic1_default>;
       pinctrl-names = "default";
       compatible = "renesas,ra-iic-slave";
       slave-address = <0x27>;  // Change to your desired address
       interrupts = <91 1>, <92 1>, <93 1>, <94 1>;
       interrupt-names = "rxi", "txi", "tei", "eri";
       status = "okay";
   };

Troubleshooting
--------------

- **Device not detected**: Check pull-up resistors and wiring
- **Address conflicts**: Ensure no other device uses 0x50
- **Communication errors**: Verify voltage levels (3.3V) and signal integrity
- **Interrupt issues**: Check that interrupts 91-94 are not used by other peripherals

Performance Considerations
-------------------------

- The IIC1 peripheral supports standard (100 kbps) and fast (400 kbps) I2C modes
- Interrupt-driven operation provides efficient CPU usage
- DMA is not supported for slave mode on this peripheral
- Maximum data transfer size is limited by FSP driver configuration

Example Applications
--------------------

1. **I2C Sensor Emulator**: Simulate various sensors for testing
2. **Configuration EEPROM**: Store and retrieve configuration data
3. **Inter-MCU Communication**: Communicate between multiple MCUs
4. **Debug Interface**: Provide debug information over I2C
5. **Bootloader Interface**: Update firmware via I2C

Combined Master and Slave Example
---------------------------------

The EK-RA4M2 can simultaneously use IIC0 as master and IIC1 as slave, enabling
complex I2C network scenarios:

**Example: I2C Bridge/Proxy Application**

.. code-block:: c

   #include <zephyr/drivers/i2c.h>

   // I2C Master (IIC0) - communicates with external sensors
   const struct device *i2c_master = DEVICE_DT_GET(DT_NODELABEL(iic0));

   // I2C Slave (IIC1) - receives commands from host
   const struct device *i2c_slave = DEVICE_DT_GET(DT_NODELABEL(iic1));

   static void slave_callback(const struct device *dev, uint32_t event)
   {
       if (event == I2C_SLAVE_EVENT_WRITE_RECEIVED) {
           uint8_t command[2];
           i2c_slave_read(dev, command, sizeof(command));

           // Forward command to sensor via master
           i2c_write(i2c_master, command, sizeof(command), command[0]);

           // Read response and send back via slave
           uint8_t response[4];
           i2c_read(i2c_master, response, sizeof(response), command[0]);
           i2c_slave_write(dev, response, sizeof(response));
       }
   }

**Wiring for Combined Operation**:

.. code-block:: text

   EK-RA4M2 (Master)  ↔  Sensor
   ------------------    -------
   P400 (SCL0)         ↔  SCL
   P401 (SDA0)         ↔  SDA

   EK-RA4M2 (Slave)   ↔  Host Controller
   -----------------    ----------------
   P402 (SCL1)         ↔  SCL
   P403 (SDA1)         ↔  SDA

**Note**: Both I2C buses require separate pull-up resistors!

I2C Performance and Configuration
==================================

**Performance Characteristics**:

+-------------------+-------------------+-------------------+
| Feature            | IIC0 (Master)     | IIC1 (Slave)      |
+===================+===================+===================+
| Max Speed         | 1 MHz             | 400 kHz           |
+-------------------+-------------------+-------------------+
| Address Modes     | 7-bit, 10-bit     | 7-bit, 10-bit     |
+-------------------+-------------------+-------------------+
| DMA Support       | Yes               | No                |
+-------------------+-------------------+-------------------+
| Interrupts        | RXI, TXI, TEI, ERI| RXI, TXI, TEI, ERI|
+-------------------+-------------------+-------------------+
| Buffer Size       | Configurable      | Configurable      |
+-------------------+-------------------+-------------------+

**Configuration Options**:

.. code-block:: kconfig

   # For advanced I2C features
   CONFIG_I2C_RENESAS_RA_IIC=y
   CONFIG_I2C_RENESAS_RA_IIC_SLAVE=y

   # For DMA support on master
   CONFIG_I2C_RENESAS_RA_IIC_DMA=y

   # For higher debug output
   CONFIG_I2C_LOG_LEVEL_DBG=y

**Device Tree Configuration**:

.. code-block:: dts

   &iic0 {
       /* Master configuration */
       clock-frequency = <DT_FREQ_M(1)>;  // 1 MHz
       status = "okay";
   };

   &iic1 {
       /* Slave configuration */
       slave-address = <0x50>;
       status = "okay";
   };

Troubleshooting and Best Practices
==================================

**Common Issues and Solutions**:

1. **Communication Errors**:
   - Check pull-up resistors (4.7kΩ recommended)
   - Verify voltage levels (3.3V only)
   - Ensure proper grounding
   - Check for address conflicts

2. **Device Not Detected**:
   - Verify wiring connections
   - Check device tree configuration
   - Ensure device is powered
   - Test with i2cdetect

3. **Performance Issues**:
   - Reduce bus capacitance
   - Use shorter wires
   - Check for noise sources
   - Verify clock stretching support

**Best Practices**:

- **Bus Topology**: Use star topology for multiple devices
- **Pull-up Resistors**: 4.7kΩ for 100kHz, 2.2kΩ for 400kHz
- **Signal Integrity**: Keep I2C lines away from power traces
- **Termination**: Consider series resistors for long traces
- **Error Handling**: Implement robust error recovery

**Debugging Tips**:

.. code-block:: c

   // Enable detailed I2C logging
   LOG_INF("I2C Master: %s", device_is_ready(i2c_master) ? "Ready" : "Not Ready");
   LOG_INF("I2C Slave: %s", device_is_ready(i2c_slave) ? "Ready" : "Not Ready");

   // Check I2C bus status
   int ret = i2c_configure(i2c_master, I2C_MODE_MASTER | I2C_SPEED_STANDARD);
   if (ret) {
       LOG_ERR("I2C configuration failed: %d", ret);
   }

Example Applications
====================

**1. Sensor Hub with Remote Control**:

- IIC0 (Master): Reads multiple sensors
- IIC1 (Slave): Receives commands and sends aggregated data
- Use Case: IoT gateway, environmental monitoring

**2. Configuration and Debug Interface**:

- IIC0 (Master): Controls peripheral devices
- IIC1 (Slave): Provides debug interface to host PC
- Use Case: Development, field debugging

**3. I2C Bridge/Proxy**:

- IIC0 (Master): Communicates with low-voltage devices
- IIC1 (Slave): Interface to high-voltage system
- Use Case: Voltage level translation, protocol conversion

**4. Dual-Bus Sensor Network**:

- IIC0 (Master): Fast sensors (IMU, pressure)
- IIC1 (Slave): Host communication interface
- Use Case: Robotics, industrial control

For more information, refer to:

- `RA4M2 Group User's Manual: I2C Section`_
- `FSP I2C API Documentation`_
- `Zephyr I2C Driver Guide`_

.. _RA4M2 Group User's Manual: I2C Section:
   https://www.renesas.com/us/en/document/man/ra4m2-group-users-manual-hardware
.. _FSP I2C API Documentation:
   https://github.com/renesas/fsp
.. _Zephyr I2C Driver Guide:
   https://docs.zephyrproject.org/latest/services/device_model/i2c.html

Programming and Debugging
*************************

.. zephyr:board-supported-runners::

Applications for the ``ek_ra4m2`` board target configuration can be
built, flashed, and debugged in the usual way. See
:ref:`build_an_application` and :ref:`application_run` for more details on
building and running.

Flashing
========

Program can be flashed to EK-RA4M2 via the on-board SEGGER J-Link debugger.
SEGGER J-link's drivers are available at https://www.segger.com/downloads/jlink/

To flash the program to board

1. Connect to J-Link OB via USB port to host PC

2. Make sure J-Link OB jumper is in default configuration as describe in `EK-RA4M2 - User's Manual`_

3. Execute west command

	.. code-block:: console

		west flash -r jlink

Debugging
=========

You can use Segger Ozone (`Segger Ozone Download`_) for a visual debug interface

Once downloaded and installed, open Segger Ozone and configure the debug project
like so:

* Target Device: R7FA4M2AD
* Target Interface: SWD
* Target Interface Speed: 4 MHz
* Host Interface: USB
* Program File: <path/to/your/build/zephyr.elf>

**Note:** It's verified that we can debug OK on Segger Ozone v3.30d so please use this or later
version of Segger Ozone

References
**********
- `EK-RA4M2 Website`_
- `RA4M2 MCU group Website`_

.. _EK-RA4M2 Website:
   https://www.renesas.com/us/en/products/microcontrollers-microprocessors/ra-cortex-m-mcus/ek-ra4m2-evaluation-kit-ra4m2-mcu-group

.. _RA4M2 MCU group Website:
   https://www.renesas.com/us/en/products/microcontrollers-microprocessors/ra-cortex-m-mcus/ra4m2-100mhz-arm-cortex-m33-trustzone-high-integration-lowest-active-power-consumption

.. _EK-RA4M2 - User's Manual:
   https://www.renesas.com/us/en/document/mat/ek-ra4m2-v1-users-manual

.. _RA4M2 Group User's Manual Hardware:
   https://www.renesas.com/us/en/document/man/ra4m2-group-users-manual-hardware

.. _Segger Ozone Download:
   https://www.segger.com/downloads/jlink#Ozone
