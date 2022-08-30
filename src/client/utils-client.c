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

/* ===============  GtkWidget utility functions ================= */

gchar *
gkrellm_gtk_entry_get_text(GtkWidget **entry)
	{
	static /*const*/ gchar *def_s = "";
	gchar *s = def_s;

	if (*entry)
		{
		s = (gchar *)gtk_entry_get_text(GTK_ENTRY(*entry));
		while (*s == ' ' || *s == '\t')
			++s;
		}
	return s;
	}


/* ===============  Miscellaneous utility functions ================= */

  /* Print a size, abbreviating it to kilo, mega, or giga units depending
  |  on its magnitude.
  |  An aside:  Memory capacities are traditionally reported in binary
  |  units (Kib, Mib, etc) while just about everything else should be
  |  reported in decimal units (KB, MB, etc).  This includes transfer
  |  rates, and disk capacities, contrary to what many people think.
  |  Take a look at http://www.pcguide.com/intro/fun/bindec.htm
  */
gint
gkrellm_format_size_abbrev(gchar *buf, size_t buflen, gfloat size,
		GkrellmSizeAbbrev *tbl, size_t tbl_size)
	{
	gfloat	abs_size;
	gint	i;
	int		ret;

	abs_size = (size < 0.0) ? -size : size;

	for (i = 0; i < tbl_size - 1; ++i)
		if (abs_size < tbl[i].limit)
			break;
	ret = snprintf(buf, buflen, tbl[i].format, size / tbl[i].divisor);
	if (ret < 0 || ret >= buflen)
		return 0;
	return ret;
	}


  /* Next three calls return string extent info.  Width extents are logical
  |  so that spaces will be counted while height extent is ink so that gkrellm
  |  can optimize vertical space utilization.
  */
gint
gkrellm_gdk_text_width(PangoFontDescription *font_desc,
				const gchar *string, gint len)
	{
	PangoLayout				*layout;
	gint					w, h;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_text(layout, string, len);
	pango_layout_get_pixel_size(layout, &w, &h);
	g_object_unref(layout);
	return w;
	}

gint
gkrellm_gdk_text_markup_width(PangoFontDescription *font_desc,
				const gchar *string, gint len)
	{
	PangoLayout				*layout;
	gint					w, h;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_markup(layout, string, len);
	pango_layout_get_pixel_size(layout, &w, &h);
	g_object_unref(layout);
	return w;
	}

gint
gkrellm_gdk_string_width(PangoFontDescription *font_desc, gchar *string)
	{
	PangoLayout	*layout;
	gint		w, h;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_text(layout, string, strlen(string));
	pango_layout_get_pixel_size(layout, &w, &h);
	g_object_unref(layout);
	return w;
	}

gint
gkrellm_gdk_string_markup_width(PangoFontDescription *font_desc, gchar *string)
	{
	PangoLayout	*layout;
	gint		w, h;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_markup(layout, string, strlen(string));
	pango_layout_get_pixel_size(layout, &w, &h);
	g_object_unref(layout);
	return w;
	}


void
gkrellm_text_extents(PangoFontDescription *font_desc, gchar *text,
		gint len, gint *width, gint *height, gint *baseline, gint *y_ink)
	{
	PangoLayout		*layout;
	PangoLayoutIter	*iter;
	PangoRectangle	ink, logical;
	gchar			*utf8;
	gint			base;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	if (g_utf8_validate(text, -1, NULL))
		pango_layout_set_text(layout, text, len);
	else
		{
		utf8 = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
		if (utf8)
			pango_layout_set_text(layout, utf8, len);
		g_free(utf8);
		}
	iter = pango_layout_get_iter(layout);
	base = pango_layout_iter_get_baseline(iter) / PANGO_SCALE;
	pango_layout_get_pixel_extents(layout, &ink, &logical);
	pango_layout_iter_free(iter);
	g_object_unref(layout);

	if (width)
		*width = logical.width;
	if (height)
		*height = ink.height;
	if (baseline)
		*baseline = base;
	if (y_ink)
		*y_ink = ink.y - logical.y;
	}

