# https://github.com/mpaland/printf/

project(
    'printf',
    'c',
    version : '3.1.4'
)

embedded_printf_inc = include_directories([
    './src'
])

embedded_printf_src = files([
    './src/printf.c'
])

embedded_printf_dep = declare_dependency(
    include_directories : [ embedded_printf_inc ],
	sources             : [ embedded_printf_src ]
)
