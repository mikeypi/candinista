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

#include <string.h>
#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"
#include "sensor.h"
#include "d3-array.h"
#include "yaml-loader.h"
#include "gtk-glue.h"
#include "tree-picker.h"
#include "editor-window.h"
#include "sensor-editor.h"
#include "dropdowns.h"

/* ---------------------------------------------------------------
 * Clone / append / remove
 * ------------------------------------------------------------- */

Sensor *
sensor_clone (const Sensor *src)
{
  Sensor *s = g_malloc (sizeof (Sensor));
  memcpy (s, src, sizeof (Sensor));
  s->name = g_strdup (src->name);
  if (src->n_values > 0) {
    s->x_values = g_memdup2 (src->x_values, src->n_values * sizeof (double));
    s->y_values = g_memdup2 (src->y_values, src->n_values * sizeof (double));
  } else {
    s->x_values = NULL;
    s->y_values = NULL;
  }
  return s;
}

void
cfg_append_sensor (Configuration *cfg, Sensor *s)
{
  cfg->sensor_count++;
  cfg->sensors = g_realloc (cfg->sensors, cfg->sensor_count * sizeof (Sensor *));
  cfg->sensors[cfg->sensor_count - 1] = s;
}

void
cfg_remove_sensor (Configuration *cfg, Sensor *s)
{
  for (int i = 0; i < cfg->sensor_count; i++) {
    if (cfg->sensors[i] == s) {
      sensor_destroy (cfg->sensors[i]);
      memmove (&cfg->sensors[i],
               &cfg->sensors[i + 1],
               (cfg->sensor_count - i - 1) * sizeof (Sensor *));
      cfg->sensor_count--;
      cfg->sensors = g_realloc (cfg->sensors, cfg->sensor_count * sizeof (Sensor *));
      return;
    }
  }
}

/* ---------------------------------------------------------------
 * Editor widget
 * ------------------------------------------------------------- */

GtkWidget *
widget_for_sensor (Sensor *s)
{
  GtkWidget *vbox    = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  GtkGrid   *grid    = GTK_GRID (gtk_grid_new ());
  GtkWidget *scroller = gtk_scrolled_window_new ();

  gtk_widget_set_hexpand (scroller, TRUE);
  gtk_widget_set_vexpand (scroller, TRUE);
  gtk_widget_set_size_request (scroller, 700, (s->n_values != 0) ? 400 : 145);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), vbox);
  gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (grid));

  if (s->n_values > 0) {
    int cols = (s->n_values > 9) ? 9 : s->n_values;

    GtkGrid   *xgrid  = GTK_GRID (gtk_grid_new ());
    GtkWidget *xlabel = new_label_for_string ("X Interpolation Values");
    gtk_label_set_xalign (GTK_LABEL (xlabel), 0.0);
    gtk_grid_attach (xgrid, xlabel, 0, 0, cols, 1);
    for (int i = 0; i < s->n_values; i++)
      gtk_grid_attach (xgrid, new_entry_for_double ((gpointer) &(s->x_values[i])),
                       i % cols, (i / cols) + 1, 1, 1);
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (xgrid));

    GtkGrid   *ygrid  = GTK_GRID (gtk_grid_new ());
    GtkWidget *ylabel = new_label_for_string ("Y Interpolation Values");
    gtk_label_set_xalign (GTK_LABEL (ylabel), 0.0);
    gtk_grid_attach (ygrid, ylabel, 0, 0, cols, 1);
    for (int i = 0; i < s->n_values; i++)
      gtk_grid_attach (ygrid, new_entry_for_double ((gpointer) &(s->y_values[i])),
                       i % cols, (i / cols) + 1, 1, 1);
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (ygrid));
  }

  int row = 0, column = 0;
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can id")),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can data offset")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can data width")),  column, row++, 1, 1);

  row = 0; column += 2;
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("n_values")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("scale")),    column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("offset")),   column, row++, 1, 1);

  row = 0; column += 2;
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("x_index")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("y_index")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("id")),      column, row++, 1, 1);

  row = 0; column = 1;
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(s->can_id))),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s->can_data_offset))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s->can_data_width))),  column, row++, 1, 1);

  row = 0; column += 2;
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int    ((gpointer) &(s->n_values))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(s->scale))),    column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(s->offset))),   column, row++, 1, 1);

  row = 0; column += 2;
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s->x_index))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s->y_index))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s->id))),      column, row++, 1, 1);

  return GTK_WIDGET (scroller);
}

/* ---------------------------------------------------------------
 * Store builder
 * ------------------------------------------------------------- */

static int         sensor_get_x (const void *s) { return sensor_get_x_index ((Sensor *) s); }
static int         sensor_get_y (const void *s) { return sensor_get_y_index ((Sensor *) s); }
static const char *sensor_get_n (const void *s) { return sensor_get_name    ((Sensor *) s); }

GListStore *
build_sensor_store (void)
{
  ItemCollection c = {
    cfg->sensor_count, (void **) cfg->sensors,
    sensor_get_x, sensor_get_y, sensor_get_n
  };
  return build_store (&c);
}

/* ---------------------------------------------------------------
 * on_select callbacks
 * ------------------------------------------------------------- */

void
on_select_clone_sensor (GtkWindow *parent, void *data)
{
  Sensor *s = sensor_clone ((Sensor *) data);
  cfg_append_sensor (cfg, s);
  show_editor_window (GTK_WIDGET (parent), widget_for_sensor (s), "Sensor");
}

void
on_select_delete_sensor (GtkWindow *parent, void *data)
{
  (void) parent;
  cfg_remove_sensor (cfg, (Sensor *) data);
}
