#include <stdio.h>
#include <gtk/gtk.h>

#include "units.h"
#include "panel.h"

gboolean
gtk_update_gauge_panel_value (gpointer user_data) {
  struct
  {
    GtkDrawingArea *area;
    Panel *p;
  } *ctx = user_data;
  
  gtk_widget_queue_draw (GTK_WIDGET (ctx -> area));

  return G_SOURCE_CONTINUE;
}

void
gtk_draw_gauge_panel_cb (GtkDrawingArea* area,
                  cairo_t*        cr,
                  int             width,
                  int             height,
                  gpointer        user_data)
{
  Panel* p = user_data;

  g_return_if_fail (p != NULL);
  g_return_if_fail (p -> draw != NULL);

  p -> draw (area, cr, width, height, p);
}

static void
on_activate_for_int (GtkEntry *entry, gpointer user_data) {
  int* s = user_data;
  const char* txt = gtk_editable_get_text (GTK_EDITABLE (entry));

  errno = 0;
  char* end = NULL;
  unsigned long v = strtoul (txt, &end, 0);   // base 0 accepts 123 or 0x7B

  if (errno == 0 && end != txt && *end == '\0') {
    *s = (uint32_t)v;
    g_print ("Committed value = 0x%x\n", *s);
  }
}

static void
on_activate_for_double (GtkEntry *entry, gpointer user_data) {
  double* s = user_data;
  const char* txt = gtk_editable_get_text (GTK_EDITABLE (entry));

  errno = 0;
  char* end = NULL;
  unsigned long v = strtod (txt, &end);   

  if (errno == 0 && end != txt && *end == '\0') {
    *s = (double)v;
    g_print ("Committed value = %f\n", *s);
  }
}

static void
on_activate_for_string (GtkEntry *entry, gpointer user_data) {
  (void) user_data;
  const char* txt = gtk_editable_get_text (GTK_EDITABLE (entry));
  g_print ("Committed value = %s\n", txt);
}

GtkWidget*
new_label_for_string (char* c) {
  GtkWidget* label = gtk_label_new (c);
  gtk_widget_add_css_class (label, "cell");
  gtk_label_set_xalign (GTK_LABEL (label), 1.0);   // text aligned right
  return (label);
}

GtkWidget*
new_entry_for_int (gpointer user_data) {
  int* s = user_data;
  char buffer[80];
    
  GtkWidget* entry = gtk_entry_new ();
  snprintf (buffer, sizeof buffer, "%d", *s);
  gtk_editable_set_text (GTK_EDITABLE (entry), buffer);
  g_signal_connect (entry, "activate", G_CALLBACK (on_activate_for_int), (gpointer) s);
  gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_NUMBER);
  gtk_widget_add_css_class (entry, "cell");
  return (entry);
}

GtkWidget*
new_entry_for_hex (gpointer user_data) {
  int* s = user_data;
  char buffer[80];
    
  GtkWidget* entry = gtk_entry_new ();
  snprintf (buffer, sizeof buffer, "0x%02x", *s);
  gtk_editable_set_text (GTK_EDITABLE (entry), buffer);
  g_signal_connect (entry, "activate", G_CALLBACK (on_activate_for_int), (gpointer) s);
  gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_NUMBER);
  gtk_widget_add_css_class (entry, "cell");
  return (entry);
}

GtkWidget*
new_entry_for_double (gpointer user_data) {
  double* s = user_data;
  char buffer[80];
    
  GtkWidget* entry = gtk_entry_new ();
  snprintf (buffer, sizeof buffer, "%.2f", *s);
  gtk_editable_set_text (GTK_EDITABLE (entry), buffer);
  g_signal_connect (entry, "activate", G_CALLBACK (on_activate_for_double), (gpointer) s);
  gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_NUMBER);
  gtk_widget_add_css_class (entry, "cell");
  return (entry);
}

GtkWidget*
new_entry_for_string (gpointer user_data) {
  char* s = user_data;

  GtkWidget* entry = gtk_entry_new ();
  gtk_editable_set_text (GTK_EDITABLE (entry), s);
  g_signal_connect (entry, "activate", G_CALLBACK (on_activate_for_string), (gpointer) s);
  gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_NUMBER);
  gtk_widget_add_css_class (entry, "cell");
  return (entry);
}

