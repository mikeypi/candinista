#ifndef GTK_GLUE_H
#define GTK_GLUE_H

extern gboolean gtk_update_gauge_panel_value (gpointer user_data);
extern void gtk_draw_gauge_panel_cb (GtkDrawingArea*, cairo_t*, int, int, gpointer);
extern void on_activate_for_int (GtkEntry *entry, gpointer user_data);
extern void on_activate_for_double (GtkEntry *entry, gpointer user_data);
extern GtkWidget* new_label_for_string (const char* c);
extern GtkWidget* new_entry_for_int (gpointer user_data);
extern GtkWidget* new_entry_for_hex (gpointer user_data);
extern GtkWidget* new_entry_for_double (gpointer user_data);
extern GtkWidget* new_entry_for_string (gpointer user_data);
      
#endif
