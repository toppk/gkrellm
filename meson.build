project(
       'gkrellm',
       'c',
       version : run_command('cat', 'VERSION', check: true).stdout().strip(),
       license : 'GPL',
       meson_version : '>= 0.61.1',
       default_options : []
)

windows = import('windows')

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

glib = dependency('glib-2.0', version : '>= 2.62.6', required : true)
no_memdup2 = glib.version().version_compare('< 2.68.0') 
gmodule = dependency('gmodule-2.0', version : '>= 2.62.6', required : true)


openssl = dependency('openssl', version: '>= 1.1.1o', required : false)
## Only use own md5-code if neither OpenSSL nor GnuTLS are present
#ifneq ($(HAVE_SSL),1)
#    EXTRAOBJS += md5c.o
#endif


enable_nls = get_option('enable_nls')

if host_system == 'windows'
   winpdh = cc.find_library('pdh')
   winnetapi32 = cc.find_library('netapi32')
   winiphlpapi = cc.find_library('iphlpapi')
   winintl = cc.find_library('intl')
   winwtsapi32 = cc.find_library('wtsapi32')
   winws2_32 = cc.find_library('ws2_32')
   windeps = [winws2_32, winwtsapi32, winpdh, winnetapi32, winiphlpapi, winintl]
elif host_system == 'sunos'
      devinfo = cc.find_library('devinfo', required : false)
      md = cc.find_library('md', required : false)
      kstat = cc.find_library('kstat', required : false)
      kvm = cc.find_library('kvm', required : false)
      socket = cc.find_library('socket', required : false)
elif host_system == 'freebsd' or host_system == 'dragonfly' or\
   host_system == 'netbsd' or host_system == 'openbsd'
      devstat = cc.find_library('devstat', required : false)
      md = cc.find_library('md', required : false)
      kvm = cc.find_library('kvm', required : false)
endif




# system options
sysdeps = []
sysargs = []
syssrc = []
if host_system == 'windows'
   syssrc += ['src/sysdeps/win32.c']
elif host_system == 'darwin'
   syssrc += ['src/sysdeps/darwin.c', 'src/sysdeps/bsd-common.c']
elif host_system == 'freebsd'
   syssrc += ['src/sysdeps/freebsd.c', 'src/sysdeps/bsd-common.c',
                 'src/sysdeps/sensors-common.c']
   sysargs += ['-DHAVE_MD5_H']
   sysargs += ['-DSENSORS_COMMON']
   sysdeps += [devstat,md,kvm]
elif host_system == 'dragonfly'
   syssrc += ['src/sysdeps/dragonfly.c', 'src/sysdeps/bsd-common.c',
                 'src/sysdeps/sensors-common.c']
   sysargs += ['-DSENSORS_COMMON']
   sysargs += ['-DHAVE_MD5_H']
   sysdeps += [devstat,md,kvm]
elif host_system == 'netbsd'
   syssrc += ['src/sysdeps/netbsd.c', 'src/sysdeps/bsd-common.c',
                 'src/sysdeps/sensors-common.c']
   sysargs += ['-DHAVE_MD5_H']
   sysdeps += [devstat,md,kvm]
elif host_system == 'openbsd'
   syssrc += ['src/sysdeps/openbsd.c', 'src/sysdeps/bsd-common.c']
   sysargs += ['-DHAVE_MD5_H']
   sysdeps += [devstat,md,kvm]
elif host_system == 'sunos'
   syssrc += ['src/sysdeps/solaris.c']
   sysargs += ['-m32'] # , '-D_FILE_OFFSET_BITS=32']
   sysdeps += [devinfo,md,kvm,kstat,socket]
elif host_system == 'linux'
   syssrc += ['src/sysdeps/linux.c', 'src/sysdeps/sensors-common.c']
   sysargs += ['-DSENSORS_COMMON']
elif libgtop.found()
   sysargs += ['-DUSE_LIBGTOP']
   syssrc += ['src/sysdeps/gtop.c']
endif
sysargs += ['-DHAVE_GETADDRINFO']
if no_memdup2
    sysargs += ['-DMISSING_MEMDUP2']
endif




clientdeps = []
clientargs = []
clientlinks = []
clientlinkwiths = []

