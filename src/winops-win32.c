/* GKrellM
|  Copyright (C) 1999-2019 Bill Wilson
|                2007-2019 Stefan Gehn
|
|  Authors:  Bill Wilson    billw@gkrellm.net
|            Stefan Gehn    stefan+gkrellm@srcbox.net
|  Latest versions might be found at:  http://gkrellm.net
|
|
|  GKrellM is free software: you can redistribute it and/or modify it
|  under the terms of the GNU General Public License as published by
|  the Free Software Foundation, either version 3 of the License, or
|  (at your option) any later version.
|
|  GKrellM is distributed in the hope that it will be useful, but WITHOUT
|  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
|  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
|  License for more details.
|
|  You should have received a copy of the GNU General Public License
|  along with this program. If not, see http://www.gnu.org/licenses/
|
|
|  Additional permission under GNU GPL version 3 section 7
|
|  If you modify this program, or any covered work, by linking or
|  combining it with the OpenSSL project's OpenSSL library (or a
|  modified version of that library), containing parts covered by
|  the terms of the OpenSSL or SSLeay licenses, you are granted
|  additional permission to convey the resulting work.
|  Corresponding Source for a non-source form of such a combination
|  shall include the source code for the parts of OpenSSL used as well
|  as that of the covered work.
*/

#include "gkrellm.h"
#include "gkrellm-private.h"
#include <gdk/gdkwin32.h>

static GdkGC	*trans_gc  = NULL;
static GdkColor trans_color;


void
gkrellm_winop_reset(void)
{
}

void
gkrellm_winop_options(gint argc, gchar **argv)
	{
	// This essentially hides the taskbar entry on win32, unfortunately
	// gtk_window_set_skip_taskbar_hint() does not have any effect
	gtk_window_set_type_hint(GTK_WINDOW(gkrellm_get_top_window()), GDK_WINDOW_TYPE_HINT_UTILITY);

	gkrellm_winop_state_above(_GK.on_top);
	}

void
gkrellm_winop_withdrawn(void)
	{
	}

void
gkrellm_winop_place_gkrellm(gchar *geom)
	{
	gint newX = _GK.x_position;
	gint newY = _GK.y_position;

	// parse the command line
	// +x+y or -x+y or +x-y or -x-y

	gint startx = -1, starty = -1, endx = -1, endy = -1;
	gint w_gkrell, h_gkrell;
	gint stringSize = strlen(geom);
	gint i;
	gint xsign = 1, ysign = 1;
	gchar* part;

	for (i = 0; i < stringSize; i++)
		{
		if (geom[i] != '+' && geom[i] != '-')
			{
			if (startx == -1)
				startx = i;
			if (starty == -1 && endx != -1)
				starty = i;
			}
		else
			{
			if (startx != -1 && endx == -1)
				{
				endx = i - 1;
				if (geom[i] == '-')
					xsign = -1;
				}
			if (starty != -1 && endy == -1)
				{
				endy = i - 1;
				if (geom[i] == '-')
					ysign = -1;
				}
			}
		}

	if (starty != -1 && endy == -1)
		endy = stringSize - 1;

	w_gkrell = _GK.chart_width + _GK.frame_left_width + _GK.frame_right_width;
	h_gkrell = _GK.monitor_height + _GK.total_frame_height;

	if (startx >= 0 && startx <= endx && endx >= 0)
		{
		part = malloc(sizeof(gchar) * (endx - startx + 1 + 1));
		for (i = 0; i < endx - startx + 1; i++)
			part[i] = geom[i + startx];
		part[i] = '\0';
		newX = atoi(part);
		if (xsign == -1)
			newX = _GK.w_display - w_gkrell + newX;
		free(part);
		}

	if (starty >= 0 && starty <= endy && endy >= 0)
		{
		part = malloc(sizeof(gchar) * (endy - starty + 1 + 1));
		for (i = 0; i < endy - starty + 1; i++)
			part[i] = geom[i + starty];
		part[i] = '\0';
		newY = atoi(part);
		if (ysign == -1)
			newY = _GK.h_display - h_gkrell + newY;
		free(part);
		}

	if (   newX >= 0 && newX < _GK.w_display - 10
	    && newY >= 0 && newY < _GK.h_display - 25)
		{
		gdk_window_move(gtk_widget_get_window(gkrellm_get_top_window()), newX, newY);
		_GK.y_position = newY;
		_GK.x_position = newX;
		}

	_GK.position_valid = TRUE;
	}

