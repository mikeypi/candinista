#ifndef TREE_PICKER_H
#define TREE_PICKER_H

#include <gtk/gtk.h>

/*
 * ItemCollection — describes an array of opaque items for build_store().
 * The caller provides function pointers for x/y index and name access
 * so build_store() can remain generic.
 */
typedef struct {
  int          count;
  void       **items;
  int        (*get_x)    (const void *item);
  int        (*get_y)    (const void *item);
  const char *(*get_name)(const void *item);
} ItemCollection;

/*
 * build_store — builds a GListStore tree grouped by x/y position.
 * Each leaf node carries the original item pointer as "payload".
 * Group nodes have payload == NULL.
 */
GListStore *build_store      (const ItemCollection *c);

/*
 * build_tree_picker — creates and presents a modal GtkColumnView picker
 * dialog. on_select is called with the parent window and the item's
 * payload pointer when the user clicks a leaf row. The dialog closes
 * itself automatically after on_select returns.
 */
GtkWidget  *build_tree_picker (GtkWindow  *parent,
                                const char *title,
                                GListStore *root_store,
                                void      (*on_select)(GtkWindow *parent,
                                                       void      *data));

#endif /* TREE_PICKER_H */
