################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../06_App_Component/diskio/diskio.c 

OBJS += \
./06_App_Component/diskio/diskio.o 

C_DEPS += \
./06_App_Component/diskio/diskio.d 


# Each subdirectory must supply rules for building sources it contributes
06_App_Component/diskio/%.o: ../06_App_Component/diskio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCPU_MK64FN1M0VMD12 -DUSB_STACK_BM -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/02_CPU" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/03_MCU" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/sdhc" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/flash" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/gprs" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/uart" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/gpio" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/spi" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/light" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/Camera" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/emfs" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/DMX512" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/diskio" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/common" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/printf" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/buflist" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib_server" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib_terminal" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/w5500" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/08_Source" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/app" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/bsp" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/config" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/include" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/kernel" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/core/M4" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/compiler/cwgcc" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


