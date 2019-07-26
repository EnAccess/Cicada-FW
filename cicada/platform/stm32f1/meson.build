bin_suffix = 'elf'

platform_src_files = files([
    'irq_stm32.cpp',
    'tick_stm32.cpp',
    'stm32uart.h',
    'stm32uart.cpp'
])

stm32_proj = subproject('STM32Cube')
target_inc = stm32_proj.get_variable('stm32cube_inc')
target_link_args = stm32_proj.get_variable('stm32cube_link_args')
target_deps = stm32_proj.get_variable('stm32cube_dep')
