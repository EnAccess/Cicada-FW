project(
    'EnAccess - IoT Comms Library',
    'c',
    'cpp',
    version: '0.0.1',
    subproject_dir: 'dep',
    default_options:  [ 'c_std=c11', 'cpp_std=gnu++11' ]
)

# Import binary helpers
python       = find_program('python3', 'python', required: false)
clangFormat  = find_program('clang-format',  required: false)

if (meson.is_cross_build() == true)
    objcopy  = '@0@'.format(find_program('objcopy').path())
    objdump  = '@0@'.format(find_program('objdump').path())
    size     = '@0@'.format(find_program('size').path())
    gdb      = '@0@'.format(find_program('gdb').path())
    openocd  = '@0@'.format(find_program('openocd', required: false).path())
endif

# Add src 
subdir('src')
src_inc   = get_variable('src_inc')
src_files = get_variable('src_files')

# Add platform specific sources and compiler/linker arguments
mcu_args = []
target_c_args = []
target_cpp_args = []
target_link_args = []
target_deps = []
platform_src_files = []
platform_src_inc = []
bin_suffix = []
if (meson.is_cross_build() != true)
    subdir('platform/linux')
else
    subdir('platform/stm32f1')
endif

# Add embedded printf dependancy
embedded_printf    = subproject('printf')
embedded_prinf_dep = embedded_printf.get_variable('embedded_prinf_dep')

# Add mqtt dependancy
eclipse_paho_mqtt     = subproject('paho.mqtt.embedded-c')
eclipse_paho_mqtt_dep = eclipse_paho_mqtt.get_variable('eclipse_paho_mqtt_dep')

# Setup examples
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
    include_directories : [ test_src_inc, src_inc ],
    dependencies        : [ cpputest_dep ],
    c_args              : [ '-std=c11', test_args ],
    cpp_args            : [ '-std=c++11', test_args ],
    native              : true,
    build_by_default    : false
)

# Setup custom build commands
run_target('lint',      command: [ 'clang-format', '-verbose', '-style=file', '-i', src_files, platform_src_files, test_src_files ])
run_target('ut',        command: [ run_tests, '-c' ], depends: run_tests)

message('''

Build commands:
    ninja           compile
    ninja clean     clean
    ninja lint      prints a diff for files that do not match the style guide
    ninja ut        run unit tests - note: `ninja test` is reserved and doesn't do anything
    
'''
)
