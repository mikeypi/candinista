#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <gtk/gtk.h>

/*
 * show_editor_window — opens a modal transient window containing content.
 * relative_to must already be in the widget tree so gtk_widget_get_root()
 * can find the parent GtkWindow.
 */
void       show_editor_window (GtkWidget  *relative_to,
                                GtkWidget  *content,
                                const char *title);

/*
 * widget_for_legend — builds the main 3x2 grid of panel/sensor
 * menu buttons that forms the application's central widget.
 */
GtkWidget *widget_for_legend  (void);

#endif /* EDITOR_WINDOW_H */
