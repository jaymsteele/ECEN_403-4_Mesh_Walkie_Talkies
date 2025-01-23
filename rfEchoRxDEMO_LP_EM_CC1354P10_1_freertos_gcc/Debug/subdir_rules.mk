################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/gcc_arm_none_eabi_9_2_1/bin/arm-none-eabi-gcc-9.2.1.exe" -c -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc" -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc/Debug" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/kernel/freertos" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/ti/posix/gcc" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/third_party/freertos/include" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/third_party/freertos/portable/GCC/ARM_CM33_NTZ/non_secure" -I"C:/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include/newlib-nano" -I"C:/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include" -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-324967569: ../rfEchoRx.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.18.1/sysconfig_cli.bat" --script "C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc/rfEchoRx.syscfg" -o "syscfg" -s "C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/.metadata/product.json" --compiler gcc
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_devices_config.c: build-324967569 ../rfEchoRx.syscfg
syscfg/ti_radio_config.c: build-324967569
syscfg/ti_radio_config.h: build-324967569
syscfg/ti_drivers_config.c: build-324967569
syscfg/ti_drivers_config.h: build-324967569
syscfg/ti_utils_build_linker.cmd.genlibs: build-324967569
syscfg/ti_utils_build_compiler.opt: build-324967569
syscfg/syscfg_c.rov.xs: build-324967569
syscfg/FreeRTOSConfig.h: build-324967569
syscfg/ti_freertos_config.c: build-324967569
syscfg/ti_freertos_portable_config.c: build-324967569
syscfg: build-324967569

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/gcc_arm_none_eabi_9_2_1/bin/arm-none-eabi-gcc-9.2.1.exe" -c -mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc" -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc/Debug" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/kernel/freertos" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/ti/posix/gcc" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/third_party/freertos/include" -I"C:/ti/simplelink_cc13xx_cc26xx_sdk_7_41_00_17/source/third_party/freertos/portable/GCC/ARM_CM33_NTZ/non_secure" -I"C:/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include/newlib-nano" -I"C:/ti/gcc_arm_none_eabi_9_2_1/arm-none-eabi/include" -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/colec/workspace_v12.4/rfEchoRxDEMO_LP_EM_CC1354P10_1_freertos_gcc/Debug/syscfg" -std=c99 $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


