#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "panel.h"

gboolean
gtk_update_panel_value (gpointer user_data) {
    struct
    {
        GtkDrawingArea* area;
        Panel* p;
        int timeout_id;
    } *ctx = user_data;

    gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));
    return G_SOURCE_CONTINUE;
}

