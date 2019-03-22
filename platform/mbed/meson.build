platform_src_inc = include_directories(['.'])

platform_src_files = files([
    'mbedserial.h',
    'mbedserial.cpp'
])

mcu = 'STM32F103xB'
mcu_args = ['-Wall', '-Wextra', '-Wno-unused-parameter',
            '-Wno-missing-field-initializers', '-fmessage-length=0',
            '-fno-exceptions', '-fno-builtin', '-ffunction-sections',
            '-fdata-sections', '-funsigned-char', '-MMD',
            '-fno-delete-null-pointer-checks', '-fomit-frame-pointer',
            '-Os', '-g1', '-mcpu=@0@'.format(host_machine.cpu()), '-mthumb']
target_inc = include_directories([
    '../../../ARMmbed-mbed-os',
    '../../../ARMmbed-mbed-os/platform',
    '../../../ARMmbed-mbed-os/targets/TARGET_STM',
    '../../../ARMmbed-mbed-os/targets/TARGET_STM/TARGET_STM32F1',
    '../../../ARMmbed-mbed-os/targets/TARGET_STM/TARGET_STM32F1/device',
    '../../../ARMmbed-mbed-os/targets/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/',
    '../../../ARMmbed-mbed-os/targets/TARGET_STM/TARGET_STM32F1/TARGET_NUCLEO_F103RB/device',
    '../../../ARMmbed-mbed-os/cmsis/TARGET_CORTEX_M'
])
