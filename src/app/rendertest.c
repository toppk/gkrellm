
/*

 src/client/main

  create_widget_tree();

  load_builtin_monitors();
    add_builtin(gkrellm_init_cpu_monitor());
  gkrellm_plugins_load();

  gkrellm_build();
  gkrellm_make_themes_list();

  // signal_connect

  ui_manager = gkrellm_create_ui_manager_popup();

  gkrellm_winop_options(argc, argv);
  gtk_widget_show(gtree.window);
  gkrellm_winop_withdrawn();


  gkrellm_start_timer(_GK.update_HZ);
  setup_signal_handler();
  gtk_main();


 */

#include "../gkrellm.h"
#include "../gkrellm-private.h"
#include "../gkrellm-sysdeps.h"
#include "../shared/log-private.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xmd.h>
#include <X11/SM/SMlib.h>
#include <X11/Xatom.h>

#include "rendertypes.h"

static GtkWidget *top_window;

GtkWidget *
gkrellm_get_top_window(void)
{
	return top_window;
}

void gkrellm_record_state(enum GkrellmState state, GkrellmMonitor *mon)
{
	_GK.gkrellm_state = state;
	_GK.active_monitor = mon;
}

struct GkrellmConfig _GK;
GkrellmTicks GK;
GList *gkrellm_monitor_list;
time_t gkrellm_time_now;

static GtkUIManager *ui_manager;

static gchar *geometry;

static gint y_pack;

static gint monitor_previous_height;
static gint monitors_visible = TRUE;
static gint mask_monitors_visible = TRUE;
static gint check_rootpixmap_transparency;
static gboolean no_transparency,
	do_intro,
	decorated,
	configure_position_lock;

static void apply_frame_transparency(gboolean force);


int main(int argc, char *argv[]) {

// main

// create_widget_tree
GtkWidget *window;

gtk_init(&argc, &argv);

window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
gtk_widget_show(window);

g_signal_connect(window, "destroy",
                 G_CALLBACK(gtk_main_quit), NULL);

GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(window), vbox);

GtkWidget *top0_event_box = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(vbox), top0_event_box);
GtkWidget *top0_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(top0_event_box), top0_vbox);

/* The middle hbox has left frame, monitors & a right frame.
 */
GtkWidget *middle_hbox = gtk_hbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(vbox), middle_hbox);

GtkWidget *left_event_box = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(middle_hbox), left_event_box);
GtkWidget *left_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(left_event_box), left_vbox);

GtkWidget *middle_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(middle_hbox), middle_vbox);

/* Hostname will go in an event box for moving gkrellm */
GtkWidget *top1_event_box = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(middle_vbox), top1_event_box);
GtkWidget *top1_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(top1_event_box), top1_vbox);

GtkWidget *monitor_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(middle_vbox), monitor_vbox);

GtkWidget *right_event_box = gtk_event_box_new();
gtk_container_add(GTK_CONTAINER(middle_hbox), right_event_box);
GtkWidget *right_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(right_event_box), right_vbox);

GtkWidget *bottom_vbox = gtk_vbox_new(FALSE, 0);
gtk_container_add(GTK_CONTAINER(vbox), bottom_vbox);

gtk_widget_realize(window);

/* Set the toplevel window size handling to be under program control.
 */
gtk_window_set_resizable((GtkWindow *)window, FALSE);

gtk_window_set_decorated((GtkWindow *)window, FALSE);

gtk_widget_show_all(vbox);

/* Probably don't need to realize all these here. Just a little paranoia.
 */
gtk_widget_realize(vbox);
gtk_widget_realize(top0_vbox);
gtk_widget_realize(middle_hbox);
gtk_widget_realize(left_vbox);
gtk_widget_realize(middle_vbox);
gtk_widget_realize(monitor_vbox);
gtk_widget_realize(top1_vbox);
gtk_widget_realize(right_vbox);
gtk_widget_realize(bottom_vbox);

