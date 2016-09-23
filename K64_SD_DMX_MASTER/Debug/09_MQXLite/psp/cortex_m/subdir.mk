################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../09_MQXLite/psp/cortex_m/cortex.c \
../09_MQXLite/psp/cortex_m/int_gkis.c \
../09_MQXLite/psp/cortex_m/int_inst.c \
../09_MQXLite/psp/cortex_m/int_kisr.c \
../09_MQXLite/psp/cortex_m/int_pvta.c \
../09_MQXLite/psp/cortex_m/int_unx.c \
../09_MQXLite/psp/cortex_m/int_vtab.c \
../09_MQXLite/psp/cortex_m/int_xcpt.c \
../09_MQXLite/psp/cortex_m/mem_zero.c \
../09_MQXLite/psp/cortex_m/psp_iinit.c \
../09_MQXLite/psp/cortex_m/psp_supp.c \
../09_MQXLite/psp/cortex_m/psp_tiad.c \
../09_MQXLite/psp/cortex_m/psp_tinm.c \
../09_MQXLite/psp/cortex_m/psp_tipr.c \
../09_MQXLite/psp/cortex_m/psp_tisu.c \
../09_MQXLite/psp/cortex_m/sc_irdyq.c \
../09_MQXLite/psp/cortex_m/stack_bu.c \
../09_MQXLite/psp/cortex_m/stack_de.c \
../09_MQXLite/psp/cortex_m/stack_st.c 

OBJS += \
./09_MQXLite/psp/cortex_m/cortex.o \
./09_MQXLite/psp/cortex_m/int_gkis.o \
./09_MQXLite/psp/cortex_m/int_inst.o \
./09_MQXLite/psp/cortex_m/int_kisr.o \
./09_MQXLite/psp/cortex_m/int_pvta.o \
./09_MQXLite/psp/cortex_m/int_unx.o \
./09_MQXLite/psp/cortex_m/int_vtab.o \
./09_MQXLite/psp/cortex_m/int_xcpt.o \
./09_MQXLite/psp/cortex_m/mem_zero.o \
./09_MQXLite/psp/cortex_m/psp_iinit.o \
./09_MQXLite/psp/cortex_m/psp_supp.o \
./09_MQXLite/psp/cortex_m/psp_tiad.o \
./09_MQXLite/psp/cortex_m/psp_tinm.o \
./09_MQXLite/psp/cortex_m/psp_tipr.o \
./09_MQXLite/psp/cortex_m/psp_tisu.o \
./09_MQXLite/psp/cortex_m/sc_irdyq.o \
./09_MQXLite/psp/cortex_m/stack_bu.o \
./09_MQXLite/psp/cortex_m/stack_de.o \
./09_MQXLite/psp/cortex_m/stack_st.o 

C_DEPS += \
./09_MQXLite/psp/cortex_m/cortex.d \
./09_MQXLite/psp/cortex_m/int_gkis.d \
./09_MQXLite/psp/cortex_m/int_inst.d \
./09_MQXLite/psp/cortex_m/int_kisr.d \
./09_MQXLite/psp/cortex_m/int_pvta.d \
./09_MQXLite/psp/cortex_m/int_unx.d \
./09_MQXLite/psp/cortex_m/int_vtab.d \
./09_MQXLite/psp/cortex_m/int_xcpt.d \
./09_MQXLite/psp/cortex_m/mem_zero.d \
./09_MQXLite/psp/cortex_m/psp_iinit.d \
./09_MQXLite/psp/cortex_m/psp_supp.d \
./09_MQXLite/psp/cortex_m/psp_tiad.d \
./09_MQXLite/psp/cortex_m/psp_tinm.d \
./09_MQXLite/psp/cortex_m/psp_tipr.d \
./09_MQXLite/psp/cortex_m/psp_tisu.d \
./09_MQXLite/psp/cortex_m/sc_irdyq.d \
./09_MQXLite/psp/cortex_m/stack_bu.d \
./09_MQXLite/psp/cortex_m/stack_de.d \
./09_MQXLite/psp/cortex_m/stack_st.d 


# Each subdirectory must supply rules for building sources it contributes
09_MQXLite/psp/cortex_m/%.o: ../09_MQXLite/psp/cortex_m/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DCPU_MK64FN1M0VMD12 -DUSB_STACK_BM -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/02_CPU" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/03_MCU" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/sdhc" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/flash" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/gprs" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/uart" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/gpio" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/05_Driver/spi" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/light" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/Camera" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/emfs" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/DMX512" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/06_App_Component/diskio" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/common" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/printf" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/buflist" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib_server" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/comlib_terminal" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/07_Soft_Component/w5500" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/08_Source" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/app" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/bsp" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/config" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/include" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/kernel" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/core/M4" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/psp/cortex_m/compiler/cwgcc" -I"E:/LED_DMX512/git/DMXlighting/K64_SD_DMX_MASTER/09_MQXLite/" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


