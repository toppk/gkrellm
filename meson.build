project('gkrellm', 'c',
version: '2.99.0',
license: 'GPL')
host_system = host_machine.system()

gtk = dependency('gtk+-2.0', version : '>=2.24.33')
glib = dependency('glib-2.0', version : '>= 2.72.3')
x11 = dependency('x11', version: '>= 1.7.3.1')
x11sm = dependency('sm', version: '>= 1.2.3')
x11ice = dependency('ice', version: '>= 1.0.10')
openssl = dependency('openssl', version: '>= 3.0.5', required : false)
gmodule = dependency('gmodule-2.0', version : '>=2.72.3')
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

if openssl.found()
   extra_args = ['-DHAVE_SSL']
else
   extra_args = []
endif

if 1 == 1
   extra_args += ['-DGDK_DISABLE_DEPRECATED', '-DGTK_DISABLE_DEPRECATED' ]
endif
if 1 == 1
   extra_args += ['-DGSEAL_ENABLE']
endif
if 1 == 1
   extra_args += ['-Wno-deprecated-declarations']
endif
if 1 == 1
   extra_args += ['-Wno-implicit-function-declaration']
endif


src = ['src/alerts.c', 'src/base64.c', 'src/battery.c', 'src/chart.c', 'src/client.c',
       'src/clock.c', 'src/config.c', 'src/cpu.c', 'src/deprecated.c', 'src/disk.c',
       'src/fs.c', 'src/gui.c', 'src/hostname.c', 'src/inet.c', 'src/krell.c', 'src/mail.c',
       'src/main.c', 'src/md5c.c', 'src/mem.c', 'src/net.c', 'src/panel.c', 'src/pixops.c',
       'src/plugins.c', 'src/proc.c', 'src/sensors.c', 'src/sysdeps-unix.c', 'src/uptime.c',
       'src/utils.c']

if host_system == 'darwin'
   src += ['src/winops-gtk-mac.c']
elif host_system == 'windows'
   src += ['src/win32-libgkrellm.c', 'src/win32-plugin.c', 'src/winops-win32.c']
else
   src += [ 'src/winops-x11.c']
endif


server = ['server/mail.c', 'server/main.c', 'server/monitor.c', 'server/plugins.c',
          'server/sysdeps-unix.c', 'server/utils.c']
if host_system == 'windows'
   server += ['server/win32-libgkrellmd.c', 'server/win32-plugin.c']
endif

sharedh = include_directories('shared')
shared = ['shared/log.c']

host_system = host_machine.system()

client = executable('gkrellm', shared + src, dependencies : [gtk, glib, gmodule, x11, x11sm, x11ice, openssl, m_dep], include_directories: sharedh, c_args: extra_args)
daemon = executable('gkrellmd', shared + server, dependencies : [glib, gmodule], include_directories: sharedh, c_args: extra_args + ['-DGKRELLM_SERVER'])
