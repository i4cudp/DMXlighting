################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../09_MQXLite/psp/cortex_m/core/M4/dispatch.S 

OBJS += \
./09_MQXLite/psp/cortex_m/core/M4/dispatch.o 

S_UPPER_DEPS += \
./09_MQXLite/psp/cortex_m/core/M4/dispatch.d 


# Each subdirectory must supply rules for building sources it contributes
09_MQXLite/psp/cortex_m/core/M4/%.o: ../09_MQXLite/psp/cortex_m/core/M4/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -x assembler-with-cpp -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/app" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/bsp" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/config" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/include" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/kernel" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/psp/cortex_m" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/psp/cortex_m/core/M4" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/09_MQXLite/psp/cortex_m/compiler/cwgcc" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/08_Source" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/07_Soft_Component" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/07_Soft_Component/printf" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/06_App_Component" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/06_App_Component/light" -I"E:/LED_DMX512/K64_SD_DMX_SLAVE/03_MCU" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


