incdir = include_directories('../src')

executable(
    'scheduler',
    src_files,
    'scheduler.cpp',
    include_directories : incdir,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

executable(
    'termios',
    src_files,
    'termios.cpp',
    include_directories : incdir,
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
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

stm32_dep = stm32_proj.get_variable('stm32_dep')
blinky_stm32 = executable(
    'blinky_stm32',
    src_files,
    'blinky_stm32.cpp',
    include_directories : incdir,
    dependencies        : stm32_dep,
    name_suffix         : bin_suffix,
    c_args              : [ mcu_args, target_c_args ],
    cpp_args            : [ mcu_args, target_cpp_args ],
    link_args           : [ mcu_args, target_link_args ]
)

custom_target(
    'bin',
    output           : [ 'blinky_stm.bin' ],
    build_by_default : true,
    command          : [ objcopy, '-O', 'binary', '-S', 'examples/blinky_stm32.elf',
                         'examples/blinky_stm32.bin' ],
    depends          : [ blinky_stm32 ]
)

#executable(
#    'blockingmqtt',
#    src_files,
#    'blockingmqtt.cpp',
#    include_directories : [incdir, eclipse_paho_mqtt_inc],
#    name_suffix         : bin_suffix,
#    c_args              : [ mcu_args, target_c_args ],
#    cpp_args            : [ mcu_args, target_cpp_args ],
#    link_args           : [ mcu_args, target_link_args ]
#)
