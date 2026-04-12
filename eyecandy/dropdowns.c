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

#include "d3-array.h"
#include "candinista.h"
#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "gtk-glue.h"

extern Configuration* cfg;

void
callback_for_unit_type_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  int *p = (int*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  *p = index;
}

void
callback_for_yesno_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  int* p = (int*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  *p = index;
}

void
callback_for_panel_type_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  Panel* p = (Panel*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  p -> type = index;
}

void
callback_for_xylayer_index_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  int* p = (int*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  *p = index;
}

void
callback_for_row_index_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  Panel* p = (Panel*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  p -> row_index = index;
}

void
callback_for_layer_index_dropdown (GObject *obj, GParamSpec *pspec, gpointer user_data)
{
  (void) pspec;
  Panel* p = (Panel*) user_data;
  GtkDropDown *dd = GTK_DROP_DOWN (obj);
  guint index = gtk_drop_down_get_selected (dd);
  p -> layer_index = index;
}
  
GtkWidget*
widget_for_unit_type_dropdown (Panel* p, const int state) {
  char** drop_down_strings = (char**) calloc (UNKNOWN_UNIT + 1, sizeof (char*));

  for (unit_type ut = 0; ut < UNKNOWN_UNIT; ut++) {
    drop_down_strings[ut] = str_from_unit_enum (ut);
  }

  GtkWidget* selector = gtk_drop_down_new_from_strings ((const char**) drop_down_strings);
  gtk_drop_down_set_selected (GTK_DROP_DOWN (selector), state);
  
  g_signal_connect (selector, "notify::selected",
		    G_CALLBACK (callback_for_panel_type_dropdown),
		    (gpointer*) p);

 return (GTK_WIDGET (selector));
}

GtkWidget*
widget_for_panel_type_dropdown (const Panel *p) {
  (void) p;
  
  char** drop_down_strings = (char**) calloc (UNKNOWN_PANEL + 1, sizeof (char*));

  for (panel_type pt = 0; pt < UNKNOWN_PANEL; pt++) {
    drop_down_strings[pt] = string_from_panel_type_enum (pt);
  }

  GtkWidget* selector = gtk_drop_down_new_from_strings ((const char**) drop_down_strings);
  gtk_drop_down_set_selected (GTK_DROP_DOWN (selector), p -> type);

  g_signal_connect (selector, "notify::selected",
		    G_CALLBACK (callback_for_panel_type_dropdown),
		    (gpointer*) p);

  return (GTK_WIDGET (selector));
}

GtkWidget*
widget_for_xylayer_index_dropdown (Panel* p, const int range, const int index) {
  GtkWidget* selector;
  
  switch (range) {
  case 7:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", "2", "3", "4", "5", "6", NULL});
    break;
  case 6:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", "2", "3", "4", "5", NULL});
    break;
  case 5:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", "2", "3", "4", NULL});
    break;
  case 4:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", "2", "3", NULL});
    break;
  case 3:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", "2", NULL});
    break;
  case 2:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", "1", NULL});
    break;
  case 1:
    selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"0", NULL});
    break;
  }

  gtk_drop_down_set_selected (GTK_DROP_DOWN (selector), index);

  g_signal_connect (selector, "notify::selected",
		    G_CALLBACK (callback_for_panel_type_dropdown),
		    (gpointer*) p);

  return (GTK_WIDGET (selector));
}

GtkWidget*
widget_for_yesno_dropdown (Panel* p, const int state) {
  GtkWidget* selector = selector = gtk_drop_down_new_from_strings ((const char**)(char *[]){"no", "yes", NULL});  
  gtk_drop_down_set_selected (GTK_DROP_DOWN (selector), state);

  g_signal_connect (selector, "notify::selected",
		    G_CALLBACK (callback_for_panel_type_dropdown),
		    (gpointer*) p);
  
  return (GTK_WIDGET (selector));
}

