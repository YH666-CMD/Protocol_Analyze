################################################################################
# MRS Version: 1.9.2
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Peripheral/src/ch32v20x_adc.c \
../Peripheral/src/ch32v20x_bkp.c \
../Peripheral/src/ch32v20x_can.c \
../Peripheral/src/ch32v20x_crc.c \
../Peripheral/src/ch32v20x_dbgmcu.c \
../Peripheral/src/ch32v20x_dma.c \
../Peripheral/src/ch32v20x_exti.c \
../Peripheral/src/ch32v20x_flash.c \
../Peripheral/src/ch32v20x_gpio.c \
../Peripheral/src/ch32v20x_i2c.c \
../Peripheral/src/ch32v20x_iwdg.c \
../Peripheral/src/ch32v20x_misc.c \
../Peripheral/src/ch32v20x_opa.c \
../Peripheral/src/ch32v20x_pwr.c \
../Peripheral/src/ch32v20x_rcc.c \
../Peripheral/src/ch32v20x_rtc.c \
../Peripheral/src/ch32v20x_spi.c \
../Peripheral/src/ch32v20x_tim.c \
../Peripheral/src/ch32v20x_usart.c \
../Peripheral/src/ch32v20x_wwdg.c 

OBJS += \
./Peripheral/src/ch32v20x_adc.o \
./Peripheral/src/ch32v20x_bkp.o \
./Peripheral/src/ch32v20x_can.o \
./Peripheral/src/ch32v20x_crc.o \
./Peripheral/src/ch32v20x_dbgmcu.o \
./Peripheral/src/ch32v20x_dma.o \
./Peripheral/src/ch32v20x_exti.o \
./Peripheral/src/ch32v20x_flash.o \
./Peripheral/src/ch32v20x_gpio.o \
./Peripheral/src/ch32v20x_i2c.o \
./Peripheral/src/ch32v20x_iwdg.o \
./Peripheral/src/ch32v20x_misc.o \
./Peripheral/src/ch32v20x_opa.o \
./Peripheral/src/ch32v20x_pwr.o \
./Peripheral/src/ch32v20x_rcc.o \
./Peripheral/src/ch32v20x_rtc.o \
./Peripheral/src/ch32v20x_spi.o \
./Peripheral/src/ch32v20x_tim.o \
./Peripheral/src/ch32v20x_usart.o \
./Peripheral/src/ch32v20x_wwdg.o 

C_DEPS += \
./Peripheral/src/ch32v20x_adc.d \
./Peripheral/src/ch32v20x_bkp.d \
./Peripheral/src/ch32v20x_can.d \
./Peripheral/src/ch32v20x_crc.d \
./Peripheral/src/ch32v20x_dbgmcu.d \
./Peripheral/src/ch32v20x_dma.d \
./Peripheral/src/ch32v20x_exti.d \
./Peripheral/src/ch32v20x_flash.d \
./Peripheral/src/ch32v20x_gpio.d \
./Peripheral/src/ch32v20x_i2c.d \
./Peripheral/src/ch32v20x_iwdg.d \
./Peripheral/src/ch32v20x_misc.d \
./Peripheral/src/ch32v20x_opa.d \
./Peripheral/src/ch32v20x_pwr.d \
./Peripheral/src/ch32v20x_rcc.d \
./Peripheral/src/ch32v20x_rtc.d \
./Peripheral/src/ch32v20x_spi.d \
./Peripheral/src/ch32v20x_tim.d \
./Peripheral/src/ch32v20x_usart.d \
./Peripheral/src/ch32v20x_wwdg.d 


# Each subdirectory must supply rules for building sources it contributes
Peripheral/src/%.o: ../Peripheral/src/%.c
	@	@	riscv-none-embed-gcc -march=rv32imacxw -mabi=ilp32 -msmall-data-limit=8 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -Wall -Wextra  -g -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Debug" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Core" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\User" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\Peripheral\inc" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\CRC" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\ring_buffer" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\pingpong_buf" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\aes" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\evt" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\log" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\frame" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\00_System\System" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\01_Port\port" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\board" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\clk" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\gpio" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\time" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\02_BSP\wdt" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\05_Scheduler\sched" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\05_Scheduler\sched_tasks" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\heartbeat" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\uart" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\04_Components\print" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\uart_monitor" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\usb" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\06_App\usb_comm" -I"D:\desktop\Protocol_Analyze\Protocol_Analyze\软件B\fw_app\03_Driver\usb\vendor\inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

