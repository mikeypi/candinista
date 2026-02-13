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

Configuration* cfg;

int raw_output = 0;

static GtkWidget*
make_page_for_can_bus_parameters (const Sensor *s) {
  int row;
  int column;
 
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);

  GtkWidget* label = new_label_for_string ("CAN Bus Parameters");
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 2, 1);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0); 
  
  row = 1; column = 0;

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can id")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can data offset")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("can data width")), column, row++, 1, 1);

  row = 1; column = 1;

  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(s -> can_id))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> can_data_offset))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> can_data_width))), column, row++, 1, 1);

  return (GTK_WIDGET (grid));
}

static GtkWidget*
make_page_for_sensor_parameters (const Sensor *s) {
  int row;
  int column;
 
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);

  GtkWidget* label = new_label_for_string ("Sensor Parameters");
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 2, 1);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0); 
  
  row = 1; column = 0;

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("n_values")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("scale")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("offset")), column, row++, 1, 1);

  row = 1; column = 1;
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> n_values))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(s -> scale))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_double ((gpointer) &(s -> offset))), column, row++, 1, 1);

  return (GTK_WIDGET (grid));
}

static GtkWidget*
make_page_for_candinista_parameters (const Sensor *s) {
  int row;
  int column;
 
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_top (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_end (GTK_WIDGET (grid), 10);

  GtkWidget* label = new_label_for_string ("Candinista Parameters");
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 2, 1);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0); 
  
  row = 1; column = 0;

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("x_index")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("y_index")), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("id")), column, row++, 1, 1);

  row = 1; column = 1;

  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> x_index))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> y_index))), column, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(s -> id))), column, row++, 1, 1);
    
  return (GTK_WIDGET (grid));
}

static GtkWidget*
make_page_for_x_interpolation_values (const Sensor *s) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_end (GTK_WIDGET (grid), 10);

  int number_of_rows = (s -> n_values > 10) ? 10 : s -> n_values;

  if (0 == number_of_rows) {
    return (NULL);
  }
  
  GtkWidget* label = new_label_for_string ("X Interpolation Values");
  gtk_label_set_xalign (GTK_LABEL (label), 0.0); 
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, number_of_rows, 1);

  for (int i = 0; i < s -> n_values; i++) {
    GtkWidget* entry = new_entry_for_double ((gpointer) &(s -> x_values[i]));
    gtk_grid_attach (grid, GTK_WIDGET (entry), i % number_of_rows, (i / number_of_rows) + 1, 1, 1);
  }

  return (GTK_WIDGET (grid));
}

static GtkWidget*
make_page_for_y_interpolation_values (const Sensor *s) {
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_end (GTK_WIDGET (grid), 10);

  int number_of_rows = (s -> n_values > 10) ? 10 : s -> n_values;

  if (0 == number_of_rows) {
    return (NULL);
  }
  
  GtkWidget* label = new_label_for_string ("Y Interpolation Values");
  gtk_label_set_xalign (GTK_LABEL (label), 0.0); 
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, number_of_rows, 1);

  for (int i = 0; i < s -> n_values; i++) {
    GtkWidget* entry = new_entry_for_double ((gpointer) &(s -> y_values[i]));
    gtk_grid_attach (grid, GTK_WIDGET (entry), i % number_of_rows, (i / number_of_rows) + 1, 1, 1);
  }

  return (GTK_WIDGET (grid));
}


extern GtkWidget* make_page_for_radial_panel (const Panel *p, int *);

