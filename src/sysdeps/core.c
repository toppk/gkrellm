// SPDX-License-Identifier: GPL-3.0-or-later WITH GPL-3.0-linking-source-exception

/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|
|  Author:  Bill Wilson    billw@gkrellm.net
|
|  Latest versions might be found at:  http://gkrellm.net
*/

#include "../gkrellm.h"
#include "../gkrellm-private.h"
#include "../gkrellm-sysdeps.h"

#if defined(USE_LIBGTOP)
#include "../sysdeps/gtop.c"
#endif

#if !defined(WIN32)
#include <sys/utsname.h>
#endif

/* needed for mbmon functions */
gboolean gkrellm_sys_sensors_mbmon_check(gboolean force);


gchar *
gkrellm_sys_get_host_name(void)
	{
	static gboolean	have_it;
	static gchar	buf[128];

	if (!have_it && gethostname(buf, sizeof(buf)) != 0)
		strcpy(buf, "unknown");
	have_it = TRUE;
	return buf;
	}

#if !defined(WIN32)
gchar *
gkrellm_sys_get_system_name(void)
	{
	static gchar	*sname;
	struct utsname	utsn;

	if (!sname && uname(&utsn) > -1)
		sname = g_strdup_printf("%s %s", utsn.sysname, utsn.release);
	if (!sname)
		sname = g_strdup("unknown name");
	return sname;
	}
#endif



gboolean
gkrellm_sys_sensors_mbmon_supported(void)
	{
#if !defined(WIN32) && defined(SENSORS_COMMON)
		/* TODO: does it matter if it has run or not? */
		return TRUE;
		/* return mbmon_check_func ? TRUE : FALSE; */
#else
	return FALSE;
#endif
	}

	/* Remove embedded "-i2c-" or "-isa-" from lm_sensors chip names so
|  there can be a chance for config name sysfs compatibility.  This function
|  here in sensors.c is a kludge.  Give user configs a chance to get
|  converted and then move this function to sysdeps/linux.c where it
|  belongs.
|  Munge names like w83627hf-isa-0290 to w83627hf-0290
|                or w83627hf-i2c-0-0290 to w83627hf-0-0290
*/
	void
	gkrellm_sensors_linux_name_fix(gchar *id_name)
	{
#if defined(__linux__)
		gchar *s;
		gint len, bus = 0;
		guint addr = 0;

		len = strlen(id_name) + 1;
		if ((s = strstr(id_name, "-i2c-")) != NULL)
		{
			sscanf(s + 5, "%d-%x", &bus, &addr);
			snprintf(s, len - (s - id_name), "-%d-%04x", bus, addr);
		}
		else if ((s = strstr(id_name, "-isa-")) != NULL)
		{
			*(s + 1) = '0';
			memmove(s + 2, s + 4, strlen(s + 4) + 1);
		}
#endif
	}
