incdir = include_directories('../src')

executable(
    'scheduler',
    src_files,
    'scheduler.cpp',
    include_directories : incdir,
    dependencies        : target_deps,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

executable(
    'serial_linux',
    src_files,
    'serial_linux.cpp',
    include_directories : incdir,
    dependencies        : target_deps,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

executable(
    'ipcommdevice',
    src_files,
    'ipcommdevice.cpp',
    include_directories : incdir,
    dependencies        : target_deps,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

executable(
    'blocking',
    src_files,
    'blocking.cpp',
    include_directories : incdir,
    dependencies        : target_deps,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

executable(
   'blockingmqtt',
   src_files,
   'blockingmqtt.cpp',
   eclipse_paho_mqtt_src,
   include_directories : [incdir, eclipse_paho_mqtt_inc],
   dependencies        : target_deps,
   name_suffix         : bin_suffix,
   c_args              : [ mcu_args, target_c_args ],
   cpp_args            : [ mcu_args, target_cpp_args ],
   link_args           : [ mcu_args, target_link_args ]
)

if (meson.is_cross_build() == true)
  blinky_stm32 = executable(
      'blinky_stm32',
      src_files,
      'blinky_stm32.cpp',
      include_directories : incdir,
      dependencies        : target_deps,
      name_suffix         : bin_suffix,
      c_args              : [ mcu_args, target_c_args ],
      cpp_args            : [ mcu_args, target_cpp_args ],
      link_args           : [ mcu_args, target_link_args ]
  )

  serial_stm32 = executable(
      'serial_stm32',
      src_files,
      'serial_stm32.cpp',
      include_directories : incdir,
      dependencies        : target_deps,
      name_suffix         : bin_suffix,
      c_args              : [ mcu_args, target_c_args ],
      cpp_args            : [ mcu_args, target_cpp_args ],
      link_args           : [ mcu_args, target_link_args ]
  )

  custom_target(
      'blinky_bin',
      output           : [ 'blinky_stm.bin' ],
      build_by_default : true,
      command          : [ objcopy, '-O', 'binary', '-S', 'examples/blinky_stm32.elf',
                           'examples/blinky_stm32.bin' ],
      depends          : [ blinky_stm32 ]
  )
  custom_target(
      'serial_bin',
      output           : [ 'serial_stm.bin' ],
      build_by_default : true,
      command          : [ objcopy, '-O', 'binary', '-S', 'examples/serial_stm32.elf',
                           'examples/serial_stm32.bin' ],
      depends          : [ serial_stm32 ]
  )
endif
