################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../09_MQXLite/app/02_task_templates.c \
../09_MQXLite/app/03_task_main.c \
../09_MQXLite/app/task_DMX512.c \
../09_MQXLite/app/task_detect.c \
../09_MQXLite/app/task_effects.c \
../09_MQXLite/app/task_enet.c \
../09_MQXLite/app/task_horserace.c \
../09_MQXLite/app/task_light.c \
../09_MQXLite/app/task_sdhc.c \
../09_MQXLite/app/task_uartsend.c \
../09_MQXLite/app/task_waterlamp.c 

OBJS += \
./09_MQXLite/app/02_task_templates.o \
./09_MQXLite/app/03_task_main.o \
./09_MQXLite/app/task_DMX512.o \
./09_MQXLite/app/task_detect.o \
./09_MQXLite/app/task_effects.o \
./09_MQXLite/app/task_enet.o \
./09_MQXLite/app/task_horserace.o \
./09_MQXLite/app/task_light.o \
./09_MQXLite/app/task_sdhc.o \
./09_MQXLite/app/task_uartsend.o \
./09_MQXLite/app/task_waterlamp.o 

C_DEPS += \
./09_MQXLite/app/02_task_templates.d \
./09_MQXLite/app/03_task_main.d \
./09_MQXLite/app/task_DMX512.d \
./09_MQXLite/app/task_detect.d \
./09_MQXLite/app/task_effects.d \
./09_MQXLite/app/task_enet.d \
./09_MQXLite/app/task_horserace.d \
./09_MQXLite/app/task_light.d \
./09_MQXLite/app/task_sdhc.d \
./09_MQXLite/app/task_uartsend.d \
./09_MQXLite/app/task_waterlamp.d 


# Each subdirectory must supply rules for building sources it contributes
09_MQXLite/app/%.o: ../09_MQXLite/app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCPU_MK64FN1M0VMD12 -DUSB_STACK_BM -I"E:/LED_DMX512/K64_SD_DMX_MASTER/02_CPU" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/03_MCU" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/sdhc" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/flash" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/gprs" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/uart" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/gpio" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/05_Driver/spi" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component/light" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component/Camera" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component/emfs" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component/DMX512" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/06_App_Component/diskio" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/common" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/printf" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/buflist" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/comlib" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/comlib_server" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/comlib_terminal" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/07_Soft_Component/w5500" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/08_Source" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/app" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/bsp" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/config" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/include" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/kernel" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/core/M4" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/compiler/cwgcc" -I"E:/LED_DMX512/K64_SD_DMX_MASTER/09_MQXLite/" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


