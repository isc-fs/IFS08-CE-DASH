# Custom Embedded Board -- Hardware Description

## 1. Overview

This document describes the hardware architecture of a custom embedded
system based on an STM32H7 microcontroller. The platform is designed for
a graphical embedded application with external memory, LCD interface,
and CAN connectivity. The system supports an RTOS-based firmware
architecture and a graphical user interface framework.

Main functional blocks:

-   High-performance microcontroller (STM32H743 series)
-   External SDRAM for frame buffer and dynamic data
-   External QSPI NOR Flash for assets and storage
-   TFT LCD display (800x480 resolution)
-   CAN FD communication interface
-   Power regulation and system power management

------------------------------------------------------------------------

## 2. Microcontroller

**Primary MCU:** STM32H743XI/G (ARM Cortex-M7)

Key features:

-   32-bit ARM Cortex-M7 core
-   Up to 480 MHz CPU frequency
-   Advanced DMA architecture
-   Multiple communication interfaces
-   FMC external memory controller
-   LTDC LCD-TFT display controller
-   FDCAN peripheral

The MCU is the central processing element of the board and controls all
peripherals, external memories, and communication interfaces.

------------------------------------------------------------------------

## 3. External SDRAM

**Component:** IS42S16400J -- 4M × 16 SDRAM

**Connected via:**

-   FMC (Flexible Memory Controller)

**Purpose:**

-   Framebuffer storage for the LCD controller
-   Large runtime buffers
-   Graphics memory for TouchGFX
-   Heap or application data

Typical configuration:

-   16-bit data bus
-   CAS latency configured according to SDRAM clock
-   Periodic refresh controlled by FMC

------------------------------------------------------------------------

## 4. External NOR Flash

**Component:** MT25QL128 Serial NOR Flash

**Interface:**

-   Quad-SPI (QSPI)

**Purpose:**

-   Storage of firmware assets
-   GUI graphics and fonts
-   External non-volatile storage
-   Potential memory-mapped read access

Features:

-   128 Mbit density
-   Quad I/O mode
-   Memory-mapped read capability

------------------------------------------------------------------------

## 5. Display Subsystem

**Display:** Newhaven NHD-5.0-800480TF-ATXL-T

Specifications:

-   5.0 inch TFT LCD
-   Resolution: 800 × 480
-   RGB parallel interface
-   Integrated display timing controller

Connected to MCU via:

-   LTDC peripheral

Typical signals:

-   RGB data lines
-   HSYNC
-   VSYNC
-   DE (Data Enable)
-   Pixel Clock

Framebuffer is stored in external SDRAM and read by LTDC.

------------------------------------------------------------------------

## 6. Communication Interfaces

### CAN FD Interface

**Transceiver:** TCAN33x series

Connection:

-   STM32 FDCAN peripheral
-   Differential CAN bus lines (CANH / CANL)

Purpose:

-   Real-time communication with external systems
-   Automotive or industrial network compatibility

------------------------------------------------------------------------

## 7. Power System

**Voltage Regulator:** LD39200

Characteristics:

-   Low Dropout Linear Regulator
-   Up to 2A output current

Purpose:

-   Provides stable voltage supply for MCU and digital components
-   Low noise characteristics suitable for sensitive digital circuits

Typical rails:

-   3.3V digital domain
-   Additional rails depending on board design

------------------------------------------------------------------------

## 8. Software Stack

Operating System:

-   FreeRTOS with CMSIS-RTOS v2 API

Graphics Framework:

-   TouchGFX

Kernel capabilities:

-   Multithreaded application execution
-   GUI rendering
-   Peripheral drivers
-   Communication protocols
cambia 