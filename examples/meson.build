foreach example : examples_list
    example_exec = executable(
        example,
        cicada_build_target + '/' + example + '.cpp',
        include_directories : [ include_directories('..'),
                                eclipse_paho_mqtt_inc, embedded_printf_inc ],
        dependencies        : [ target_deps, cicada_dep ],
        name_suffix         : bin_suffix,
        c_args              : [ mcu_args, target_c_args ],
        cpp_args            : [ mcu_args, target_cpp_args ],
        link_args           : [ mcu_args, target_link_args ]
    )

    if (meson.is_cross_build() == true)
        if (cicada_build_target == 'stm32f1')
            custom_target(
                example + '.bin',
                output           : [ example + '.bin' ],
                build_by_default : true,
                command          : [ objcopy, '-O', 'binary', '-S', 'examples/' + example + '.elf',
                                    'examples/' + example + '.bin' ],
                depends          : [ example_exec ]
            )
            custom_target(
                example + '.elf',
                output           : [ example + '.hex' ],
                build_by_default : true,
                command          : [ objcopy, '-O', 'ihex', '-S', 'examples/' + example + '.elf',
                                    'examples/' + example + '.hex' ],
                depends          : [ example_exec ]
            )
        endif
    endif
endforeach
