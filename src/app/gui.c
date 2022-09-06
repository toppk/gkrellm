#include <gtk/gtk.h>

#include "theme.h"
#include "widgets.h"


#define APPLICATION_ID "net.gkrellm.app"

gboolean
draw_callback(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
		guint width, height;
		GdkRGBA color;
		GtkStyleContext *context;
		PangoLayout *layout;
		
		
		Monitor *monitor = (Monitor *)user_data;
		printf("in draw callback [%s]\n", monitor->name);
		context = gtk_widget_get_style_context(widget);

		width = gtk_widget_get_allocated_width(widget);
		height = gtk_widget_get_allocated_height(widget);

		gtk_render_background(context, cr, 0, 0, width, height);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_arc(cr,
				  width / 2.0, height / 2.0,
				  MIN(width, height) / 2.0,
				  0, 2 * G_PI);



		gtk_style_context_get_color(context,
									gtk_style_context_get_state(context),
									&color);
		gdk_cairo_set_source_rgba(cr, &color);
		cairo_set_source_rgba(cr, 0.7, 0.5, 0.3, 1.0);

		cairo_fill(cr);

		layout = pango_cairo_create_layout(cr);
		PangoFontDescription *desc = pango_font_description_from_string("Sans 20");
		pango_layout_set_font_description(layout, desc);
		pango_font_description_free(desc);

		pango_layout_set_single_paragraph_mode(layout, 1);
		cairo_set_source_rgb(cr, 0.1, 0.1, 0.7);

		pango_layout_set_text(layout, monitor->name, strlen(monitor->name));
		//pango_cairo_update_layout(cairo, layout);
		pango_cairo_show_layout(cr, layout);

		/*cairo_select_font_face(cr, "Sans",
								   CAIRO_FONT_SLANT_NORMAL,
								   CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_source_rgb(cr, 0.1, 0.1, 0.7);
		cairo_set_font_size(cr, 17);
		cairo_move_to(cr, 20, 30);
		cairo_show_text(cr, monitor->name); */

		return FALSE;
}

static void
activate(GtkApplication *app,
		 gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *drawing_area1, *drawing_area2;

	gchar *css = "\
window {\
	background-color : rgba(0, 255, 255, 0.0);\
}\
decoration {\
	border-radius : 6px 6px 0 0;\
	border-width : 0px;\
	box-shadow: 1px 12px 12px 12px rgba(0, 0, 0, 0.4), 0 0 0 1px rgba(0, 0, 0, 0.18);\
	margin: 4px;\
}\
decoration:backdrop {\
	border - radius : 6px 6px 0 0;\
	border - width : 0px;\
	box - shadow : 1px 12px 12px 12px rgba(0, 0, 0, 0.4), 0 0 0 1px rgba(0, 0, 0, 0.18);\
	margin:	4px;\
}\
";
	GtkCssProvider *css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(css_provider, css, -1, NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	window = gtk_application_window_new(app);
	//gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "Launcher");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
	GdkScreen *screen = gtk_window_get_screen(GTK_WINDOW(window));
	GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
	gtk_widget_set_visual(window, visual);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	Monitor *monitor1 = malloc(sizeof(Monitor));
	monitor1->name = "first";
	monitor1->value = 3;
	Monitor *monitor2 = malloc(sizeof(Monitor));
	monitor2->name = "second";
	monitor2->value = 3;

	drawing_area1 = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area1, 100, 100);
	g_signal_connect(G_OBJECT(drawing_area1), "draw",
					 G_CALLBACK(draw_callback), (gpointer) monitor1);
	gtk_box_pack_start(GTK_BOX(vbox), drawing_area1, TRUE, TRUE, 0);

	drawing_area2 = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area2, 100, 100);
	g_signal_connect(G_OBJECT(drawing_area2), "draw",
					 G_CALLBACK(draw_callback), (gpointer)monitor2);
	gtk_box_pack_start(GTK_BOX(vbox), drawing_area2, TRUE, TRUE, 0);

	button = gtk_button_new_with_label("Quit");
	gtk_widget_set_margin_top(button, 50);
	gtk_widget_set_margin_bottom(button, 50);

	g_signal_connect_swapped(button, "clicked", G_CALLBACK(gtk_widget_destroy), window);
	gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

	gtk_widget_show_all(window);
}


int gk_start_ui()
{
	GtkApplication *app;
	int status;

	app = gtk_application_new(APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
	g_set_prgname(APPLICATION_ID);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);

	return status;
}