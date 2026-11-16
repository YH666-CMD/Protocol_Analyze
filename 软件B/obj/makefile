################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include fw_app/06_App/usb_comm/subdir.mk
-include fw_app/06_App/uart_monitor/subdir.mk
-include fw_app/06_App/spi_analyze/subdir.mk
-include fw_app/06_App/pwm_measure/subdir.mk
-include fw_app/06_App/pwm_gen/subdir.mk
-include fw_app/06_App/ota_service/subdir.mk
-include fw_app/06_App/i2c_comm/subdir.mk
-include fw_app/06_App/heartbeat/subdir.mk
-include fw_app/06_App/adc_analyze/subdir.mk
-include fw_app/05_Scheduler/sched_tasks/subdir.mk
-include fw_app/05_Scheduler/sched/subdir.mk
-include fw_app/04_Components/ring_buffer/subdir.mk
-include fw_app/04_Components/print/subdir.mk
-include fw_app/04_Components/pingpong_buf/subdir.mk
-include fw_app/04_Components/ota_boot/subdir.mk
-include fw_app/04_Components/log/subdir.mk
-include fw_app/04_Components/frame/subdir.mk
-include fw_app/04_Components/evt/subdir.mk
-include fw_app/04_Components/aes/subdir.mk
-include fw_app/04_Components/CRC/subdir.mk
-include fw_app/03_Driver/usb/vendor/src/subdir.mk
-include fw_app/03_Driver/usb/subdir.mk
-include fw_app/03_Driver/uart/subdir.mk
-include fw_app/03_Driver/spi/subdir.mk
-include fw_app/03_Driver/pwm_out/subdir.mk
-include fw_app/03_Driver/pwm_in/subdir.mk
-include fw_app/03_Driver/i2c/subdir.mk
-include fw_app/03_Driver/adc/subdir.mk
-include fw_app/02_BSP/wdt/subdir.mk
-include fw_app/02_BSP/time/subdir.mk
-include fw_app/02_BSP/gpio/subdir.mk
-include fw_app/02_BSP/dma/subdir.mk
-include fw_app/02_BSP/clk/subdir.mk
-include fw_app/02_BSP/board/subdir.mk
-include fw_app/01_Port/port/subdir.mk
-include fw_app/00_System/System/subdir.mk
-include User/subdir.mk
-include Startup/subdir.mk
-include Peripheral/src/subdir.mk
-include Debug/subdir.mk
-include Core/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
CH32V203C8T6.hexAndbin \

SECONDARY_LIST += \
CH32V203C8T6.lst \

SECONDARY_SIZE += \
CH32V203C8T6.siz \


# 所有目标
all: CH32V203C8T6.elf secondary-outputs

# 工具调用
CH32V203C8T6.elf: $(OBJS) $(USER_OBJS)
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -Wall -Wextra  -g -T "D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map,"CH32V203C8T6.map" --specs=nano.specs --specs=nosys.specs  -o "CH32V203C8T6.elf" $(OBJS) $(USER_OBJS) $(LIBS) -lIQmath_RV32 -lprintf
	@	@
CH32V203C8T6.hexAndbin: CH32V203C8T6.elf
	@	riscv-none-embed-objcopy -O ihex "CH32V203C8T6.elf"  "CH32V203C8T6.hex" && riscv-none-embed-objcopy -O binary "CH32V203C8T6.elf"  "CH32V203C8T6.bin"
	@	@
CH32V203C8T6.lst: CH32V203C8T6.elf
	@	riscv-none-embed-objdump --all-headers --demangle --disassemble -M xw "CH32V203C8T6.elf" > "CH32V203C8T6.lst"
	@	@
CH32V203C8T6.siz: CH32V203C8T6.elf
	@	riscv-none-embed-size --format=berkeley "CH32V203C8T6.elf"
	@	@
	$(MAKE) --no-print-directory post-build

# 其他目标
clean:
	-$(RM) $(ASM_UPPER_DEPS)$(OBJS)$(SECONDARY_FLASH)$(SECONDARY_LIST)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_DEPS)$(S_UPPER_DEPS)$(C_DEPS) CH32V203C8T6.elf
	-@
post-build:
	-riscv-none-embed-objcopy -O ihex "CH32V203C8T6.elf" "CH32V203C8T6.hex" && riscv-none-embed-objcopy -O binary "CH32V203C8T6.elf" "CH32V203C8T6.bin"
	-@
secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_LIST) $(SECONDARY_SIZE)

.PHONY: all clean dependents post-build

-include ../makefile.targets
