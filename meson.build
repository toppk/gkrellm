project(
       'gkrellm',
       'c',
       version : '2.5.0-pre1',
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

glib = dependency('glib-2.0', version : '>= 2.62.6', required : true)
no_memdup2 = glib.version().version_compare('< 2.68.0') 
gmodule = dependency('gmodule-2.0', version : '>= 2.62.6', required : true)

disable_openssl = get_option('disable_openssl')
enable_gnutls = get_option('enable_gnutls')

if disable_openssl or enable_gnutls
	openssl = null_dep
else
	openssl = dependency('openssl', version: '>= 1.1.1o', required : false)
endif

if enable_gnutls or not openssl.found()
        gnutls = dependency('gnutls', version: '>= 3.6.16.1', required : enable_gnutls)
else
	gnutls = null_dep
endif

md = null_dep

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
      md = cc.find_library('md5', required : false)
      kstat = cc.find_library('kstat', required : false)
      kvm = cc.find_library('kvm', required : false)
      socket = cc.find_library('socket', required : true)
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

have_openssl = false
have_gnutls = false
have_md5 = false
if gnutls.found()
	sysdeps += [gnutls]
	md = null_dep
        have_gnutls = true
elif openssl.found()
	sysdeps += [openssl]
	md = null_dep
        have_openssl = true
endif
if host_system == 'windows'
   syssrc += ['src/sysdeps/win32.c']
elif host_system == 'darwin'
   syssrc += ['src/sysdeps/darwin.c', 'src/sysdeps/bsd-common.c']
elif host_system == 'freebsd'
   syssrc += ['src/sysdeps/freebsd.c', 'src/sysdeps/bsd-common.c',
                 'src/sysdeps/sensors-common.c']
	if md.found()
		have_md5 = true
		sysdeps += [md]
	endif
   sysargs += ['-DSENSORS_COMMON']
   sysdeps += [devstat,kvm]
elif host_system == 'dragonfly'
   syssrc += ['src/sysdeps/dragonfly.c', 'src/sysdeps/bsd-common.c',
              'src/sysdeps/sensors-common.c']
   sysargs += ['-DSENSORS_COMMON']
	if md.found()
		have_md5 = true
		sysdeps += [md]
	endif
	sysdeps += [devstat,kvm]
elif host_system == 'netbsd'
   syssrc += ['src/sysdeps/netbsd.c', 'src/sysdeps/bsd-common.c',
                 'src/sysdeps/sensors-common.c']
	if md.found()
		have_md5 = true
		sysdeps += [md]
	endif
	sysdeps += [devstat,kvm]
elif host_system == 'openbsd'
   syssrc += ['src/sysdeps/openbsd.c', 'src/sysdeps/bsd-common.c']
	if md.found()
		have_md5 = true
		sysdeps += [md]
	endif
	sysdeps += [devstat,kvm]
elif host_system == 'sunos'
   syssrc += ['src/sysdeps/solaris.c']
   sysargs += ['-m32'] # , '-D_FILE_OFFSET_BITS=32']
	if md.found()
		have_md5 = true
		sysdeps += [md]
	endif
   sysdeps += [devinfo,kvm,kstat,socket]
elif host_system == 'linux'
   syssrc += ['src/sysdeps/linux.c', 'src/sysdeps/sensors-common.c']
   sysargs += ['-DSENSORS_COMMON']
endif
if not openssl.found() and not md.found()
   syssrc += ['src/shared/md5.c']
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
   if lmsensors.found()
      clientdeps += lmsensors
      clientargs += '-DHAVE_LIBSENSORS=1'
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
   cwinrsc = windows.compile_resources(join_paths(meson.current_build_dir(),'win32-resource-client.rc'))
   src += cwinrsc
   libgkrellm = static_library('gkrellm', 'src/client/win32-libgkrellm.c',
                               c_args : clientargs + debugargs,
                               dependencies : clientdeps)
   src += ['src/client/winops-win32.c', 'src/client/win32-plugin.c']
elif host_system == 'darwin'
   src += ['src/client/winops-gtk-mac.c']
else
   src += ['src/client/winops-x11.c']
endif
# sysdeps




server = ['src/server/mail.c', 'src/server/main.c', 'src/server/monitor.c', 'src/server/plugins.c']
if host_system == 'windows'
   swinrsc = windows.compile_resources(join_paths(meson.current_build_dir(),'win32-resource-server.rc'))
   server += swinrsc
   server += ['src/server/win32-plugin.c']
   libgkrellmd = static_library('gkrellmd', 'src/server/win32-libgkrellmd.c',
                                c_args : serverargs + debugargs,
                                dependencies : serverdeps)
endif

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

src += syssrc + shared
server += syssrc + shared

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


config_h = configuration_data()
config_h.set('HAVE_SSL', have_openssl)
config_h.set('HAVE_TLS', have_gnutls)
config_h.set('HAVE_MD5', have_md5)
config_h.set('MISSING_MEMDUP2', no_memdup2)
config_h.set('HAVE_GETADDRINFO', true)

configure_file(
  input: 'config.h.in',
  output: 'config.h',
  configuration: config_h,
)


full_version = meson.project_version()
version_split = full_version.split('-')
version_components = version_split[0].split('.')

if version_split.length() == 2
	version_extra = version_split[1]
else
	version_extra = ''
endif
gkrellmv_h = configuration_data()
gkrellmv_h.set('GKRELLM_VERSION_MAJOR', version_components[0].to_int())
gkrellmv_h.set('GKRELLM_VERSION_MINOR', version_components[1].to_int())
gkrellmv_h.set('GKRELLM_VERSION_REV', version_components[2].to_int())
gkrellmv_h.set_quoted('GKRELLM_EXTRAVERSION', version_extra)
gkrellmv_h.set_quoted('GKRELLM_VERSION_STR', version_split[0])
gkrellmv_h.set_quoted('GKRELLM_FULL_VERSION_STR', full_version)

configure_file(
  input: 'gkrellmv.h.in',
  output: 'gkrellmv.h',
  configuration: gkrellmv_h,
)

if host_system == 'windows'


	resource_client_h = configuration_data()
	resource_client_h.set('version', version_split[0])
	resource_client_h.set('version_major', version_components[0].to_int())
	resource_client_h.set('version_minor', version_components[1].to_int())
	resource_client_h.set('version_rev', version_components[2].to_int())
	resource_client_h.set('description', 'GKrellM')
	resource_client_h.set('shortdesc', 'gkrellm')
	resource_client_h.set('icon', '#define IDI_ICON3 105\n\n//\n// Icon resources\n//\nLANGUAGE LANG_NEUTRAL, SUBLANG_NEUTRAL\nIDI_ICON3 ICON "../gkrellm.ico"\n')

	configure_file(
	input: 'win32-resource.rc.in',
	output: 'win32-resource-client.rc',
	configuration: resource_client_h,
	)

	resource_server_h = configuration_data()
	resource_server_h.set('version', version_split[0])
	resource_server_h.set('version_major', version_components[0].to_int())
	resource_server_h.set('version_minor', version_components[1].to_int())
	resource_server_h.set('version_rev', version_components[2].to_int())
	resource_server_h.set('description', 'GKrellM Daemon')
	resource_server_h.set('shortdesc', 'gkrellmd')
	resource_server_h.set('icon', '')

	configure_file(
	input: 'win32-resource.rc.in',
	output: 'win32-resource-server.rc',
	configuration: resource_server_h,
	)

endif
