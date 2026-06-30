################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.cpp \
../TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.cpp 

OBJS += \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.o \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.o \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.o \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.o \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.o 

CPP_DEPS += \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.d \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.d \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.d \
./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.d \
./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/images/src/mainMenuElements/%.o TouchGFX/generated/images/src/mainMenuElements/%.su TouchGFX/generated/images/src/mainMenuElements/%.cyclo: ../TouchGFX/generated/images/src/mainMenuElements/%.cpp TouchGFX/generated/images/src/mainMenuElements/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-mainMenuElements

clean-TouchGFX-2f-generated-2f-images-2f-src-2f-mainMenuElements:
	-$(RM) ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.d ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.o ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_left_pressed.su ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.d ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.o ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_gold_right_pressed.su ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.d ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.o ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_left_normal.su ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.d ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.o ./TouchGFX/generated/images/src/mainMenuElements/image_arrow_btn_right_normal.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_normal.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_close_pressed.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_normal.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_details_pressed.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_normal.su ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.cyclo ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.d ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.o ./TouchGFX/generated/images/src/mainMenuElements/image_icon_button_home_pressed.su

.PHONY: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-mainMenuElements

