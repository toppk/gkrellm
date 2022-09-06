


#include <glib.h>

#include "theme.h"

GKTheme *gk_get_theme(void)
{

	GKTheme *gk_theme = malloc(sizeof(GKTheme));
	g_datalist_init(&gk_theme->images);
	gk_theme->name = "default";
	g_datalist_set_data(&gk_theme->images, "foo", "bar");
	return gk_theme;
}