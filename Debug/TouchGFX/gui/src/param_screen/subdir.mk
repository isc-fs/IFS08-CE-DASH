################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/gui/src/param_screen/ParamPresenter.cpp \
../TouchGFX/gui/src/param_screen/ParamView.cpp 

OBJS += \
./TouchGFX/gui/src/param_screen/ParamPresenter.o \
./TouchGFX/gui/src/param_screen/ParamView.o 

CPP_DEPS += \
./TouchGFX/gui/src/param_screen/ParamPresenter.d \
./TouchGFX/gui/src/param_screen/ParamView.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/gui/src/param_screen/%.o TouchGFX/gui/src/param_screen/%.su TouchGFX/gui/src/param_screen/%.cyclo: ../TouchGFX/gui/src/param_screen/%.cpp TouchGFX/gui/src/param_screen/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-gui-2f-src-2f-param_screen

clean-TouchGFX-2f-gui-2f-src-2f-param_screen:
	-$(RM) ./TouchGFX/gui/src/param_screen/ParamPresenter.cyclo ./TouchGFX/gui/src/param_screen/ParamPresenter.d ./TouchGFX/gui/src/param_screen/ParamPresenter.o ./TouchGFX/gui/src/param_screen/ParamPresenter.su ./TouchGFX/gui/src/param_screen/ParamView.cyclo ./TouchGFX/gui/src/param_screen/ParamView.d ./TouchGFX/gui/src/param_screen/ParamView.o ./TouchGFX/gui/src/param_screen/ParamView.su

.PHONY: clean-TouchGFX-2f-gui-2f-src-2f-param_screen