void
gkrellm_winop_flush_motion_events(void)
	{
	}

gboolean
gkrellm_winop_updated_background(void)
	{
	return TRUE;
	}

void
gkrellm_winop_update_struts(void)
	{
	}

gboolean
gkrellm_winop_draw_rootpixmap_onto_transparent_chart(GkrellmChart *p)
	{
		if (!p->transparency || !p->drawing_area || !gtk_widget_get_window(p->drawing_area) || !trans_gc)
			return FALSE;

		// Fill the panel with transparency color
		gdk_draw_rectangle(p->bg_src_pixmap, trans_gc, TRUE, 0, 0, p->w, p->h);

		// If mode permits, stencil on non transparent parts of bg_clean_pixmap.
		if (p->transparency == 2 && p->bg_mask)
		{
		gdk_gc_set_clip_mask(_GK.text_GC, p->bg_mask);
		gdk_draw_drawable(p->bg_src_pixmap, _GK.text_GC, p->bg_clean_pixmap,
			0, 0, 0, 0, p->w, p->h);
		}

	gdk_gc_set_clip_mask(_GK.text_GC, NULL);
	p->bg_sequence_id += 1;
	return TRUE;
	}

gboolean
gkrellm_winop_draw_rootpixmap_onto_transparent_panel(GkrellmPanel *p)
	{
		if (!p->transparency || !p->drawing_area || !gtk_widget_get_window(p->drawing_area) || !trans_gc)
			return FALSE;

		gdk_gc_set_fill(trans_gc, GDK_SOLID);
		gdk_gc_set_foreground(trans_gc, &trans_color);

		// Fill the panel with transparency color
		gdk_draw_rectangle(p->bg_pixmap, trans_gc, TRUE, 0, 0, p->w, p->h);

		// If mode permits, stencil on non transparent parts of bg_clean_pixmap.
		if (p->transparency == 2 && p->bg_mask)
		{
		gdk_gc_set_clip_mask(_GK.text_GC, p->bg_mask);
		gdk_draw_drawable(p->bg_pixmap, _GK.text_GC, p->bg_clean_pixmap,
			0, 0, 0, 0, p->w, p->h);
		gdk_gc_set_clip_mask(_GK.text_GC, NULL);
		}
	return TRUE;
	}

static void
draw_rootpixmap_onto_transparent_spacers(GkrellmMonitor *mon)
{
	GkrellmMonprivate	*mp = mon->privat;

	if (mp->top_spacer.image)
	{
		// Fill the panel with transparency color
		gdk_draw_rectangle(mp->top_spacer.pixmap, trans_gc, TRUE, 0, 0,
			_GK.chart_width, mp->top_spacer.height);

		if (mp->top_spacer.mask)
		{
			gdk_gc_set_clip_mask(_GK.text_GC, mp->top_spacer.mask);
			gdk_draw_drawable(mp->top_spacer.pixmap, _GK.text_GC,
				mp->top_spacer.clean_pixmap, 0, 0, 0, 0, _GK.chart_width,
				mp->top_spacer.height);
		}

		gtk_image_set_from_pixmap(GTK_IMAGE(mp->top_spacer.image),
			mp->top_spacer.pixmap, NULL);
	}

	if (mp->bottom_spacer.image)
	{
		// Fill the panel with transparency color
		gdk_draw_rectangle(mp->bottom_spacer.pixmap, trans_gc, TRUE, 0, 0,
			_GK.chart_width, mp->bottom_spacer.height);

		if (mp->bottom_spacer.mask)
		{
			gdk_gc_set_clip_mask(_GK.text_GC, mp->bottom_spacer.mask);
			gdk_draw_drawable(mp->bottom_spacer.pixmap, _GK.text_GC,
				mp->bottom_spacer.clean_pixmap, 0, 0, 0, 0, _GK.chart_width,
				mp->bottom_spacer.height);
		}

		gtk_image_set_from_pixmap(GTK_IMAGE(mp->bottom_spacer.image),
			mp->bottom_spacer.pixmap, NULL);
	}
}

