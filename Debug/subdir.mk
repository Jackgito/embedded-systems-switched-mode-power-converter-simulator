################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../converter_model.c \
../main.c \
../piController.c \
../state_functions.c \
../uart_utils.c 

OBJS += \
./converter_model.o \
./main.o \
./piController.o \
./state_functions.o \
./uart_utils.o 

C_DEPS += \
./converter_model.d \
./main.d \
./piController.d \
./state_functions.d \
./uart_utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o %.su %.cyclo: ../%.c subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F411xE -DSTM32F411RETx -DENABLE_SERIAL=1 -c -I../ -I../external -I../external/CMSIS -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean--2e-

clean--2e-:
	-$(RM) ./converter_model.cyclo ./converter_model.d ./converter_model.o ./converter_model.su ./main.cyclo ./main.d ./main.o ./main.su ./piController.cyclo ./piController.d ./piController.o ./piController.su ./state_functions.cyclo ./state_functions.d ./state_functions.o ./state_functions.su ./uart_utils.cyclo ./uart_utils.d ./uart_utils.o ./uart_utils.su

.PHONY: clean--2e-

