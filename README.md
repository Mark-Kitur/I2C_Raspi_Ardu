# Sensor Monitoring System — Arduino, Raspberry Pi, Linux Kernel and PyQt

## Overview

This project implements an end-to-end sensor monitoring system that acquires sensor measurements using an Arduino and transfers the collected data to a Raspberry Pi 5 over the I²C communication bus.

On the Raspberry Pi, the communication interface is integrated into the Linux kernel using a custom Device Tree overlay and an I²C kernel module. The kernel module exposes a character-device interface to userspace, allowing a Python application to retrieve sensor data and display it through a PyQt speedometer dashboard.

The Linux kernel module and Device Tree overlay are integrated into a custom embedded Linux image built using the Yocto Project and BitBake.

The project demonstrates embedded sensor acquisition, I²C communication, Linux kernel driver development, Device Tree integration, kernel-to-userspace communication, Yocto-based embedded Linux development, and graphical data visualization.

## System Architecture

```text
              PHYSICAL ENVIRONMENT
                       |
                       v
                    SENSORS
                       |
                       v
              ARDUINO SLAVE DEVICE
          Sensor acquisition and processing
                       |
                       | I²C (SDA + SCL)
                       |
                       v
              RASPBERRY PI 5
                       |
                       v
                 LINUX KERNEL
                       |
             +---------+----------+
             |                    |
             v                    v
       Device Tree          I²C Subsystem
       Overlay                    |
                                  v
                         Custom I²C Driver
                                  |
                                  v
                         Character Device
                        /dev/raspi_master
                                  |
                                  v
                            USERSPACE
                         Python Application
                                  |
                                  v
                         PyQt Speedometer
                         Sensor Visualization
```

### Yocto Build and Deployment

```text
       YOCTO PROJECT / BITBAKE
                  |
          +-------+--------+
          |                |
          v                v
    Kernel Module    Device Tree Overlay
          |                |
          +-------+--------+
                  |
                  v
        Custom Linux Image
                  |
                  v
           RASPBERRY PI 5
                  |
                  v
       Integrated I²C Interface
```

## Repository Structure

```text
.
├── Arduino_slave
│   ├── platformio.ini
│   └── src
│       └── main.cpp
│
├── Raspi_master
│   ├── dts
│   │   └── i2c_dt_overlay.dts
│   ├── Makefile
│   ├── master_i2c.c
│   └── master_i2c.dtbo
│
└── README.md
```

## System Components

### Arduino Slave

The Arduino acts as an I²C peripheral and collects measurements from the connected sensors. It processes the measurements and makes the resulting data available to the Raspberry Pi when requested.

The Arduino firmware is located in `Arduino_slave/` and is developed using PlatformIO.

### Raspberry Pi I²C Master

The Raspberry Pi initiates I²C transactions to request sensor data from the Arduino. The communication logic is implemented in a custom Linux kernel driver.

The `Raspi_master/` directory contains the driver source code, Device Tree overlay source, compiled overlay, and Makefile.

### Linux Kernel Integration

The Linux kernel provides the I²C subsystem through which the driver communicates with the Arduino. A Device Tree overlay describes the Arduino as an I²C device so that the kernel can identify it and associate it with the appropriate driver.

The custom kernel module handles the I²C communication and exposes a character-device interface to userspace.

### Yocto Project Integration

The Raspberry Pi software stack is built using the Yocto Project and BitBake to produce a custom Linux image for the Raspberry Pi 5.

The build integrates the custom kernel module and Device Tree overlay through the project's BitBake recipes and image configuration.

The integration workflow includes:

1. Building the custom I²C kernel module.
2. Compiling the Device Tree overlay from its source.
3. Packaging the required components into the appropriate image or deployment outputs.
4. Deploying the generated Linux image to the Raspberry Pi 5.
5. Configuring the target system to load the kernel module and enable the Device Tree overlay.
6. Accessing the I²C communication interface from userspace.

This approach makes the driver and hardware-description components part of a reproducible embedded Linux build rather than relying solely on manual installation.

### PyQt Speedometer Dashboard

The [PyQt Speedometer](https://github.com/Mark-Kitur/speedometer) is a separate userspace application that visualizes the data received from the Arduino through the Raspberry Pi.

The application accesses the kernel driver's character-device interface, retrieves the available sensor data, and presents it through a graphical speedometer interface.

Keeping the dashboard in userspace separates data visualization from low-level hardware communication. The Linux kernel driver manages device communication, while Python and PyQt handle application logic and presentation.

## Communication Pipeline

### 1. Sensor Acquisition

The sensors provide measurements to the Arduino. The Arduino firmware reads the sensor outputs, processes the measurements, and prepares the data for transmission.

### 2. I²C Communication

The Raspberry Pi initiates communication with the Arduino over the I²C bus. The Arduino operates as the slave/peripheral, responding to requests from the Raspberry Pi.

The communication uses the SDA and SCL signal lines, with both devices sharing a common ground and compatible electrical logic levels.

### 3. Device Tree Integration

The Device Tree overlay describes the Arduino peripheral on the Raspberry Pi's I²C bus, including its I²C address and compatible identifier.

The overlay source is located at:

`Raspi_master/dts/i2c_dt_overlay.dts`

The compiled overlay is:

`Raspi_master/master_i2c.dtbo`

The overlay must be installed and enabled in the target system's boot configuration for the device to be described to the kernel.

### 4. Linux Kernel Driver

The custom I²C driver, implemented in `Raspi_master/master_i2c.c`, handles communication between the Linux kernel and the Arduino peripheral.

The driver integrates with the Linux I²C subsystem and exposes a character-device interface for userspace access.

### 5. Yocto Build and Deployment

The Yocto Project and BitBake are used to build the custom embedded Linux image and integrate the required driver and Device Tree overlay.

The resulting deployment provides the software components required for the Raspberry Pi to communicate with the Arduino and make the received data available to userspace.

### 6. Userspace Data Access

The character device provides the interface through which a userspace application reads sensor data from the kernel driver.

The device node used by this project is:

`/dev/raspi_master`

Applications can interact with the device using standard file operations such as `open()`, `read()`, `write()`, and `close()`, subject to the operations implemented by the driver.

### 7. PyQt Visualization

The Python application retrieves sensor data through the character-device interface and updates the PyQt speedometer dashboard, allowing measurements to be monitored through a graphical user interface.

The dashboard source code is maintained separately in the [speedometer repository](https://github.com/Mark-Kitur/speedometer).

## Technologies Used

- **Embedded firmware:** C/C++, Arduino framework, PlatformIO
- **Communication protocol:** I²C
- **Embedded Linux hardware:** Raspberry Pi 5
- **Operating system build:** Yocto Project, BitBake
- **Kernel development:** Linux I²C subsystem, custom kernel module, character devices
- **Hardware description:** Device Tree overlays
- **Userspace:** Python
- **Graphical interface:** PyQt
- **Version control:** Git and GitHub

## Project Objectives

- Acquire sensor measurements using an Arduino.
- Transfer sensor data between the Arduino and Raspberry Pi over I²C.
- Integrate the peripheral into Linux using a Device Tree overlay.
- Develop a custom Linux kernel driver for I²C communication.
- Expose kernel functionality through a character-device interface.
- Integrate the kernel module and Device Tree overlay into a Yocto-built Linux image.
- Retrieve sensor measurements from userspace.
- Visualize sensor data through a PyQt speedometer dashboard.