if buildclient
   clientdeps += dependency('gtk+-2.0', version : '>=2.24.33', required : true)
   if (host_system != 'windows') and (host_system !=  'darwin')
      clientdeps += dependency('x11', version: '>= 1.6.10', required : true)
      clientdeps += dependency('sm', version: '>= 1.2.3', required : true)
      clientdeps += dependency('ice', version: '>= 1.0.10', required : true)
   endif
   clientdeps += [glib, gmodule]
   math = cc.find_library('m', required : true)
   clientdeps += math
   # optional libraries
   if openssl.found()
      clientdeps += openssl
      clientargs += '-DHAVE_SSL'
   endif
   if lmsensors.found()
      clientdeps += lmsensors
      clientargs += '-DHAVE_LIBSENSORS=1'
   endif
   if libgtop.found()
      clientdeps += libgtop
   endif
   if enable_nls
      clientargs += '-DENABLE_NLS'
   endif
endif

serverdeps = []
serverargs = ['-DGKRELLM_SERVER']
serverlinks = []
serverlinkwiths = []

serverdeps += [glib, gmodule]
if lmsensors.found()
   serverdeps += lmsensors
   serverargs += '-DHAVE_LIBSENSORS=1'
endif
if libgtop.found()
   serverdeps += libgtop
endif
if enable_nls
   serverargs += '-DENABLE_NLS'
endif

# debugging
debugargs = []
if 1 == 1
   debugargs += ['-DGTK_DISABLE_SINGLE_INCLUDES' ]
endif
# this breaks gkrellm on freebsd
if 1 == 0
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



builtins = ['src/builtins/battery.c', 'src/builtins/clock.c', 'src/builtins/cpu.c',
            'src/builtins/disk.c', 'src/builtins/fs.c', 'src/builtins/hostname.c',
            'src/builtins/inet.c', 'src/builtins/mail.c', 'src/builtins/mem.c',
            'src/builtins/net.c', 'src/builtins/proc.c', 'src/builtins/sensors.c',
            'src/builtins/uptime.c']
# client
# common
src = ['src/client/alerts.c', 'src/client/chart.c', 'src/client/client.c',
       'src/client/config.c', 'src/client/gui.c', 'src/client/krell.c',
       'src/client/main.c', 'src/client/panel.c', 'src/client/pixops.c',
       'src/client/plugins.c', 'src/client/utils-client.c']

src += builtins
# gui (+extras for windows)
if host_system == 'windows'
   cwinrsc = windows.compile_resources('src/client/win32-resource.rc')
   src += cwinrsc
   libgkrellm = static_library('gkrellm', 'src/client/win32-libgkrellm.c',
                               c_args : clientargs + debugargs,
                               dependencies : clientdeps)
   src += ['src/client/winops-win32.c', 'src/client/win32-plugin.c']
   src += ['src/shared/md5c.c']
elif host_system == 'darwin'
   src += ['src/client/winops-gtk-mac.c']
else
   src += ['src/client/winops-x11.c']
endif
# sysdeps
src += syssrc




server = ['src/server/mail.c', 'src/server/main.c', 'src/server/monitor.c', 'src/server/plugins.c']
if host_system == 'windows'
   swinrsc = windows.compile_resources('src/server/win32-resource.rc')
   server += swinrsc
   server += ['src/server/win32-plugin.c']
   libgkrellmd = static_library('gkrellmd', 'src/server/win32-libgkrellmd.c',
                                c_args : serverargs + debugargs,
                                dependencies : serverdeps)
endif
server += syssrc

#sharedh = include_directories('src/shared')
shared = ['src/shared/base64.c', 'src/sysdeps/core.c', 'src/shared/log.c', 'src/shared/utils.c']

clientargs += sysargs
serverargs += sysargs
if host_system == 'windows'
   clientdeps += windeps
   clientargs += ['-D_WIN32_WINNT=0x0501', '-DWINVER=0x0501', '-DWIN32_LEAN_AND_MEAN']
   #clientlinks += ['-Wl,--dynamicbase', '-Wl,--nxcompat']
   clientlinkwiths = libgkrellm
   serverdeps += windeps
   serverargs += ['-D_WIN32_WINNT=0x0501', '-DWINVER=0x0501', '-DWIN32_LEAN_AND_MEAN']
   #serverlinks += ['-Wl,--dynamicbase', '-Wl,--nxcompat']
   serverlinkwiths = libgkrellmd
endif

clientdeps += sysdeps
serverdeps += sysdeps

server += shared
src += shared

buildserver = true
#buildclient = false

if buildclient
   client = executable('gkrellm', 
                       src, 
                       dependencies : clientdeps,
                       c_args : clientargs + debugargs,
                       link_args : clientlinks,
                       link_with : clientlinkwiths,
                       win_subsystem : 'windows')
endif
if buildserver
    daemon = executable('gkrellmd',
                        server,
                        dependencies : serverdeps,
                        c_args : serverargs + debugargs,
                        link_args : serverlinks,
                        link_with : serverlinkwiths,
                        win_subsystem : 'console')
endif
