project(
    'EnAccess - IoT Comms Library',
    'c',
    'cpp',
    version: '0.0.1',
    subproject_dir: 'dep',
    default_options:  [ 'c_std=c11', 'cpp_std=gnu++11' ]
)

# Import binary helpers
python   = find_program('python3', 'python')
clangFormat  = find_program('clang-format')

if (meson.is_cross_build() == true)
    objcopy  = '@0@'.format(find_program('objcopy').path())
    objdump  = '@0@'.format(find_program('objdump').path())
    size     = '@0@'.format(find_program('size').path())
    gdb      = '@0@'.format(find_program('gdb').path())
    openocd  = '@0@'.format(find_program('openocd', required: false).path())
endif

# Platform specific compiler/linker arguments
mcu_args = []
target_c_args = []
target_cpp_args = []
target_link_args = []
bin_suffix = []
target_deps = []


# if (meson.is_cross_build() == true)
#   stm32_proj = subproject('STM32Cube')
#   mcu = 'STM32F103xE'
#   mcu_args = stm32_proj.get_variable('mcu_args')
#   target_link_args = stm32_proj.get_variable('stm32_link_args')
#   target_deps = stm32_proj.get_variable('stm32_dep')
#   bin_suffix = 'elf'
# endif

# subdir('dep')
# subdir('examples')


# Add src dependancies
subdir('src')
src_inc   = get_variable('src_inc')
src_files = get_variable('src_files')

# Add unit test src dependencies
subdir('test')
test_src_inc   = get_variable('test_src_inc')
test_src_files = get_variable('test_src_files')

# Add CppUTest dependancy
cpputest     = subproject('cpputest')
cpputest_dep = cpputest.get_variable('cpputest_dep')

# Unit test args
test_args = []
# test_args += '-D@0@'.format(mcu)
# test_args += '-D@0@'.format('OKRA_UNIT_TEST_BUILD')
# test_args += '-W@0@'.format('no-unused-variable')      # HAL Mocks causes these annoying warnings, disable in the test build
# test_args += '-W@0@'.format('no-int-to-pointer-cast')  # HAL Mocks causes these annoying warnings, disable in the test build

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
run_target('lint',      command: [ python, '.clang-format.py', '-r', '-e', 'dep', 'src', 'test' ])  # TODO: confirm these -e exclude args are correct
run_target('ut',        command: [ run_tests, '-c' ], depends: run_tests)

message('''

Build commands:
    ninja           compile
    ninja clean     clean
    ninja lint      prints a diff for files that do not match the style guide
    ninja ut        run unit tests - note: `ninja test` is reserved and doesn't do anything
'''
)