void
gkrellm_winop_apply_rootpixmap_transparency(void)
	{
	static gboolean isTransparent = FALSE;

	GList			*list;
	GkrellmChart	*cp;
	GkrellmPanel	*p;
	HWND			w;

	w = GDK_WINDOW_HWND(gtk_widget_get_window(gkrellm_get_top_window()));
	if (!_GK.any_transparency && isTransparent)
		{	// make opaque
		SetWindowLong(w, GWL_EXSTYLE, GetWindowLong(w, GWL_EXSTYLE) & ~WS_EX_LAYERED);
		isTransparent = FALSE;
		return;
		}
	else if (_GK.any_transparency && !isTransparent)
		{	// make transparent
		if (trans_gc == NULL)
			{
			GdkColormap *cm = gtk_widget_get_colormap(gkrellm_get_top_window());

			trans_gc = gdk_gc_new(gtk_widget_get_window(gkrellm_get_top_window()));
			if (trans_gc == NULL)
				{
				g_warning("Could not create trans_gc!\n");
				return;
				}

			gdk_gc_copy(trans_gc, _GK.draw1_GC);
			gdk_gc_set_fill(trans_gc, GDK_SOLID);

			trans_color.red   = 65535;
			trans_color.green = 0;
			trans_color.blue  = 65535;

			if (!gdk_colormap_alloc_color(cm , &trans_color, FALSE, TRUE))
				{
				g_warning("Could not allocate trans_color!\n");
				g_object_unref(trans_gc);
				trans_gc = NULL;
				return;
				}

			gdk_gc_set_foreground(trans_gc, &trans_color);
			}
		SetWindowLong(w, GWL_EXSTYLE, GetWindowLong(w, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(w, RGB(255, 0, 255), 0, LWA_COLORKEY);
		isTransparent = TRUE;
		}

	if (isTransparent)
		{
		for (list = gkrellm_get_chart_list(); list; list = list->next)
			{
			cp = (GkrellmChart *) list->data;
			if (!cp->transparency || !cp->shown)
				continue;
			gkrellm_winop_draw_rootpixmap_onto_transparent_chart(cp);
			gkrellm_refresh_chart(cp);
			}

		for (list = gkrellm_get_panel_list(); list; list = list->next)
			{
			p = (GkrellmPanel *) list->data;
			if (!p->transparency || !p->shown)
				continue;
			gkrellm_draw_panel_label(p);
			}

		for (list = gkrellm_monitor_list; list; list = list->next)
			{
			draw_rootpixmap_onto_transparent_spacers((GkrellmMonitor *)list->data);
			}

		gdk_gc_set_clip_mask(_GK.text_GC, NULL);
		}
	}

void
gkrellm_winop_state_skip_taskbar(gboolean state)
	{
	}

void
gkrellm_winop_state_skip_pager(gboolean state)
	{
	}

void
gkrellm_winop_state_above(gboolean state)
	{
	GtkWindow *window = GTK_WINDOW(gkrellm_get_top_window());
	gtk_window_set_keep_above(window, state);
	}

void
gkrellm_winop_state_below(gboolean state)
	{
	}
