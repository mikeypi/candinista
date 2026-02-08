#ifndef GTK_GLUE_H
#define GTK_GLUE_H

extern gboolean gtk_update_gauge_panel_value (gpointer user_data);
extern void gtk_draw_gauge_panel_cb (GtkDrawingArea*, cairo_t*, int, int, gpointer);

#endif
