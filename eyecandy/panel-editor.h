#ifndef PANEL_EDITOR_H
#define PANEL_EDITOR_H

#include <gtk/gtk.h>

#include "panel.h"
#include "yaml-loader.h"

/* Editor widget — returns a GtkGrid populated with all panel fields */
GtkWidget  *widget_for_panel    (Panel         *p);

/* Clone / append / remove */
Panel      *panel_clone         (const Panel   *src);
void        cfg_append_panel    (Configuration *cfg, Panel *p);
void        cfg_remove_panel    (Configuration *cfg, Panel *p);

/* Store builder for tree picker */
GListStore *build_panel_store   (void);

/* on_select callbacks for use with build_tree_picker() */
void        on_select_clone_panel  (GtkWindow *parent, void *data);
void        on_select_delete_panel (GtkWindow *parent, void *data);

#endif /* PANEL_EDITOR_H */