static GtkWidget*
make_page_for_panel (const Panel *p) {
  int row = 1;
  GtkGrid* grid;
  
  if ((RADIAL_PRESSURE_PANEL == panel_get_type (p)) || (RADIAL_TEMPERATURE_PANEL == panel_get_type (p))) {
    grid = GTK_GRID (make_page_for_radial_panel ((p), &row));
  } else {
    grid = GTK_GRID (gtk_grid_new ());
  }

  g_assert (GTK_IS_GRID (grid));
  gtk_widget_set_margin_end (GTK_WIDGET (grid), 10);

  GtkWidget* label = new_label_for_string ("panel type");
  gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 1, 1);
  GtkWidget* entry = new_entry_for_string (string_from_panel_type_enum (panel_get_type (p)));
  gtk_grid_attach (grid, GTK_WIDGET (entry), 1, 0, 1, 1);

  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("low_warn")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> low_warn))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("high_warn")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> high_warn))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("x_index")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> x_index))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("y_index")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> y_index))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("z_index")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> z_index))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("border")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> border))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("foreground_color")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(p -> foreground_color))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("background_color")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(p -> background_color))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("high_warn_color")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(p -> high_warn_color))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("low_warn_color")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_hex ((gpointer) &(p -> low_warn_color))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("timeout")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> timeout))), 1, row++, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_label_for_string ("id")), 0, row, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (new_entry_for_int ((gpointer) &(p -> id))), 1, row++, 1, 1);

  return (GTK_WIDGET (grid));
}

static GtkWidget*
make_page_for_panel_info (const Sensor *s) {
  Panel* p;

  int x_index = sensor_get_x_index (s);
  int y_index = sensor_get_y_index (s);
  int sensor_id = sensor_get_id (s);
  
  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_widget_set_margin_start (GTK_WIDGET (grid), 10);
  gtk_widget_set_margin_end (GTK_WIDGET (grid), 10);

  for (int i = 0; i < cfg -> panel_z_dimension; i++) {
    if (NULL != (p = cfg_get_panel (cfg, x_index, y_index, i))) {
      if ((NULL != p) && (panel_get_id (p) == sensor_id)) {
	gtk_grid_attach (grid, GTK_WIDGET (make_page_for_panel(p)), i, 0, 1, 1);
      }
    }
  }

  return (GTK_WIDGET (grid));
}

GtkWidget* make_page (const Sensor *s) {
  GtkWidget* vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 20);
  gtk_widget_add_css_class (vbox, "vbox");
  
  /* Print the sensor name at the top */
  GtkWidget* label = gtk_label_new (s -> name);
  gtk_widget_set_margin_top (label, 10);
  gtk_widget_set_margin_bottom (label, 10);
  gtk_widget_set_margin_start (label, 10);
  gtk_widget_set_margin_end (label, 20);

  GtkGrid* grid = GTK_GRID (gtk_grid_new ());
  gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (grid));

  gtk_grid_attach (grid, GTK_WIDGET (make_page_for_can_bus_parameters (s)), 0, 0, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (make_page_for_sensor_parameters (s)), 1, 0, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (make_page_for_candinista_parameters (s)), 2, 0, 1, 1);
  gtk_grid_attach (grid, GTK_WIDGET (label), 3, 0, 1, 1);
  
  GtkWidget* iv = GTK_WIDGET (make_page_for_x_interpolation_values (s));
  if (NULL != iv) {
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (iv));
  }

  iv = GTK_WIDGET (make_page_for_y_interpolation_values (s));
  if (NULL != iv) {
    gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (iv));
  }

  gtk_box_append (GTK_BOX (vbox), GTK_WIDGET (make_page_for_panel_info (s)));

  return (GtkWidget*) vbox;
}

static void
activate (GtkApplication *app, gpointer user_data)
{
  (void)user_data;

  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "StackSidebar Example");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 400);

  // CSS
  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (provider,
    ".cell { border: 2px solid black; padding: 2px 10px; font-size: 12px; "
    "color: #ffa600; background-color: #3b3b3b; }"
    ".vbox { background-color: #636363; }"
  );
  gtk_style_context_add_provider_for_display (
    gdk_display_get_default (),
    GTK_STYLE_PROVIDER (provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child (GTK_WINDOW (window), box);

  // Stack
  GtkWidget *stack = gtk_stack_new ();
  gtk_stack_set_transition_type (GTK_STACK (stack),
                                 GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  gtk_stack_set_transition_duration (GTK_STACK (stack), 150);

  // Sidebar (create BEFORE appending)
  GtkWidget *sidebar = gtk_stack_sidebar_new ();
  gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR (sidebar), GTK_STACK (stack));
  gtk_widget_set_size_request (sidebar, 150, -1);

  // Scroller wrapping the stack (main panel)
  GtkWidget *scroller = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), stack);

  // Make scroller take remaining space
  gtk_widget_set_hexpand (scroller, TRUE);
  gtk_widget_set_vexpand (scroller, TRUE);

  // Pack exactly once
  gtk_box_append (GTK_BOX (box), sidebar);
  gtk_box_append (GTK_BOX (box), scroller);

  // Pages
  for (size_t i = 0; i < cfg->sensor_count; i++) {
    Sensor *s = cfg->sensors[i];

    char page_id[32];
    g_snprintf(page_id, sizeof page_id, "sensor-%zu", i);

    gtk_stack_add_titled (GTK_STACK (stack),
                          make_page (s),
                          page_id,
                          page_id);
  }

  // Optional styling
  gtk_widget_add_css_class (sidebar, "cell");
  gtk_widget_add_css_class (scroller, "cell");  // style the viewport container

  gtk_window_present (GTK_WINDOW (window));
}

