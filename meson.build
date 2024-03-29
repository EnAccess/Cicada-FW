project(
    'Cicada - IoT Comms Library',
    'c', 'cpp',
    version: '0.0.1',
    subproject_dir: 'dep'
)

# Debugging options
debug_args = []

# Uncomment next line to enable debug log output
# debug_args += '-DCICADA_DEBUG'

# Import binary helpers
python       = find_program('python3', 'python', required: false)
clangFormat  = find_program('clang-format',  required: false)

if (meson.is_cross_build() == true)
    objcopy  = '@0@'.format(find_program('objcopy').path())
endif

# Add library sources
subdir('cicada')
src_files = get_variable('src_files')

# Add platform specific sources and compiler/linker arguments
mcu_args = []
target_inc = []
target_c_args = []
target_cpp_args = []
target_link_args = []
target_deps = []
platform_src_files = []
bin_suffix = []

if (meson.is_cross_build() != true)
    cicada_build_target = 'linux'
else
    cicada_build_target = meson.get_cross_property('cicada_build_target')
endif

## Add printf dependancy
embedded_printf       = subproject('printf')
embedded_printf_dep   = embedded_printf.get_variable('embedded_printf_dep')

# Add mqtt dependancy
eclipse_paho_mqtt     = subproject('paho.mqtt.embedded-c')
eclipse_paho_mqtt_dep = eclipse_paho_mqtt.get_variable('eclipse_paho_mqtt_dep')
eclipse_paho_mqtt_inc = eclipse_paho_mqtt.get_variable('eclipse_paho_mqtt_inc')

# Include directory when used as submodule
cicada_inc = include_directories(['.'])

# Actually build library
subdir('cicada/platform/' + cicada_build_target)
cicada_lib = static_library(
    'Cicada',
    [ src_files, platform_src_files ],
    include_directories : [ target_inc  ],
    dependencies        : [ embedded_printf_dep, eclipse_paho_mqtt_dep ],
    c_args              : [ mcu_args, target_c_args, debug_args ],
    cpp_args            : [ mcu_args, target_cpp_args, debug_args ],
    link_args           : [ mcu_args, target_link_args ],
    pic                 : false
)
cicada_dep = declare_dependency(
    include_directories : cicada_inc,
    link_with           : cicada_lib
)

# Only build examples and tests when not a subproject

if (meson.is_subproject() != true)

    # Build examples
    subdir('examples/' + cicada_build_target)
    subdir('examples')


    # Add unit test src
    subdir('test')
    test_src_inc   = get_variable('test_src_inc')
    test_src_files = get_variable('test_src_files')

    # Add CppUTest dependancy
    cpputest     = subproject('cpputest')
    cpputest_dep = cpputest.get_variable('cpputest_dep')

    # Unit test args
    test_args = []

    # Build native unit tests
    run_tests = executable(
        'run_tests',
        [ test_src_files, src_files, './test/main.cpp' ],
        include_directories : [ test_src_inc ],
        dependencies        : [ embedded_printf_dep, cpputest_dep ],
        c_args              : [ '-std=c11', test_args ],
        cpp_args            : [ '-std=c++11', test_args ],
        native              : true,
        build_by_default    : false
    )

    # Unit test
    test('cpputest', run_tests)

    # Setup custom build commands
    run_target('lint',      command: [ 'clang-format', '-verbose',
                                       '-style=file', '-i', src_files,
                                       platform_src_files, test_src_files ])
    run_target('doc',       command: [ 'doxygen', 'Doxyfile' ])

endif

message('''

Build commands:
    ninja           compile
    ninja clean     clean
    ninja lint      prints a diff for files that do not match the style guide
    ninja doc       create documentation with doxyen
'''
)
