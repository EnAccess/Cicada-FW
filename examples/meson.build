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
