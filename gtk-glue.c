#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"

gboolean
gtk_update_gauge_panel_value (gpointer user_data) {
  struct
  {
    GtkDrawingArea *area;
    Panel *p;
  } *ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}


void
gtk_draw_gauge_panel_cb (GtkDrawingArea* area,
                  cairo_t*        cr,
                  int             width,
                  int             height,
                  gpointer        user_data)
{
  Panel* p = user_data;

  g_return_if_fail (p != NULL);
  g_return_if_fail (p -> draw != NULL);

  p -> draw (area, cr, width, height, p);
}

