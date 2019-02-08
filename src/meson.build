cpp_files = ['escheduler.cpp', 'ecircularbuffer.cpp',
             'ebufferedserial.cpp', 'esim7x00.cpp', 'eblockingcommdev.cpp']

if (meson.is_cross_build() != true)
  cpp_files += ['platform/etick_linux.cpp', 'platform/etermios.cpp']
else
  cpp_files += ['platform/etick_none.cpp']
endif

src_files = files(cpp_files)
