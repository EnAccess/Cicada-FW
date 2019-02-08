project('EnAccess', 'c', 'cpp')

if (meson.is_cross_build() == true)
  subproject('STM32Cube')
endif

subdir('src')

subdir('dep/paho.mqtt.embedded-c')

subdir('examples')
