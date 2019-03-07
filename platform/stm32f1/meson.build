bin_suffix = 'elf'

platform_src_inc = include_directories(['.'])

platform_src_files = files([
    'tick_stm32.cpp',
    'stm32uart.h',
    'stm32uart.cpp'
])

stm32_proj = subproject('STM32Cube')
mcu = 'STM32F103xE'
mcu_args = stm32_proj.get_variable('mcu_args')
target_link_args = stm32_proj.get_variable('stm32_link_args')
target_deps = stm32_proj.get_variable('stm32_dep')
