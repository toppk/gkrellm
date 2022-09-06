
#ifndef THEME_H
#define THEME_H

#include <glib.h>

typedef struct GKTheme {
	gchar *name;
	GData *images;
} GKTheme;

GKTheme *gk_get_theme(void);

#endif