//  load_builtin_monitors();
//    add_builtin(gkrellm_init_cpu_monitor());
//  GkrellmMonitor -> GkrellmMonprivate -> GkrellmSpacer
GtkWidget *cpu_main_vbox = gtk_vbox_new(FALSE, 0);
GtkWidget *cpu_top_spacer_vbox = gtk_vbox_new(FALSE, 0);
GtkWidget *cpu_box = gtk_vbox_new(FALSE, 0);
GtkWidget *cpu_bottom_spacer_vbox = gtk_vbox_new(FALSE, 0);

//  gkrellm_plugins_load();

//  gkrellm_build();
//     	gkrellm_winop_reset();
//     gkrellm_start_timer(0);
Pixmap root_xpixmap = None;
/* 	if (!first_create)
    edge_record();
  gkrellm_alert_reset_all();
  gkrellm_panel_cleanup();

  gkrellm_theme_config();
    gkrellm_load_theme_config();
    gkrellm_init_theme();

    if (first_create)
      gkrellm_load_user_config(NULL, TRUE);
  setup_colors();

*/
GdkGC *draw1_GC,
    *draw2_GC,
    *draw3_GC,
    *draw_stencil_GC,
    *text_GC;

GdkGC *bit1_GC, /* Depth 1 GCs		*/
    *bit0_GC;

draw1_GC = gdk_gc_new(gtk_widget_get_window(window));
gdk_gc_copy(draw1_GC, gtk_widget_get_style(window)->white_gc);
draw2_GC = gdk_gc_new(gtk_widget_get_window(window));
gdk_gc_copy(draw2_GC, gtk_widget_get_style(window)->white_gc);
draw3_GC = gdk_gc_new(gtk_widget_get_window(window));
gdk_gc_copy(draw3_GC, gtk_widget_get_style(window)->white_gc);
draw_stencil_GC = gdk_gc_new(gtk_widget_get_window(window));
gdk_gc_copy(draw_stencil_GC, gtk_widget_get_style(window)->white_gc);
text_GC = gdk_gc_new(gtk_widget_get_window(window));
gdk_gc_copy(text_GC, gtk_widget_get_style(window)->white_gc);

GdkBitmap *dummy_bitmap;
GdkColor bit_color;

dummy_bitmap = gdk_pixmap_new(gtk_widget_get_window(window), 16, 16, 1);
bit1_GC = gdk_gc_new(dummy_bitmap);
bit0_GC = gdk_gc_new(dummy_bitmap);
bit_color.pixel = 1;
gdk_gc_set_foreground(bit1_GC, &bit_color);
bit_color.pixel = 0;
gdk_gc_set_foreground(bit0_GC, &bit_color);
g_object_unref(G_OBJECT(dummy_bitmap));

/*
  setup_fonts();
    load_font(_GK.large_font_string, &_GK.large_font, fail_large_font);
    load_font(_GK.normal_font_string, &_GK.normal_font, fail_normal_font);
    load_font(_GK.small_font_string, &_GK.small_font, fail_small_font);

  */
PangoFontDescription *large_font = pango_font_description_from_string("Sans 11");
PangoFontDescription *normal_font = pango_font_description_from_string("Sans 9");
PangoFontDescription *small_font = pango_font_description_from_string("Sans 8");

/*
  gkrellm_load_theme_piximages();

*/
gkrellm_load_theme_piximages();

/*

  gkrellm_chart_setup();
  gkrellm_freeze_side_frame_packing();
*/
//  gkrellm_make_themes_list();

// lots of g_signal_connect...

//  ui_manager = gkrellm_create_ui_manager_popup();

//  gkrellm_winop_options(argc, argv);
gtk_widget_show(window);
//  gkrellm_winop_withdrawn();


//gkrellm_start_timer(_GK.update_HZ);
// setup_signal_handler();
  gtk_main();


return 0;
}
