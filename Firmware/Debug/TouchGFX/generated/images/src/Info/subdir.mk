################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.cpp \
../TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.cpp \
../TouchGFX/generated/images/src/Info/image_content_indicator_active.cpp \
../TouchGFX/generated/images/src/Info/image_content_indicator_bar.cpp \
../TouchGFX/generated/images/src/Info/image_content_indicator_normal.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.cpp \
../TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.cpp \
../TouchGFX/generated/images/src/Info/image_qr_code_frame.cpp \
../TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.cpp \
../TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.cpp \
../TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.cpp \
../TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.cpp \
../TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.cpp \
../TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.cpp \
../TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.cpp 

OBJS += \
./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.o \
./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.o \
./TouchGFX/generated/images/src/Info/image_content_indicator_active.o \
./TouchGFX/generated/images/src/Info/image_content_indicator_bar.o \
./TouchGFX/generated/images/src/Info/image_content_indicator_normal.o \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.o \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.o \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.o \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.o \
./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.o \
./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.o \
./TouchGFX/generated/images/src/Info/image_qr_code_frame.o \
./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.o \
./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.o \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.o \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.o \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.o \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.o \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.o 

CPP_DEPS += \
./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.d \
./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.d \
./TouchGFX/generated/images/src/Info/image_content_indicator_active.d \
./TouchGFX/generated/images/src/Info/image_content_indicator_bar.d \
./TouchGFX/generated/images/src/Info/image_content_indicator_normal.d \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.d \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.d \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.d \
./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.d \
./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.d \
./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.d \
./TouchGFX/generated/images/src/Info/image_qr_code_frame.d \
./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.d \
./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.d \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.d \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.d \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.d \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.d \
./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/images/src/Info/%.o TouchGFX/generated/images/src/Info/%.su TouchGFX/generated/images/src/Info/%.cyclo: ../TouchGFX/generated/images/src/Info/%.cpp TouchGFX/generated/images/src/Info/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Info

clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Info:
	-$(RM) ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.cyclo ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.d ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.o ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_left_pressed.su ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.cyclo ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.d ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.o ./TouchGFX/generated/images/src/Info/image_arrow_btn_brown_right_pressed.su ./TouchGFX/generated/images/src/Info/image_content_indicator_active.cyclo ./TouchGFX/generated/images/src/Info/image_content_indicator_active.d ./TouchGFX/generated/images/src/Info/image_content_indicator_active.o ./TouchGFX/generated/images/src/Info/image_content_indicator_active.su ./TouchGFX/generated/images/src/Info/image_content_indicator_bar.cyclo ./TouchGFX/generated/images/src/Info/image_content_indicator_bar.d ./TouchGFX/generated/images/src/Info/image_content_indicator_bar.o ./TouchGFX/generated/images/src/Info/image_content_indicator_bar.su ./TouchGFX/generated/images/src/Info/image_content_indicator_normal.cyclo ./TouchGFX/generated/images/src/Info/image_content_indicator_normal.d ./TouchGFX/generated/images/src/Info/image_content_indicator_normal.o ./TouchGFX/generated/images/src/Info/image_content_indicator_normal.su ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.d ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.o ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_bottom.su ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.d ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.o ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_left_top.su ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.d ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.o ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_bottom.su ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.d ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.o ./TouchGFX/generated/images/src/Info/image_lightbox_cornor_right_top.su ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.d ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.o ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar.su ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.cyclo ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.d ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.o ./TouchGFX/generated/images/src/Info/image_lightbox_tab_bar_selected.su ./TouchGFX/generated/images/src/Info/image_qr_code_frame.cyclo ./TouchGFX/generated/images/src/Info/image_qr_code_frame.d ./TouchGFX/generated/images/src/Info/image_qr_code_frame.o ./TouchGFX/generated/images/src/Info/image_qr_code_frame.su ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.cyclo ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.d ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.o ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_left.su ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.cyclo ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.d ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.o ./TouchGFX/generated/images/src/Info/image_qr_tag_cup_right.su ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.cyclo ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.d ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.o ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_1.su ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.cyclo ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.d ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.o ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_2.su ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.cyclo ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.d ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.o ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_3.su ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.cyclo ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.d ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.o ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_4.su ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.cyclo ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.d ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.o ./TouchGFX/generated/images/src/Info/image_recipes_ingredient_5.su

.PHONY: clean-TouchGFX-2f-generated-2f-images-2f-src-2f-Info

