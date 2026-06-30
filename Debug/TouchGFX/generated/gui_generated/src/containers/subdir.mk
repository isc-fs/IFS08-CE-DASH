################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.cpp \
../TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.cpp \
../TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.cpp \
../TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.cpp \
../TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.cpp 

OBJS += \
./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.o \
./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.o \
./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.o \
./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.o \
./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.o 

CPP_DEPS += \
./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.d \
./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.d \
./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.d \
./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.d \
./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.d 


# Each subdirectory must supply rules for building sources it contributes
TouchGFX/generated/gui_generated/src/containers/%.o TouchGFX/generated/gui_generated/src/containers/%.su TouchGFX/generated/gui_generated/src/containers/%.cyclo: ../TouchGFX/generated/gui_generated/src/containers/%.cpp TouchGFX/generated/gui_generated/src/containers/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Utilities/JPEG -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../TouchGFX/App -I../TouchGFX/target/generated -I../TouchGFX/target -I../Middlewares/ST/touchgfx/framework/include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Middlewares/Third_Party/FatFs/src -I../TouchGFX/generated/fonts/include -I../TouchGFX/generated/gui_generated/include -I../TouchGFX/generated/images/include -I../TouchGFX/generated/texts/include -I../TouchGFX/generated/videos/include -I../TouchGFX/gui/include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -femit-class-debug-always -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers

clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers:
	-$(RM) ./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.cyclo ./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.d ./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.o ./TouchGFX/generated/gui_generated/src/containers/AnimatingWavesBase.su ./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.cyclo ./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.d ./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.o ./TouchGFX/generated/gui_generated/src/containers/BeverageCupsBase.su ./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.cyclo ./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.d ./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.o ./TouchGFX/generated/gui_generated/src/containers/RecipesDetailsSwitchBase.su ./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.cyclo ./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.d ./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.o ./TouchGFX/generated/gui_generated/src/containers/VariantBackgroundBase.su ./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.cyclo ./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.d ./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.o ./TouchGFX/generated/gui_generated/src/containers/mainMenuBannerBase.su

.PHONY: clean-TouchGFX-2f-generated-2f-gui_generated-2f-src-2f-containers

