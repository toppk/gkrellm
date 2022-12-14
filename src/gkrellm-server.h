// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|
|  Author:  Bill Wilson    billw@gkrellm.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#include "shared/log.h"

#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>

#if !defined(WIN32)
	#include <unistd.h>
	#include <utime.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <sys/ioctl.h>
	#include <pwd.h>
	#include <grp.h>
	#if defined(__solaris__)
		#include <sys/filio.h>
	#endif /* defined(__solaris__) */
	#include <sys/select.h>
	#include <sys/wait.h>
#else
	#include <winsock2.h>
	#include <ws2tcpip.h>
	typedef int sa_family_t; // WIN32 uses int for ai_family;
	#include <stdint.h> // defines uint32_t
#endif /* !defined(WIN32) */

#include <sys/stat.h>
#include <sys/types.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>

/* -------------------------------------------------------------------
*/

#define GKRELLMD_CONFIG				"gkrellmd.conf"
#if defined(WIN32)
	// no dot in front of config-filename on win32
	#define GKRELLMD_USER_CONFIG  GKRELLMD_CONFIG
#else
	#define GKRELLMD_USER_CONFIG	".gkrellmd.conf"
#endif

#define GKRELLMD_PLUGINS_DIR		".gkrellm2/plugins-gkrellmd"
#if !defined(WIN32)
	#define GKRELLMD_LOCAL_PLUGINS_DIR	"/usr/local/lib/gkrellm2/plugins-gkrellmd"
	#if !defined(GKRELLMD_SYSTEM_PLUGINS_DIR)
		#define GKRELLMD_SYSTEM_PLUGINS_DIR	"/usr/lib/gkrellm2/plugins-gkrellmd"
	#endif
	#define GKRELLMD_SYS_ETC	"/etc"
	#define GKRELLMD_LOCAL_ETC	"/usr/local/etc"
#endif // !defined(WIN32)


typedef struct _GkrellmdClient
	{
	gint		major_version,
				minor_version,
				rev_version;
	gchar		*hostname;

	gint		fd;
	gboolean	served,
				alive,
				last_client;
	gboolean	feature_subdisk;
	GString		*input_gstring;
	void		(*input_func)(struct _GkrellmdClient *, gchar *);
	}
	GkrellmdClient;


typedef struct
	{
	gint	timer_ticks,
			second_tick,
			two_second_tick,
			five_second_tick,
			ten_second_tick,
			minute_tick;
	}
	GkrellmdTicks;

extern GkrellmdTicks			GK;


typedef struct
	{
	gboolean		need_serve;
	const gchar		*serve_name;
	gboolean		serve_name_sent;
	GString			*serve_gstring;
	GkrellmdClient	*client;

	GList			*config_list;

	gboolean		is_plugin;
	void			*handle;
	gchar			*path;
	void			(*client_input_func)(GkrellmdClient *, gchar *);
	}
	GkrellmdMonitorPrivate;


typedef struct _GkrellmdMonitor
	{
	gchar		*name;
	void		(*update_monitor)(struct _GkrellmdMonitor *mon,
							gboolean first_update);
	void		(*serve_data)(struct _GkrellmdMonitor *mon,
							gboolean first_serve);
	void		(*serve_setup)(struct _GkrellmdMonitor *mon);

	GkrellmdMonitorPrivate
				*privat;
	}
	GkrellmdMonitor;



  /* gkrellmd serve data functions used by builtins and plugins.
  */
void		gkrellmd_plugin_serve_setup(GkrellmdMonitor *mon,
						gchar *name, gchar *line);
void		gkrellmd_need_serve(GkrellmdMonitor *mon);
void		gkrellmd_set_serve_name(GkrellmdMonitor *mon, const gchar *name);
void		gkrellmd_serve_data(GkrellmdMonitor *mon, gchar *line);
void		gkrellmd_add_serveflag_done(gboolean *);
gboolean	gkrellmd_check_client_version(GkrellmdMonitor *mon,
						gint major, gint minor, gint rev);

const gchar	*gkrellmd_config_getline(GkrellmdMonitor *mon);

void		gkrellmd_client_input_connect(GkrellmdMonitor *mon,
						void (*func)(GkrellmdClient *, gchar *));


  /* Small set of useful functions duplicated from src/utils.c.
  |  These really should just be in the gkrellm_ namespace for sysdep code
  |  common to gkrellm and gkrellmd, but for convenience, offer them in
  |  both gkrellm_ and gkrellmd_ namespaces.
  */
void		gkrellmd_free_glist_and_data(GList **list_head);
gboolean	gkrellmd_getline_from_gstring(GString **, gchar *, gint);
gchar		*gkrellmd_dup_token(gchar **string, gchar *delimeters);
gboolean	gkrellmd_dup_string(gchar **dst, gchar *src);

void		gkrellm_free_glist_and_data(GList **list_head);
gboolean	gkrellm_getline_from_gstring(GString **, gchar *, gint);
gchar		*gkrellm_dup_token(gchar **string, gchar *delimeters);
gboolean	gkrellm_dup_string(gchar **dst, gchar *src);


  /* Plugins should use above data serve functions instead of this.
  */
gint		gkrellmd_send_to_client(GkrellmdClient *client, gchar *buf);


  /* Misc
  */
void		gkrellmd_add_mailbox(gchar *);
GkrellmdTicks *gkrellmd_ticks(void);
gint		gkrellmd_get_timer_ticks(void);
