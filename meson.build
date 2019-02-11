project('EnAccess', 'c', 'cpp')

# Import binary helpers
objcopy  = '@0@'.format(find_program('objcopy').path())
objdump  = '@0@'.format(find_program('objdump').path())
size     = '@0@'.format(find_program('size').path())
gdb      = '@0@'.format(find_program('gdb').path())

# Platform specific compiler/linker arguments
mcu_args = []
target_c_args = []
target_cpp_args = []
target_link_args = []
bin_suffix = []

if (meson.is_cross_build() == true)
  stm32_proj = subproject('STM32Cube')
  mcu = 'STM32F103xE'
  mcu_args = stm32_proj.get_variable('mcu_args')
  target_link_args = stm32_proj.get_variable('stm32_link_args')
  bin_suffix = 'elf'
endif

# Actually process the code
subdir('src')
subdir('dep/paho.mqtt.embedded-c')
subdir('examples')
