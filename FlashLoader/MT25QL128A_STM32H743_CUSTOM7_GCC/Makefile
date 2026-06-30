TOOLROOT ?= C:/ST/STM32CubeIDE_1.17.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.1.0.202410251130/tools/bin
CC := $(TOOLROOT)/arm-none-eabi-gcc
OBJCOPY := $(TOOLROOT)/arm-none-eabi-objcopy
SIZE := $(TOOLROOT)/arm-none-eabi-size
READELF := $(TOOLROOT)/arm-none-eabi-readelf

TARGET := MT25QL128A_STM32H743_CUSTOM7
BUILD := build

PROJECT_ROOT := ../..

HAL_INC := $(PROJECT_ROOT)/Core/Inc
HAL_DRV_INC := $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Inc
CMSIS_DEV_INC := $(PROJECT_ROOT)/Drivers/CMSIS/Device/ST/STM32H7xx/Include
CMSIS_CORE_INC := $(PROJECT_ROOT)/Drivers/CMSIS/Include

SRCS := \
	loader.c \
	Dev_Inf.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
	$(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
	$(PROJECT_ROOT)/Core/Src/system_stm32h7xx.c

OBJS := $(addprefix $(BUILD)/,$(notdir $(SRCS:.c=.o)))

CFLAGS := \
	-mcpu=cortex-m7 \
	-mthumb \
	-mfloat-abi=soft \
	-Os \
	-ffunction-sections \
	-fdata-sections \
	-fno-common \
	-Wall \
	-Wextra \
	-DUSE_HAL_DRIVER \
	-DSTM32H743xx \
	-I. \
	-I$(HAL_INC) \
	-I$(HAL_DRV_INC) \
	-I$(CMSIS_DEV_INC) \
	-I$(CMSIS_CORE_INC)

LDFLAGS := \
	-mcpu=cortex-m7 \
	-mthumb \
	-mfloat-abi=soft \
	-nostartfiles \
	-T loader.ld \
	-Wl,--gc-sections \
	-Wl,-Map,$(BUILD)/$(TARGET).map

.PHONY: all clean inspect

all: $(BUILD)/$(TARGET).stldr

$(BUILD):
	mkdir $(BUILD)

$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_cortex.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_gpio.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_qspi.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_rcc.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_rcc_ex.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_pwr.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/stm32h7xx_hal_pwr_ex.o: $(PROJECT_ROOT)/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/system_stm32h7xx.o: $(PROJECT_ROOT)/Core/Src/system_stm32h7xx.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/$(TARGET).stldr: $(BUILD)/$(TARGET).elf
	copy /Y $(BUILD)\\$(TARGET).elf $(BUILD)\\$(TARGET).stldr

inspect: $(BUILD)/$(TARGET).elf
	$(READELF) -S $(BUILD)/$(TARGET).elf
	$(READELF) -s $(BUILD)/$(TARGET).elf

clean:
	if exist $(BUILD) rmdir /S /Q $(BUILD)