#if 0
static void
activate (GtkApplication *app, gpointer user_data) {
  (void) user_data;
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *stack;
  GtkWidget *sidebar;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "StackSidebar Example");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 400);
  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (provider,
				     ".cell { border: 2px solid #888; padding: 20px; border-color: black; border-radius: 0; "
				     "padding-left: 10px; padding-right: 10px; padding-top: 2px; padding-bottom: 2px; "
				     "font-size: 12px; color: #ffa600; background-color: #3b3b3b; }"
				     ".vbox { background-color: #636363; }"
				     );

  gtk_style_context_add_provider_for_display (
					      gdk_display_get_default (),
					      GTK_STYLE_PROVIDER (provider),
					      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  /* Horizontal container */
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_child (GTK_WINDOW (window), box);

  /* Stack (right side) */
  stack = gtk_stack_new ();
  gtk_stack_set_transition_type (GTK_STACK (stack),
				 GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  gtk_stack_set_transition_duration (GTK_STACK (stack), 150);

  GtkWidget *scroller = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), stack);

  gtk_box_append (GTK_BOX (box), sidebar);
  gtk_box_append (GTK_BOX (box), scroller);

  for (size_t i = 0; i < cfg -> sensor_count; i++) {
    char* page_title = (char*) calloc (20, sizeof (char));
    Sensor* s = cfg -> sensors[i];

    sprintf (page_title, "sensor-%d", (int) i);

    gtk_stack_add_titled (GTK_STACK (stack),
			  make_page (s),
			  page_title,
			  page_title);
  }
    
  /* Sidebar (left side) */
  sidebar = gtk_stack_sidebar_new ();
  gtk_stack_sidebar_set_stack (GTK_STACK_SIDEBAR (sidebar),
			       GTK_STACK (stack));
  gtk_widget_set_size_request (sidebar, 150, -1);

  gtk_box_append (GTK_BOX (box), sidebar);
  gtk_widget_add_css_class (sidebar, "cell");
  gtk_box_append (GTK_BOX (box), stack);
  gtk_widget_add_css_class (stack, "cell");
  gtk_window_present (GTK_WINDOW (window));
}
#endif
int
main (int argc, char** argv) {
  GtkApplication *app;
  GIOChannel* input_channel;
  int status;
  int option;

  (void) app;
  (void) input_channel;
  (void) status;

  get_environment_variables ();
  
 if (NULL == (cfg = configuration_load_yaml (config_file_name))) {
    fprintf (stderr, "unable to open config file %s\n", config_file_name);
    exit (-1);
  }
  
  build_tables (cfg);

  while (-1 != (option = getopt (argc, argv, "p"))) {
    switch (option) {
    case 'p':
      configuration_print (stdout, cfg);
      exit (0);
      break;

    default:
      fprintf (stderr, "unknown option %c\n", option);
      fprintf (stderr,
	       "Usage: %s options device_name.\n"
	       "Options:\n"
	       "\t -d: enable datalogging\n"
	       "\t -p: print yaml database\n"
	       "\t -r: print raw sensor values\n",
	       argv[0]);

      exit (-1);
    }
  }

  app = gtk_application_new ("com.example.can.dbc.editor", 0);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  int rc = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);
  return rc;
}

