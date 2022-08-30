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

#include <errno.h>
#if !defined(F_TLOCK)
#include <sys/file.h>
#endif

#include <pwd.h>
#include <sys/types.h>


static FILE		*f_lock;

static gboolean
_gkrellm_get_lock(void)
	{
	gchar	*lock_dir, *lock_file; //, *display, *s;
	gchar	buf[32];

	snprintf(buf, sizeof(buf), "LCK..%d", (gint) getuid());

#if defined(F_TLOCK)
	lock_dir = "/var/lock/gkrellm";
	if (!g_file_test(lock_dir, G_FILE_TEST_IS_DIR))
		mkdir(lock_dir, 0755);

	lock_file = gkrellm_make_config_file_name(lock_dir, buf);
	/*display = XDisplayName(NULL);
	if (display)
		{
		s = g_strconcat(lock_file, "_", display, NULL);
		g_free(lock_file);
		lock_file = s;
		}*/
	f_lock = fopen(lock_file, "w+");	/* buffering does not apply here */
	g_free(lock_file);
	if (   f_lock
		&& lockf(fileno(f_lock), F_TLOCK, 0) != 0
		&& errno == EAGAIN
	   )
		return FALSE;
	if (f_lock)
		{
		fprintf(f_lock, "%10d\n", (gint) getpid());
		fflush(f_lock); 
		}
#endif
	return TRUE;
	}




void
gkrellm_winop_reset(void)
	{
	}

void
gkrellm_winop_state_skip_taskbar(gboolean state)
	{
	gdk_window_set_skip_taskbar_hint(gtk_widget_get_window(gkrellm_get_top_window()), state);
	}

void
gkrellm_winop_state_skip_pager(gboolean state)
	{
	gdk_window_set_skip_pager_hint(gtk_widget_get_window(gkrellm_get_top_window()), state);
	}

void
gkrellm_winop_state_above(gboolean state)
	{
	gdk_window_set_keep_above(gtk_widget_get_window(gkrellm_get_top_window()), state);
	}

void
gkrellm_winop_state_below(gboolean state)
	{
	gdk_window_set_keep_below(gtk_widget_get_window(gkrellm_get_top_window()), state);
	}

void
gkrellm_winop_update_struts(void)
	{
	}

void
gkrellm_winop_options(gint argc, gchar **argv)
	{
	gint		n = 0;

	if (   !_GK.allow_multiple_instances_real
		&& !_GK.force_host_config
		&& !_gkrellm_get_lock()
	   )
		{
		g_message("gkrellm: %s\n",
			_("Exiting because multiple instances option is off.\n"));
		exit(0);
		}
	} 

void
gkrellm_winop_withdrawn(void)
	{
	}

  /* Use XParseGeometry, but width and height are ignored.
  |  If GKrellM is moved, update _GK.y_position.
  */
void
gkrellm_winop_place_gkrellm(gchar *geom)
    {
	/*gint	place, x, y, w_gkrell, h_gkrell;

	x = y = 0;
	place = XParseGeometry(geom, &x, &y,
				(guint *) &w_gkrell, (guint *) &h_gkrell);

	w_gkrell = _GK.chart_width + _GK.frame_left_width + _GK.frame_right_width;
	h_gkrell = _GK.monitor_height + _GK.total_frame_height;

	if (place & YNegative)
		y = _GK.h_display - h_gkrell + y;
	if (place & XNegative)
		x = _GK.w_display - w_gkrell + x;
	gdk_window_move(gtk_widget_get_window(gkrellm_get_top_window()), x, y);
	_GK.y_position = y;
	_GK.x_position = x;
	_GK.position_valid = TRUE;
	gkrellm_debug(DEBUG_POSITION, "geometry moveto %d %d\n", x, y);
	*/
	}

void
gkrellm_winop_flush_motion_events(void)
	{
	}

  /* Check if background has changed
  */
gboolean
gkrellm_winop_updated_background(void)
	{
	return TRUE;
	}

gboolean
gkrellm_winop_draw_rootpixmap_onto_transparent_chart(GkrellmChart *cp)
	{
	return FALSE;
	}	

gboolean
gkrellm_winop_draw_rootpixmap_onto_transparent_panel(GkrellmPanel *p)
	{
	return FALSE;
	}	

void
gkrellm_winop_apply_rootpixmap_transparency(void)
	{
	}
