project(
    'STM32Cube',
    'c',
    'cpp'
    version : '1.6.1'
)

# MCU compiler args
mcu = 'STM32F103xE'
mcu_args = []
mcu_args += '-mcpu=@0@'.format(host_machine.cpu())
mcu_args += '-D@0@'.format(mcu)
mcu_args += '-mthumb'
mcu_args += '-Os'                  # optimise for size
mcu_args += '-Wall'                # enable all compiler warnings
mcu_args += '-fdata-sections'      # each function to a seperate section ==> Code-optimization / deletion
mcu_args += '-ffunction-sections'  # each variable to a seperate section ==> Code-optimization / deletion
mcu_args += '-fno-exceptions'      # is this needed?
mcu_args += '-fno-builtin'         # is this needed?
mcu_args += '-ggdb'                # for attaching a debugger   (alternates -g -gdwarf-2)

# Linker args
stm32_link_args = []
stm32_link_args += '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'config/linker_STM32F103RCTx.ld')
stm32_link_args += '--specs=nosys.specs'
stm32_link_args += '--specs=nano.specs'
stm32_link_args += '-Wl,--gc-sections'

# Defines for the HAL Driver
stm32_c_args = []
stm32_c_args += '-DUSE_HAL_DRIVER'
# stm32_c_args += '-DUSE_LEGACY'

# Include directories
stm32_inc = include_directories([
    'config',
    'Drivers/CMSIS/Include',
    'Drivers/CMSIS/Device/ST/STM32F1xx/Include',
    'Drivers/STM32F1xx_HAL_Driver/Inc',
    'Drivers/STM32F1xx_HAL_Driver/Inc/Legacy'
])

# Source files
stm32_src = files([
    'config/startup_stm32f103xe.s',
    'Drivers/CMSIS/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cec.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_crc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_eth.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_hcd.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2s.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_irda.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_mmc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nand.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nor.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pccard.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sd.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_smartcard.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sram.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_wwdg.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_adc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_crc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dac.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_fsmc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rtc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_sdmmc.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_spi.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_tim.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usart.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usb.c',
    'Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.c'
])

# Import binary helpers
objcopy  = '@0@'.format(find_program('objcopy').path())
objdump  = '@0@'.format(find_program('objdump').path())
size     = '@0@'.format(find_program('size').path())
gdb      = '@0@'.format(find_program('gdb').path())

# Generate library
stm32_lib = static_library(
    'STM32Cube',
    [ stm32_src ],
    include_directories : [ stm32_inc ],
    c_args              : [ mcu_args, stm32_c_args ],
    cpp_args            : [ mcu_args ],
    link_args           : [ mcu_args, stm32_link_args ]
)

stm32_dep = declare_dependency(
    include_directories : stm32_inc,
    sources : stm32_src,
    link_with : stm32_lib
)
