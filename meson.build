project(
       'gkrellm',
       'c',
       version : run_command('cat', 'VERSION', check: true).stdout().strip(),
       license : 'GPL',
       meson_version : '>= 0.62.2',
       default_options : []
)

host_system = host_machine.system()
buildclient = not get_option('onlyserver')
cc = meson.get_compiler('c')
null_dep = dependency('', required : false)

_sensors = get_option('lmsensors')
if _sensors != 'false'
   lmsensors = cc.find_library('libsensors', required : _sensors == 'true')
else
   lmsensors = null_dep
endif

use_libgtop = get_option('use_libgtop')
if use_libgtop
   libgtop = dependency('libgtop-2.0', version: '>= 2.40.0', required : true)
else
   libgtop = null_dep
endif


openssl = dependency('openssl', version: '>= 3.0.5', required : false)

clientdeps = []
clientargs = []

if buildclient
   clientdeps += dependency('gtk+-2.0', version : '>=2.24.33', required : true)
   if (host_system != 'windows') and (host_system !=  'darwin')
      clientdeps += dependency('x11', version: '>= 1.7.3.1', required : true)
      clientdeps += dependency('sm', version: '>= 1.2.3', required : true)
      clientdeps += dependency('ice', version: '>= 1.0.10', required : true)
   endif
   clientdeps += dependency('glib-2.0', version : '>= 2.72.1', required : true)
   clientdeps += dependency('gmodule-2.0', version : '>=2.72.1', required : true)
   math = cc.find_library('m', required : true)
   clientdeps += math
   # optional libraries
   if openssl.found()
      clientdeps += openssl
      clientargs += '-DHAVE_SSL'
   endif
   if libgtop.found()
      clientdeps += libgtop
      clientargs += '-DUSE_LIBGTOP'
   endif
   if lmsensors.found()
      clientdeps += lmsensors
      clientargs += '-DHAVE_LIBSENSORS=1'
    endif
endif

serverdeps = []
serverargs = ['-DGKRELLM_SERVER']
serverdeps += dependency('glib-2.0', version : '>= 2.72.1', required : true)
serverdeps += dependency('gmodule-2.0', version : '>=2.72.1', required : true)
if lmsensors.found()
   serverdeps += lmsensors
   serverargs += '-DHAVE_LIBSENSORS=1'
endif
if libgtop.found()
   serverdeps += libgtop
   serverargs += '-DUSE_LIBGTOP'
endif

# debugging
debugargs = []
if 1 == 1
   debugargs += ['-DGDK_DISABLE_DEPRECATED', '-DGTK_DISABLE_DEPRECATED' ]
endif
if 1 == 1
   debugargs += ['-DGSEAL_ENABLE']
endif
if 1 == 1
   debugargs += ['-Wno-deprecated-declarations']
endif
if 1 == 1
   debugargs += ['-Wno-implicit-function-declaration']
endif


# sysdeps
sysdepargs = []
sysdepsrc = [] 
if host_system == 'windows'
   sysdepsrc += ['src/sysdeps/win32.c']
elif host_system == 'darwin'
   sysdepsrc += ['src/sysdeps/darwin.c', 'src/sysdeps/bsd-common.c']
elif host_system == 'freebsd'
   sysdepsrc += ['src/sysdeps/freebsd.c', 'src/sysdeps/bsd-common.c', 
                 'src/sysdeps/sensors-common.c']
   sysdepargs += ['-DSENSORS_COMMON']
elif host_system == 'dragonfly'
   sysdepsrc += ['src/sysdeps/dragonfly.c', 'src/sysdeps/bsd-common.c', 
                 'src/sysdeps/sensors-common.c']
   sysdepargs += ['-DSENSORS_COMMON']
elif host_system == 'netbsd'
   sysdepsrc += ['src/sysdeps/netbsd.c', 'src/sysdeps/bsd-common.c', 
                 'src/sysdeps/sensors-common.c',
                 'src/sysdeps/bsd-net-open.c']
elif host_system == 'openbsd'
   sysdepsrc += ['src/sysdeps/openbsd.c', 'src/sysdeps/bsd-common.c', 
                 'src/sysdeps/bsd-net-open.c']
elif host_system == 'sunos'
   sysdepsrc += ['src/sysdeps/solaris.c']
elif host_system == 'linux'
   sysdepsrc += ['src/sysdeps/linux.c', 'src/sysdeps/sensors-common.c']
   sysdepargs += ['-DSENSORS_COMMON']
endif


# client
# common
src = ['src/client/alerts.c', 'src/client/base64.c', 'src/client/battery.c', 'src/client/chart.c', 'src/client/client.c',
       'src/client/clock.c', 'src/client/config.c', 'src/client/cpu.c', 'src/client/deprecated.c', 'src/client/disk.c',
       'src/client/fs.c', 'src/client/gui.c', 'src/client/hostname.c', 'src/client/inet.c', 'src/client/krell.c', 'src/client/mail.c',
       'src/client/main.c', 'src/client/md5c.c', 'src/client/mem.c', 'src/client/net.c', 'src/client/panel.c', 'src/client/pixops.c',
       'src/client/plugins.c', 'src/client/sysdeps-unix.c', 'src/client/utils.c']
# gui (+extras for windows)
if host_system == 'windows'
   src += ['src/client/winops-win32.c', 'src/client/win32-plugin.c', 'src/client/win32-libgkrellm.c']
elif host_system == 'darwin'
   src += ['src/client/winops-gtk-mac.c']
else
   src += ['src/client/winops-x11.c']
endif
# sysdeps
src += sysdepsrc

if host_system != 'windows'
   src += [ 'src/client/uptime.c', 'src/client/sensors.c', 'src/client/proc.c']
endif



server = ['src/server/mail.c', 'src/server/main.c', 'src/server/monitor.c', 'src/server/plugins.c',
          'src/server/utils.c', 'src/server/sysdeps-unix.c']
if host_system == 'windows'
   sysdepargs += ['-DWIN32']
   server += ['src/server/win32-libgkrellmd.c', 'src/server/win32-plugin.c']
endif
server += sysdepsrc

sharedh = include_directories('src/shared')
shared = ['src/shared/log.c']


clientargs += sysdepargs
serverargs += sysdepargs

buildserver = true
#buildclient = false

if buildclient
   client = executable('gkrellm', 
                       shared + src, 
                       dependencies : clientdeps,
                       include_directories : sharedh,
                       c_args : clientargs + debugargs)
endif
if buildserver
    server = executable('gkrellmd',
                        shared + server,
                        dependencies : serverdeps,
                        include_directories: sharedh,
                        c_args : serverargs + debugargs)
endif
