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
#include "panel-editor.h"
#include "dropdowns.h"

extern GtkWidget *widget_for_radial_panel (const Panel *p, int row, int column);
extern GtkWidget *widget_for_linear_panel (const Panel *p, int row, int column);
extern GtkWidget *widget_for_tpms_panel   (const Panel *p, int row, int column);
extern GtkWidget *widget_for_gps_panel    (const Panel *p, int row, int column);

/* ---------------------------------------------------------------
 * Clone / append / remove
 * ------------------------------------------------------------- */

Panel *
panel_clone (const Panel *src)
{
  Panel *p = g_malloc (sizeof (Panel));
  memcpy (p, src, sizeof (Panel));
  return p;
}

void
cfg_append_panel (Configuration *cfg, Panel *p)
{
  cfg->panel_count++;
  cfg->panels = g_realloc (cfg->panels, cfg->panel_count * sizeof (Panel *));
  cfg->panels[cfg->panel_count - 1] = p;
}

void
cfg_remove_panel (Configuration *cfg, Panel *p)
{
  for (int i = 0; i < cfg->panel_count; i++) {
    if (cfg->panels[i] == p) {
      panel_destroy (cfg->panels[i]);
      memmove (&cfg->panels[i],
               &cfg->panels[i + 1],
               (cfg->panel_count - i - 1) * sizeof (Panel *));
      cfg->panel_count--;
      cfg->panels = g_realloc (cfg->panels, cfg->panel_count * sizeof (Panel *));
      return;
    }
  }
}

/* ---------------------------------------------------------------
 * Editor widget
 * ------------------------------------------------------------- */

GtkWidget *
widget_for_panel (Panel *p)
{
  int row = 0, column = 3;
  GtkGrid *grid;

  switch (panel_get_type (p)) {
  case RADIAL_PRESSURE_PANEL:
  case RADIAL_TEMPERATURE_PANEL: grid = GTK_GRID (widget_for_radial_panel (p, row, column)); break;
  case LINEAR_PRESSURE_PANEL:
  case LINEAR_TEMPERATURE_PANEL: grid = GTK_GRID (widget_for_linear_panel (p, row, column)); break;
  case TPMS_PANEL:               grid = GTK_GRID (widget_for_tpms_panel   (p, row, column)); break;
  case GPS_PANEL:                grid = GTK_GRID (widget_for_gps_panel    (p, row, column)); break;
  default:                       grid = GTK_GRID (gtk_grid_new ());                          break;
  }

  g_assert (GTK_IS_GRID (grid));

  row = 0; column = 0;
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("panel type")),      column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("low_warn")),         column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("high_warn")),        column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("column_index")),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("row_index")),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("layer_index")),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("border")),           column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("foreground_color")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("background_color")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("high_warn_color")),  column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("low_warn_color")),   column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("timeout")),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("id")),               column, row++, 1, 1);

  row = 0; column = 1;
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_panel_type_dropdown (p)),                               column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int   ((gpointer) &(p->low_warn))),                  column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int   ((gpointer) &(p->high_warn))),                 column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_xylayer_index_dropdown (p, cfg->x_dimension, p->column_index)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_xylayer_index_dropdown (p, cfg->y_dimension, p->row_index)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_xylayer_index_dropdown (p, cfg->z_dimension, p->layer_index)), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (widget_for_yesno_dropdown (p, p->border)),                        column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex   ((gpointer) &(p->foreground_color))),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex   ((gpointer) &(p->background_color))),          column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex   ((gpointer) &(p->high_warn_color))),           column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex   ((gpointer) &(p->low_warn_color))),            column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int   ((gpointer) &(p->timeout))),                   column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int   ((gpointer) &(p->id))),                        column, row++, 1, 1);

  return GTK_WIDGET (grid);
}

/* ---------------------------------------------------------------
 * Store builder
 * ------------------------------------------------------------- */

static int         panel_get_x (const void *p) { return panel_get_column_index ((Panel *) p); }
static int         panel_get_y (const void *p) { return panel_get_row_index ((Panel *) p); }
static const char *panel_get_n (const void *p) { return ((Panel *) p)->name;             }

GListStore *
build_panel_store (void)
{
  ItemCollection c = {
    cfg->panel_count, (void **) cfg->panels,
    panel_get_x, panel_get_y, panel_get_n
  };
  return build_store (&c);
}

/* ---------------------------------------------------------------
 * on_select callbacks
 * ------------------------------------------------------------- */

void
on_select_clone_panel (GtkWindow *parent, void *data)
{
  Panel *p = panel_clone ((Panel *) data);
  cfg_append_panel (cfg, p);
  show_editor_window (GTK_WIDGET (parent), widget_for_panel (p), "Panel");
}

void
on_select_delete_panel (GtkWindow *parent, void *data)
{
  (void) parent;
  cfg_remove_panel (cfg, (Panel *) data);
}
