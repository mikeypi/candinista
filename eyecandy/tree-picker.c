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

#include <gtk/gtk.h>

#include "units.h"
#include "candinista.h"
#include "panel.h"
#include "sensor.h"
#include "d3-array.h"
#include "yaml-loader.h"
#include "tree-picker.h"

/* Context passed to on_tree_row_activated */
typedef struct {
  GtkWindow  *picker;
  GtkWindow  *parent;
  void      (*on_select)(GtkWindow *parent, void *data);
} PickerContext;

/* ---------------------------------------------------------------
 * Position label helper
 * ------------------------------------------------------------- */

static const char *
position_label (int i, int j)
{
  static const char *labels[3][2] = {
    { "upper left",   "lower left"   },
    { "upper middle", "lower middle" },
    { "upper right",  "lower right"  }
  };
  return (i < 3 && j < 2) ? labels[i][j] : "unknown";
}

/* ---------------------------------------------------------------
 * GtkStringObject helpers
 * ------------------------------------------------------------- */

/* Creates a GtkStringObject with optional payload pointer and
   optional children GListStore attached as object data. */
static GtkStringObject *
make_tree_item (const char *label, void *payload, GListStore *children)
{
  GtkStringObject *obj = gtk_string_object_new (label);
  if (payload)  g_object_set_data (G_OBJECT (obj), "payload", payload);
  if (children) g_object_set_data_full (G_OBJECT (obj), "children",
                                        children, g_object_unref);
  return obj;
}

/* ---------------------------------------------------------------
 * GtkTreeListModel child provider
 * ------------------------------------------------------------- */

static GListModel *
tree_create_child_model (void *item, gpointer user_data)
{
  (void) user_data;
  GListStore *children = g_object_get_data (item, "children");
  if (children == NULL) return NULL;
  g_object_ref (children);
  return G_LIST_MODEL (children);
}

/* ---------------------------------------------------------------
 * GtkColumnView factory callbacks
 * ------------------------------------------------------------- */

static void
setup_label_cb (GtkSignalListItemFactory *f, GtkListItem *item, gpointer u)
{
  (void) f; (void) u;
  GtkWidget *label = gtk_label_new (NULL);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_list_item_set_child (item, label);
}

static void
bind_label_cb (GtkSignalListItemFactory *f, GtkListItem *item, gpointer u)
{
  (void) f; (void) u;
  GtkTreeListRow  *tlr    = GTK_TREE_LIST_ROW (gtk_list_item_get_item (item));
  GtkStringObject *strobj = GTK_STRING_OBJECT (gtk_tree_list_row_get_item (tlr));
  GtkWidget       *label  = gtk_list_item_get_child (item);
  gtk_label_set_text (GTK_LABEL (label), gtk_string_object_get_string (strobj));
  gtk_widget_set_margin_start (label, gtk_tree_list_row_get_depth (tlr) * 16);
  g_object_unref (strobj);
}

/* ---------------------------------------------------------------
 * Row activation
 * ------------------------------------------------------------- */

static void
on_tree_row_activated (GtkColumnView *view, guint position, gpointer user_data)
{
  PickerContext     *ctx   = user_data;
  GtkSelectionModel *model = gtk_column_view_get_model (view);
  GObject           *item  = g_list_model_get_item (G_LIST_MODEL (model), position);
  if (item == NULL) return;

  GtkTreeListRow  *tlr     = GTK_TREE_LIST_ROW (item);
  GtkStringObject *strobj  = GTK_STRING_OBJECT (gtk_tree_list_row_get_item (tlr));
  void            *payload = g_object_get_data (G_OBJECT (strobj), "payload");
  g_object_unref (item);

  if (payload == NULL) return;  /* group row — ignore */

  ctx->on_select (ctx->parent, payload);
  gtk_window_destroy (ctx->picker);
}

/* ---------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------- */

GListStore *
build_store (const ItemCollection *c)
{
  GListStore *root = g_list_store_new (GTK_TYPE_STRING_OBJECT);

  for (int i = 0; i < cfg->x_dimension; i++) {
    for (int j = 0; j < cfg->y_dimension; j++) {
      GListStore *children  = g_list_store_new (GTK_TYPE_STRING_OBJECT);
      gboolean    has_items = FALSE;

      for (int k = 0; k < c->count; k++) {
        void *item = c->items[k];
        if (c->get_x (item) == i && c->get_y (item) == j) {
          GtkStringObject *obj = make_tree_item (c->get_name (item), item, NULL);
          g_list_store_append (children, obj);
          g_object_unref (obj);
          has_items = TRUE;
        }
      }

      if (has_items) {
        GtkStringObject *group = make_tree_item (position_label (i, j), NULL, children);
        g_list_store_append (root, group);
        g_object_unref (group);
      } else {
        g_object_unref (children);
      }
    }
  }
  return root;
}

GtkWidget *
build_tree_picker (GtkWindow  *parent,
                   const char *title,
                   GListStore *root_store,
                   void      (*on_select)(GtkWindow *parent, void *data))
{
  GtkWidget *dialog = gtk_window_new ();
  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  gtk_window_set_modal         (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_title         (GTK_WINDOW (dialog), title);
  gtk_window_set_default_size  (GTK_WINDOW (dialog), 320, 450);

  GtkTreeListModel   *tree = gtk_tree_list_model_new (
    G_LIST_MODEL (root_store), FALSE, TRUE,
    tree_create_child_model, NULL, NULL);
  GtkSingleSelection *sel  = gtk_single_selection_new (G_LIST_MODEL (tree));
  GtkWidget          *cv   = gtk_column_view_new (GTK_SELECTION_MODEL (sel));
  gtk_column_view_set_show_column_separators (GTK_COLUMN_VIEW (cv), FALSE);
  gtk_column_view_set_show_row_separators    (GTK_COLUMN_VIEW (cv), TRUE);

  GtkListItemFactory  *factory = gtk_signal_list_item_factory_new ();
  g_signal_connect (factory, "setup", G_CALLBACK (setup_label_cb), NULL);
  g_signal_connect (factory, "bind",  G_CALLBACK (bind_label_cb),  NULL);
  GtkColumnViewColumn *col = gtk_column_view_column_new (NULL, factory);
  gtk_column_view_column_set_expand (col, TRUE);
  gtk_column_view_append_column (GTK_COLUMN_VIEW (cv), col);

  GtkWidget *scroller = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scroller), cv);
  gtk_widget_set_vexpand (scroller, TRUE);
  gtk_window_set_child (GTK_WINDOW (dialog), scroller);

  PickerContext *ctx = g_new0 (PickerContext, 1);
  ctx->picker        = GTK_WINDOW (dialog);
  ctx->parent        = parent;
  ctx->on_select     = on_select;
  g_object_set_data_full (G_OBJECT (dialog), "picker-ctx", ctx, (GDestroyNotify) g_free);
  g_signal_connect (cv, "activate", G_CALLBACK (on_tree_row_activated), ctx);

  gtk_window_present (GTK_WINDOW (dialog));
  return dialog;
}
