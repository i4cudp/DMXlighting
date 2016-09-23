################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include 09_MQXLite/psp/cortex_m/core/M4/subdir.mk
-include 09_MQXLite/psp/cortex_m/subdir.mk
-include 09_MQXLite/bsp/subdir.mk
-include 09_MQXLite/app/subdir.mk
-include 08_Source/subdir.mk
-include 07_Soft_Component/w5500/subdir.mk
-include 07_Soft_Component/printf/subdir.mk
-include 07_Soft_Component/common/subdir.mk
-include 07_Soft_Component/comlib_terminal/subdir.mk
-include 07_Soft_Component/comlib_server/subdir.mk
-include 07_Soft_Component/comlib/subdir.mk
-include 07_Soft_Component/buflist/subdir.mk
-include 06_App_Component/light/subdir.mk
-include 06_App_Component/emfs/subdir.mk
-include 06_App_Component/diskio/subdir.mk
-include 06_App_Component/DMX512/subdir.mk
-include 06_App_Component/Camera/subdir.mk
-include 05_Driver/uart/subdir.mk
-include 05_Driver/spi/subdir.mk
-include 05_Driver/sdhc/subdir.mk
-include 05_Driver/gprs/subdir.mk
-include 05_Driver/gpio/subdir.mk
-include 05_Driver/flash/subdir.mk
-include 03_MCU/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
K64_DMX_SLAVE.hex \

SECONDARY_LIST += \
K64_DMX_SLAVE.lst \


# All Target
all: K64_DMX_SLAVE.elf secondary-outputs

# Tool invocations
K64_DMX_SLAVE.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C++ Linker'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -T "intflash.ld" -nostartfiles -Xlinker --gc-sections -L"E:/LED_DMX512/K64_SD_DMX_SLAVE/04_Linker_File" -L"E:\LED_DMX512\K64_SD_DMX_SLAVE\09_MQXLite\kernel" -Wl,-Map,"K64_DMX_SLAVE.map" -specs=nano.specs -specs=nosys.specs -o "K64_DMX_SLAVE.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

K64_DMX_SLAVE.hex: K64_DMX_SLAVE.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "K64_DMX_SLAVE.elf"  "K64_DMX_SLAVE.hex"
	@echo 'Finished building: $@'
	@echo ' '

K64_DMX_SLAVE.lst: K64_DMX_SLAVE.elf
	@echo 'Invoking: Cross ARM GNU Create Listing'
	arm-none-eabi-objdump --source --all-headers --demangle --line-numbers --wide "K64_DMX_SLAVE.elf" > "K64_DMX_SLAVE.lst"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(ASM_DEPS)$(CC_DEPS)$(SECONDARY_FLASH)$(CPP_DEPS)$(CXX_DEPS)$(C_UPPER_DEPS)$(SECONDARY_LIST)$(S_UPPER_DEPS) K64_DMX_SLAVE.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
