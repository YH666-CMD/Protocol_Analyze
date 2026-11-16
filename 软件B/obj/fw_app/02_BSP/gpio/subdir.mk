################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fw_app/02_BSP/gpio/bsp_gpio.c 

OBJS += \
./fw_app/02_BSP/gpio/bsp_gpio.o 

C_DEPS += \
./fw_app/02_BSP/gpio/bsp_gpio.d 


# Each subdirectory must supply rules for building sources it contributes
fw_app/02_BSP/gpio/%.o: ../fw_app/02_BSP/gpio/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -Wall -Wextra  -g -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Debug" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Core" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\User" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Peripheral\inc" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\CRC" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\ring_buffer" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\pingpong_buf" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\aes" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\evt" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\log" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\frame" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\00_System\System" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\01_Port\port" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\board" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\clk" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\gpio" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\time" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\wdt" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\05_Scheduler\sched" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\05_Scheduler\sched_tasks" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\heartbeat" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\uart" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\print" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\uart_monitor" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\usb" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\usb_comm" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\usb\vendor\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

