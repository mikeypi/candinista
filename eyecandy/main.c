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
#include <string.h>
#include <gtk/gtk.h>

#include "d3-array.h"
#include "candinista.h"
#include "units.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "yaml-printer.h"
#include "tree-picker.h"
#include "panel-editor.h"
#include "sensor-editor.h"
#include "editor-window.h"

Configuration *cfg;
int raw_output = 0;

/* ---------------------------------------------------------------
 * Save As
 * ------------------------------------------------------------- */

static void
save_as_callback (GObject *source, GAsyncResult *result, gpointer user_data)
{
  (void) user_data;
  GFile *file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (source), result, NULL);
  if (file == NULL) return;

  const char *path = g_file_get_path (file);
  FILE       *fp   = fopen (path, "w");
  if (fp == NULL) {
    g_warning ("Could not open %s for writing", path);
  } else {
    configuration_print (fp, cfg);
    fclose (fp);
  }
  g_object_unref (file);
}

static void
action_save_as (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action; (void) parameter;
  GtkFileDialog *d = gtk_file_dialog_new ();
  gtk_file_dialog_set_title        (d, "Save Configuration");
  gtk_file_dialog_set_initial_name (d, "config.yaml");
  gtk_file_dialog_save (d, GTK_WINDOW (user_data), NULL,
                        (GAsyncReadyCallback) save_as_callback, NULL);
}

/* ---------------------------------------------------------------
 * Add blank actions
 * ------------------------------------------------------------- */

static void
action_add_blank_panel (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action; (void) parameter;
  PanelParameters pp = {0};
  pp.type             = UNKNOWN_PANEL;
  pp.foreground_color = DEFAULT_FOREGROUND_RGB;
  pp.background_color = DEFAULT_BACKGROUND_RGB;
  pp.high_warn_color  = DEFAULT_HIGH_WARN_RGB;
  pp.low_warn_color   = DEFAULT_LOW_WARN_RGB;
  pp.timeout          = DEFAULT_TIMEOUT;
  strncpy (pp.name, "new panel", sizeof (pp.name) - 1);
  Panel *p = panel_init_base (&pp, g_malloc0 (sizeof (Panel)));
  cfg_append_panel (cfg, p);
  show_editor_window (GTK_WIDGET (user_data), widget_for_panel (p), "Panel");
}

static void
action_add_blank_sensor (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void) action; (void) parameter;
  Sensor *s = g_malloc0 (sizeof (Sensor));
  s->name   = g_strdup ("new sensor");
  cfg_append_sensor (cfg, s);
  show_editor_window (GTK_WIDGET (user_data), widget_for_sensor (s), "Sensor");
}

/* ---------------------------------------------------------------
 * Picker actions — one macro per action to avoid repetition
 * ------------------------------------------------------------- */

#define PICKER_ACTION(name, store_fn, title, cb)                               \
static void name (GSimpleAction *a, GVariant *p, gpointer user_data) {         \
  (void) a; (void) p;                                                          \
  GListStore *store = store_fn ();                                              \
  build_tree_picker (GTK_WINDOW (user_data), title, store, cb);                \
  g_object_unref (store);                                                       \
}

PICKER_ACTION (action_clone_panel,   build_panel_store,  "Clone Panel",   on_select_clone_panel)
PICKER_ACTION (action_delete_panel,  build_panel_store,  "Delete Panel",  on_select_delete_panel)
PICKER_ACTION (action_clone_sensor,  build_sensor_store, "Clone Sensor",  on_select_clone_sensor)
PICKER_ACTION (action_delete_sensor, build_sensor_store, "Delete Sensor", on_select_delete_sensor)

/* ---------------------------------------------------------------
 * Application activate
 * ------------------------------------------------------------- */

static void
activate (GtkApplication *app, gpointer user_data)
{
  (void) user_data;

  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title        (GTK_WINDOW (window), "Candinista Editor");
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 300);

  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_path (provider, "styles.css");
  gtk_style_context_add_provider_for_display (
    gdk_display_get_default (),
    GTK_STYLE_PROVIDER (provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  /* Register actions */
  struct { const char *name; GCallback cb; } actions[] = {
    { "save-as",       G_CALLBACK (action_save_as)          },
    { "add-panel",     G_CALLBACK (action_add_blank_panel)  },
    { "clone-panel",   G_CALLBACK (action_clone_panel)      },
    { "delete-panel",  G_CALLBACK (action_delete_panel)     },
    { "add-sensor",    G_CALLBACK (action_add_blank_sensor) },
    { "clone-sensor",  G_CALLBACK (action_clone_sensor)     },
    { "delete-sensor", G_CALLBACK (action_delete_sensor)    },
  };

  for (int i = 0; i < (int) G_N_ELEMENTS (actions); i++) {
    GSimpleAction *a = g_simple_action_new (actions[i].name, NULL);
    g_signal_connect (a, "activate", actions[i].cb, window);
    g_action_map_add_action (G_ACTION_MAP (app), G_ACTION (a));
  }

  /* Build menu bar */
  GMenu *menubar     = g_menu_new ();
  GMenu *file_menu   = g_menu_new ();
  GMenu *panel_menu  = g_menu_new ();
  GMenu *sensor_menu = g_menu_new ();

  g_menu_append (file_menu,   "Save As",          "app.save-as");
  g_menu_append (panel_menu,  "Add Blank Panel",  "app.add-panel");
  g_menu_append (panel_menu,  "Clone Panel",      "app.clone-panel");
  g_menu_append (panel_menu,  "Delete Panel",     "app.delete-panel");
  g_menu_append (sensor_menu, "Add Blank Sensor", "app.add-sensor");
  g_menu_append (sensor_menu, "Clone Sensor",     "app.clone-sensor");
  g_menu_append (sensor_menu, "Delete Sensor",    "app.delete-sensor");

  g_menu_append_submenu (menubar, "File",    G_MENU_MODEL (file_menu));
  g_menu_append_submenu (menubar, "Panels",  G_MENU_MODEL (panel_menu));
  g_menu_append_submenu (menubar, "Sensors", G_MENU_MODEL (sensor_menu));

  gtk_application_set_menubar (app, G_MENU_MODEL (menubar));
  gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (window), TRUE);

  gtk_window_set_child (GTK_WINDOW (window), widget_for_legend ());
  gtk_window_present (GTK_WINDOW (window));

  GtkSettings *s = gtk_settings_get_default ();
  g_object_set (s, "gtk-entry-select-on-focus", FALSE, NULL);
}

/* ---------------------------------------------------------------
 * main
 * ------------------------------------------------------------- */

int
main (int argc, char **argv)
{
  GtkApplication *app;
  int             option;

  get_environment_variables ();

  if (NULL == (cfg = configuration_load_yaml (config_file_name))) {
    fprintf (stderr, "unable to open config file %s\n", config_file_name);
    exit (-1);
  }

  cfg_build_tables (cfg);

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
