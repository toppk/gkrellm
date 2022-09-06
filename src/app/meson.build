project(
       'rendertest',
       'c',
       version : '1.0.0',
       license : 'GPL',
       meson_version : '>= 0.62.2',
       default_options : []
)

windows = import('windows')

host_system = host_machine.system()

cc = meson.get_compiler('c')

args = []
deps = []

deps += dependency('gtk+-3.0', version : '>= 3.24.34', required : true)
deps += dependency('glib-2.0', version : '>= 2.72.1', required : true)
deps += dependency('gmodule-2.0', version : '>=2.72.1', required : true)

src = ['main.c', 'config.h', 'gui.c', 'widgets.c', 'theme.c']

client = executable('launcher', 
	src,
	dependencies : deps,
	c_args : args,
	win_subsystem : 'windows')

