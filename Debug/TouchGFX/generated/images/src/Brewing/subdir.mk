################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/images/src/Brewing/image_Labels-1.cpp \
../TouchGFX/generated/images/src/Brewing/image_Labels-2.cpp \
../TouchGFX/generated/images/src/Brewing/image_Labels-3.cpp \
../TouchGFX/generated/images/src/Brewing/image_Labels.cpp \
../TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_brewing_done.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_add_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_done_normal.cpp \
../TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.cpp \
../TouchGFX/generated/images/src/Brewing/image_label_left.cpp \
../TouchGFX/generated/images/src/Brewing/image_label_right.cpp 

OBJS += \
./TouchGFX/generated/images/src/Brewing/image_Labels-1.o \
./TouchGFX/generated/images/src/Brewing/image_Labels-2.o \
./TouchGFX/generated/images/src/Brewing/image_Labels-3.o \
./TouchGFX/generated/images/src/Brewing/image_Labels.o \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.o \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.o \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_brewing_done.o \
./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.o \
./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.o \
./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.o \
./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.o \
./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.o \
./TouchGFX/generated/images/src/Brewing/image_label_left.o \
./TouchGFX/generated/images/src/Brewing/image_label_right.o 

CPP_DEPS += \
./TouchGFX/generated/images/src/Brewing/image_Labels-1.d \
./TouchGFX/generated/images/src/Brewing/image_Labels-2.d \
./TouchGFX/generated/images/src/Brewing/image_Labels-3.d \
./TouchGFX/generated/images/src/Brewing/image_Labels.d \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.d \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.d \
./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_brewing_done.d \
./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.d \
./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.d \
./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.d \
./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.d \
./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.d \
./TouchGFX/generated/images/src/Brewing/image_label_left.d \
./TouchGFX/generated/images/src/Brewing/image_label_right.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/images/src/Brewing/%.o TouchGFX/generated/images/src/Brewing/%.su TouchGFX/generated/images/src/Brewing/%.cyclo: ../TouchGFX/generated/images/src/Brewing/%.cpp TouchGFX/generated/images/src/Brewing/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Brewing

clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Brewing:
	-$(RM) ./TouchGFX/generated/images/src/Brewing/image_Labels-1.cyclo ./TouchGFX/generated/images/src/Brewing/image_Labels-1.d ./TouchGFX/generated/images/src/Brewing/image_Labels-1.o ./TouchGFX/generated/images/src/Brewing/image_Labels-1.su ./TouchGFX/generated/images/src/Brewing/image_Labels-2.cyclo ./TouchGFX/generated/images/src/Brewing/image_Labels-2.d ./TouchGFX/generated/images/src/Brewing/image_Labels-2.o ./TouchGFX/generated/images/src/Brewing/image_Labels-2.su ./TouchGFX/generated/images/src/Brewing/image_Labels-3.cyclo ./TouchGFX/generated/images/src/Brewing/image_Labels-3.d ./TouchGFX/generated/images/src/Brewing/image_Labels-3.o ./TouchGFX/generated/images/src/Brewing/image_Labels-3.su ./TouchGFX/generated/images/src/Brewing/image_Labels.cyclo ./TouchGFX/generated/images/src/Brewing/image_Labels.d ./TouchGFX/generated/images/src/Brewing/image_Labels.o ./TouchGFX/generated/images/src/Brewing/image_Labels.su ./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.d ./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.o ./TouchGFX/generated/images/src/Brewing/image_brew_btn_normal.su ./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.d ./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.o ./TouchGFX/generated/images/src/Brewing/image_brew_btn_pressed.su ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.d ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.o ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_normal.su ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.d ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.o ./TouchGFX/generated/images/src/Brewing/image_brew_btn_txt_pressed.su ./TouchGFX/generated/images/src/Brewing/image_brewing_done.cyclo ./TouchGFX/generated/images/src/Brewing/image_brewing_done.d ./TouchGFX/generated/images/src/Brewing/image_brewing_done.o ./TouchGFX/generated/images/src/Brewing/image_brewing_done.su ./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.d ./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.o ./TouchGFX/generated/images/src/Brewing/image_icon_add_normal.su ./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.d ./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.o ./TouchGFX/generated/images/src/Brewing/image_icon_add_pressed.su ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.d ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.o ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_normal.su ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.d ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.o ./TouchGFX/generated/images/src/Brewing/image_icon_button_add_pressed.su ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.d ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.o ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_normal.su ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.d ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.o ./TouchGFX/generated/images/src/Brewing/image_icon_button_done_pressed.su ./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.d ./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.o ./TouchGFX/generated/images/src/Brewing/image_icon_done_normal.su ./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.cyclo ./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.d ./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.o ./TouchGFX/generated/images/src/Brewing/image_icon_done_pressed.su ./TouchGFX/generated/images/src/Brewing/image_label_left.cyclo ./TouchGFX/generated/images/src/Brewing/image_label_left.d ./TouchGFX/generated/images/src/Brewing/image_label_left.o ./TouchGFX/generated/images/src/Brewing/image_label_left.su ./TouchGFX/generated/images/src/Brewing/image_label_right.cyclo ./TouchGFX/generated/images/src/Brewing/image_label_right.d ./TouchGFX/generated/images/src/Brewing/image_label_right.o ./TouchGFX/generated/images/src/Brewing/image_label_right.su

.PHONY: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Brewing

