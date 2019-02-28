#  TODO: figure out best way to handle compilation for each platform
#  Can we do this based on the cross build file? For linux too, even though it may be native for some?


# Platform specific compiler/linker arguments
mcu_args = []
target_c_args = []
target_cpp_args = []
target_link_args = []
target_deps = []
platform_src_files = []
platform_src_inc = []
bin_suffix = ''

if (meson.is_cross_build() != true)
    bin_suffix = 'bin'
    platform_src_files = files(['../platform/Linux/etick_linux.cpp', '../platform/Linux/etermios.cpp'])
    platform_src_inc = include_directories(['../platform/Linux'])
else
    bin_suffix = 'elf'
    platform_src_files = files(['../platform/STM32F1/etick_stm32.cpp', '../platform/STM32F1/estm32uart.cpp'])
    platform_src_inc = include_directories(['../platform/STM32F1'])

    stm32_proj = subproject('STM32Cube')
    mcu = 'STM32F103xE'
    mcu_args = stm32_proj.get_variable('mcu_args')
    target_link_args = stm32_proj.get_variable('stm32_link_args')
    target_deps = stm32_proj.get_variable('stm32_dep')
endif

###################################
#  LINUX SPECIFIC EXAMPLES HERE
###################################

if (meson.is_cross_build() != true)
    executable(
        'scheduler',
        [ src_files, platform_src_files ],
        'scheduler.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    executable(
        'serial_linux',
        [ src_files, platform_src_files ],
        'serial_linux.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    executable(
        'ipcommdevice',
        [ src_files, platform_src_files ],
        'ipcommdevice.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    executable(
        'blocking',
        [ src_files, platform_src_files ],
        'blocking.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    executable(
        'blockingmqtt',
        [ src_files, platform_src_files ],
        'blockingmqtt.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : [ target_deps, eclipse_paho_mqtt_dep ],
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

endif

####################################
#  STM32F1 SPECIFIC EXAMPLES HERE
####################################

if (meson.is_cross_build() == true)
    blinky_stm32 = executable(
        'blinky_stm32',
        [ src_files, platform_src_files ],
        'blinky_stm32.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    serial_stm32 = executable(
        'serial_stm32',
        [ src_files, platform_src_files ],
        'serial_stm32.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : target_deps,
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    # TODO: 'printf_stm32.cpp' does this example exist?
    #
    # printf_stm32 = executable(
    #     'printf_stm32',
    #     [ src_files, platform_src_files ],
    #     'printf_stm32.cpp',
    #     include_directories : [ src_inc, platform_src_inc ],
    #     dependencies        : [ target_deps, embedded_prinf_dep ],
    #     name_suffix         : bin_suffix,
    #     c_args              : [ mcu_args, target_c_args ],
    #     cpp_args            : [ mcu_args, target_cpp_args ],
    #     link_args           : [ mcu_args, target_link_args ]
    # )

    dualport_stm32 = executable(
        'dualport_stm32',
        [ src_files, platform_src_files ],
        'dualport_stm32.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : [ target_deps, embedded_prinf_dep ],
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args],
        cpp_args            : [ mcu_args, target_cpp_args, '-DE_MULTITON_MAX_INSTANCES=2' ],
        link_args           : [ mcu_args, target_link_args ]
    )

    ipcommdevice_stm32 = executable(
        'ipcommdevice_stm32',
        [ src_files, platform_src_files ],
        'ipcommdevice_stm32.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : [ target_deps, embedded_prinf_dep ],
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args],
        cpp_args            : [ mcu_args, target_cpp_args, '-DE_MULTITON_MAX_INSTANCES=2' ],
        link_args           : [ mcu_args, target_link_args ]
    )

    blockingmqtt_stm32 = executable(
        'blockingmqtt_stm32',
        [ src_files, platform_src_files ],
        'blockingmqtt_stm32.cpp',
        include_directories : [ src_inc, platform_src_inc ],
        dependencies        : [ target_deps, embedded_prinf_dep, eclipse_paho_mqtt_dep ],
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args],
        cpp_args            : [ mcu_args, target_cpp_args, '-DE_MULTITON_MAX_INSTANCES=2' ],
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

    # TODO: 'printf_stm32.cpp' does this example exist?
    #
    # custom_target(
    #     'printf_bin',
    #     output           : [ 'printf_stm.bin' ],
    #     build_by_default : true,
    #     command          : [ objcopy, '-O', 'binary', '-S', 'examples/printf_stm32.elf',
    #                         'examples/printf_stm32.bin' ],
    #     depends          : [ printf_stm32 ]
    # )

    custom_target(
        'dualport_bin',
        output           : [ 'dualport_stm.bin' ],
        build_by_default : true,
        command          : [ objcopy, '-O', 'binary', '-S', 'examples/dualport_stm32.elf',
                            'examples/dualport_stm32.bin' ],
        depends          : [ dualport_stm32 ]
    )

    custom_target(
        'ipcommdevice_bin',
        output           : [ 'ipcommdevice_stm.bin' ],
        build_by_default : true,
        command          : [ objcopy, '-O', 'binary', '-S', 'examples/ipcommdevice_stm32.elf',
                            'examples/ipcommdevice_stm32.bin' ],
        depends          : [ ipcommdevice_stm32 ]
    )

    custom_target(
        'blockingmqtt_bin',
        output           : [ 'blockingmqtt_stm.bin' ],
        build_by_default : true,
        command          : [ objcopy, '-O', 'binary', '-S', 'examples/blockingmqtt_stm32.elf',
                            'examples/blockingmqtt_stm32.bin' ],
        depends          : [ blockingmqtt_stm32 ]
    )

endif
