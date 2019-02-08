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
  subproject('STM32Cube')
  mcu = 'STM32F103xE'
  mcu_args += '-mcpu=@0@'.format(host_machine.cpu())
  mcu_args += '-D@0@'.format(mcu)
  mcu_args += '-mthumb'
  mcu_args += '-Os'                  # optimise for size
  mcu_args += '-Wall'                # enable all compiler warnings
  mcu_args += '-fdata-sections'      # each function to a seperate section ==> Code-optimization / deletion
  mcu_args += '-ffunction-sections'  # each variable to a seperate section ==> Code-optimization / deletion
  mcu_args += '-fno-exceptions'      # no excpetions used in this code
  mcu_args += '-fno-builtin'         # Allow to override builtin functions
  mcu_args += '-ggdb'                # for attaching a debugger   (alternates -g -gdwarf-2)
  target_link_args += '-Wl,-T,@0@/@1@'.format(meson.current_source_dir(), 'config/linker_STM32F103RCTx.ld')
  target_link_args += '--specs=nosys.specs'
  target_link_args += '--specs=nano.specs'
  target_link_args += '-Wl,--gc-sections'
  target_c_args += '-DUSE_HAL_DRIVER'
  bin_suffix = 'elf'
endif

# Actually process the code
subdir('src')
subdir('dep/paho.mqtt.embedded-c')
subdir('examples')
