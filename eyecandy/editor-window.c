/*
 * Copyright (c) 2024, Joseph Hollinger
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"
#include "sensor.h"
#include "d3-array.h"
#include "yaml-loader.h"
#include "gtk-glue.h"
#include "panel-editor.h"
#include "sensor-editor.h"
#include "editor-window.h"

/* ---------------------------------------------------------------
 * Editor window
 * ------------------------------------------------------------- */

void
show_editor_window (GtkWidget  *relative_to,
                    GtkWidget  *content,
                    const char *title)
{
  GtkWidget *root = GTK_WIDGET (gtk_widget_get_root (relative_to));
  if (!GTK_IS_WINDOW (root)) return;

  GtkWidget *w = gtk_window_new ();
  gtk_window_set_transient_for (GTK_WINDOW (w), GTK_WINDOW (root));
  gtk_window_set_modal         (GTK_WINDOW (w), TRUE);
  gtk_window_set_title         (GTK_WINDOW (w), title);
  gtk_window_set_default_size  (GTK_WINDOW (w), 760, 520);
  gtk_window_set_child         (GTK_WINDOW (w), content);
  gtk_window_present           (GTK_WINDOW (w));
}

/* ---------------------------------------------------------------
 * Legend grid — panel and sensor menu buttons
 * ------------------------------------------------------------- */

static void
callback_for_panel_button (GtkWidget *button, gpointer user_data)
{
  show_editor_window (button, widget_for_panel ((Panel *) user_data), "Panel");
}

static void
callback_for_sensor_button (GtkWidget *button, gpointer user_data)
{
  show_editor_window (button, widget_for_sensor ((Sensor *) user_data), "Sensor");
}

/* Creates a GtkMenuButton with a scrollable popover box.
   The inner GtkBox is stored on the button under the key "box". */
static GtkWidget *
menu_button_with_popover (const char *label)
{
  GtkWidget *mb       = gtk_menu_button_new ();
  GtkWidget *popover  = gtk_popover_new ();
  GtkWidget *scroller = gtk_scrolled_window_new ();
  GtkWidget *box      = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);

  gtk_menu_button_set_label (GTK_MENU_BUTTON (mb), label);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (scroller, -1, 400);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), box);
  gtk_popover_set_child (GTK_POPOVER (popover), scroller);
  gtk_menu_button_set_popover (GTK_MENU_BUTTON (mb), popover);
  g_object_set_data (G_OBJECT (mb), "box", box);
  return mb;
}

static GtkWidget *
widget_for_panel_group_dropdown (int i, int j)
{
  int count = 0;
  for (int k = 0; k < cfg->panel_count; k++) {
    Panel *p = cfg->panels[k];
    if (panel_get_x_index (p) == i && panel_get_y_index (p) == j) count++;
  }

  char label[80];
  snprintf (label, sizeof (label), "panels (%d)", count);
  GtkWidget *mb  = menu_button_with_popover (label);
  GtkWidget *box = g_object_get_data (G_OBJECT (mb), "box");

  for (int k = 0; k < cfg->panel_count; k++) {
    Panel *p = cfg->panels[k];
    if (panel_get_x_index (p) == i && panel_get_y_index (p) == j) {
      GtkWidget *btn = gtk_button_new_with_label (p->name);
      g_signal_connect (btn, "clicked", G_CALLBACK (callback_for_panel_button), p);
      gtk_box_append (GTK_BOX (box), btn);
    }
  }
  return mb;
}

static GtkWidget *
widget_for_sensor_group_dropdown (int i, int j)
{
  int count = 0;
  for (int k = 0; k < cfg->sensor_count; k++) {
    Sensor *s = cfg->sensors[k];
    if (sensor_get_x_index (s) == i && sensor_get_y_index (s) == j) count++;
  }

  char label[80];
  snprintf (label, sizeof (label), "sensors (%d)", count);
  GtkWidget *mb  = menu_button_with_popover (label);
  GtkWidget *box = g_object_get_data (G_OBJECT (mb), "box");

  for (int k = 0; k < cfg->sensor_count; k++) {
    Sensor *s = cfg->sensors[k];
    if (sensor_get_x_index (s) == i && sensor_get_y_index (s) == j) {
      GtkWidget *btn = gtk_button_new_with_label (s->name);
      g_signal_connect (btn, "clicked", G_CALLBACK (callback_for_sensor_button), s);
      gtk_box_append (GTK_BOX (box), btn);
    }
  }
  return mb;
}

GtkWidget *
widget_for_legend (void)
{
  static const char *box_labels[3][2] = {
    { "upper left",   "lower left"   },
    { "upper middle", "lower middle" },
    { "upper right",  "lower right"  }
  };

  GtkGrid *grid = GTK_GRID (gtk_grid_new ());
  for (int i = 0; i < cfg->x_dimension; i++) {
    for (int j = 0; j < cfg->y_dimension; j++) {
      GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
      gtk_box_append (GTK_BOX (box), new_label_for_string (box_labels[i][j]));
      gtk_box_append (GTK_BOX (box), widget_for_panel_group_dropdown (i, j));
      gtk_box_append (GTK_BOX (box), widget_for_sensor_group_dropdown (i, j));
      gtk_grid_attach (grid, box, i, j, 1, 1);
    }
  }
  return GTK_WIDGET (grid);
}