void
gkrellm_text_markup_extents(PangoFontDescription *font_desc, gchar *text,
		gint len, gint *width, gint *height, gint *baseline, gint *y_ink)
	{
	PangoLayout		*layout;
	PangoLayoutIter	*iter;
	PangoRectangle	ink, logical;
	gchar			*utf8;
	gint			base;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	if (g_utf8_validate(text, -1, NULL))
		pango_layout_set_markup(layout, text, len);
	else
		{
		utf8 = g_locale_to_utf8(text, -1, NULL, NULL, NULL);
		pango_layout_set_markup(layout, utf8, len);
		g_free(utf8);
		}
	iter = pango_layout_get_iter(layout);
	base = pango_layout_iter_get_baseline(iter) / PANGO_SCALE;
	pango_layout_get_pixel_extents(layout, &ink, &logical);
	pango_layout_iter_free(iter);
	g_object_unref(layout);

	if (width)
		*width = logical.width;
	if (height)
		*height = ink.height;
	if (baseline)
		*baseline = base;
	if (y_ink)
		*y_ink = ink.y - logical.y;
	}

void
gkrellm_gdk_draw_string(GdkDrawable *drawable, PangoFontDescription *font_desc,
			GdkGC *gc, gint x, gint y, gchar *string)
	{
	PangoLayout	*layout;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_text(layout, string, strlen(string));
	gdk_draw_layout(drawable, gc, x, y, layout);
	g_object_unref(layout);
	}

void
gkrellm_gdk_draw_string_markup(GdkDrawable *drawable,
			PangoFontDescription *font_desc,
			GdkGC *gc, gint x, gint y, gchar *string)
	{
	PangoLayout	*layout;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_markup(layout, string, strlen(string));
	gdk_draw_layout(drawable, gc, x, y, layout);
	g_object_unref(layout);
	}

void
gkrellm_gdk_draw_text(GdkDrawable *drawable, PangoFontDescription *font_desc,
			GdkGC *gc, gint x, gint y, gchar *string, gint len)
	{
	PangoLayout	*layout;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	if (g_utf8_validate(string, -1, NULL))
		pango_layout_set_text(layout, string, len);
	gdk_draw_layout(drawable, gc, x, y, layout);
	g_object_unref(layout);
	}

void
gkrellm_gdk_draw_text_markup(GdkDrawable *drawable,
			PangoFontDescription *font_desc,
			GdkGC *gc, gint x, gint y, gchar *string, gint len)
	{
	PangoLayout	*layout;

	layout = gtk_widget_create_pango_layout(gkrellm_get_top_window(), NULL);
	pango_layout_set_font_description(layout, font_desc);
	pango_layout_set_markup(layout, string, len);
	gdk_draw_layout(drawable, gc, x, y, layout);
	g_object_unref(layout);
	}

  /* Gtk config widgets work with utf8, so as long as I'm using gdk_draw
  |  functions, both utf8 and current locale versions of strings drawn on
  |  GKrellM must be maintained.  If src is not utf8, *dst is converted
  |  to utf8 and this should fix 1.2 -> 2.0 user_config conversions
  |  (This function will usually be called from config loading).
  |  dst_locale is piggy backing so when gdk_draw is replaced by Pango
  |  equivalents, usage of this function can be replaced with a simple
  |  gkrellm_dup_string().
  |  2.2.0 converts to using Pango.  Before replacing with gkrellm_dup_string,
  |  temporarily just treat dst_locale as a direct copy of dst_utf8.
  */
gboolean
gkrellm_locale_dup_string(gchar **dst_utf8, gchar *src, gchar **dst_locale)
	{
	if (!dst_utf8 || (!*dst_utf8 && !src))
		return FALSE;
	if (*dst_utf8)
		{
		if (src && !strcmp(*dst_utf8, src))
			return FALSE;
		g_free(*dst_utf8);
		g_free(*dst_locale);
		}
	if (src)
		{
		if (g_utf8_validate(src, -1, NULL))
			{
			*dst_utf8 = g_strdup(src);

			*dst_locale = g_strdup(src);
//			*dst_locale = g_locale_from_utf8(src, -1, NULL, NULL, NULL);
//			if (!*dst_locale)
//				*dst_locale = g_strdup(src);
			}
		else
			{
			*dst_utf8 = g_locale_to_utf8(src, -1, NULL, NULL, NULL);
			if (!*dst_utf8)
				*dst_utf8 = g_strdup(src);

			*dst_locale = g_strdup(*dst_utf8);
//			*dst_locale = g_strdup(src);
			}
		}
	else
		{
		*dst_utf8 = NULL;
		*dst_locale = NULL;
		}
	return TRUE;
	}

guint
big_endian_uint(guint8 *b)
	{
	return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	}
