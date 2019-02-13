cpp_files = ['escheduler.cpp', 'ecircularbuffer.cpp', 'ebufferedserial.cpp',
             'esim7x00.cpp', 'eblockingcommdev.cpp', 'emqttcountdown.cpp']

if (meson.is_cross_build() != true)
  cpp_files += ['platform/etick_linux.cpp', 'platform/etermios.cpp']
else
  cpp_files += ['platform/etick_none.cpp', 'platform/estm32uart.cpp']
endif

src_files = files(cpp_files)
