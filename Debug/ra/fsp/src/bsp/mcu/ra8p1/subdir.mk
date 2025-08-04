################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/mcu/ra8p1/bsp_linker.c 

C_DEPS += \
./ra/fsp/src/bsp/mcu/ra8p1/bsp_linker.d 

CREF += \
PDM.cref 

OBJS += \
./ra/fsp/src/bsp/mcu/ra8p1/bsp_linker.o 

MAP += \
PDM.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/mcu/ra8p1/%.o: ../ra/fsp/src/bsp/mcu/ra8p1/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m85 -mthumb -mlittle-endian -mfloat-abi=hard -O2 -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -g3 -std=c99 -flax-vector-conversions -fshort-enums -fno-unroll-loops -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/src" -I"." -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra/fsp/inc" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra/fsp/inc/api" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra/fsp/inc/instances" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra_gen" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra_cfg/fsp_cfg/bsp" -I"/Users/kim_kyoungkun/Desktop/technonia/PDM/ra_cfg/fsp_cfg" -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

