################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/tinyprintf/printf.c 

OBJS += \
./Drivers/tinyprintf/printf.o 

C_DEPS += \
./Drivers/tinyprintf/printf.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/tinyprintf/%.o Drivers/tinyprintf/%.su Drivers/tinyprintf/%.cyclo: ../Drivers/tinyprintf/%.c Drivers/tinyprintf/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/STM_Project/STM32_F103PS2/Drivers/tinyprintf" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-tinyprintf

clean-Drivers-2f-tinyprintf:
	-$(RM) ./Drivers/tinyprintf/printf.cyclo ./Drivers/tinyprintf/printf.d ./Drivers/tinyprintf/printf.o ./Drivers/tinyprintf/printf.su

.PHONY: clean-Drivers-2f-tinyprintf

