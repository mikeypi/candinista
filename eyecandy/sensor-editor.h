#ifndef SENSOR_EDITOR_H
#define SENSOR_EDITOR_H

#include <gtk/gtk.h>

#include "sensor.h"
#include "yaml-loader.h"

/* Editor widget — returns a scrolled window containing all sensor fields */
GtkWidget  *widget_for_sensor      (Sensor        *s);

/* Clone / append / remove */
Sensor     *sensor_clone           (const Sensor  *src);
void        cfg_append_sensor      (Configuration *cfg, Sensor *s);
void        cfg_remove_sensor      (Configuration *cfg, Sensor *s);

/* Store builder for tree picker */
GListStore *build_sensor_store     (void);

/* on_select callbacks for use with build_tree_picker() */
void        on_select_clone_sensor  (GtkWindow *parent, void *data);
void        on_select_delete_sensor (GtkWindow *parent, void *data);

#endif /* SENSOR_EDITOR_H */
