// Yamlx-loader.c  (single-file refactor)

#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "units.h"
#include "cairo-misc.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "candinista.h"

/* -------------------- existing enum parsing -------------------- */

static panel_type
enum_from_type_str (const char* temp) {
    char buffer[80];
    size_t i;

    if (strlen (temp) > 80) return UNKNOWN_PANEL;

    for (i = 0; i < strlen (temp); i++) buffer[i] = (char)tolower ((unsigned char)temp[i]);
    buffer[i] = '\0';

    if (0 == strcmp (buffer, "radial_pressure"))      return RADIAL_PRESSURE_PANEL;
    if (0 == strcmp (buffer, "linear_pressure"))      return LINEAR_PRESSURE_PANEL;
    if (0 == strcmp (buffer, "radial_temperature"))   return RADIAL_TEMPERATURE_PANEL;
    if (0 == strcmp (buffer, "linear_temperature"))   return LINEAR_TEMPERATURE_PANEL;
    if (0 == strcmp (buffer, "info"))                 return INFO_PANEL;
    if (0 == strcmp (buffer, "tpms"))                 return TPMS_PANEL;
    if (0 == strcmp (buffer, "gps"))                  return GPS_PANEL;

    fprintf (stderr, "unknown panel type %s\n", temp);
    return UNKNOWN_PANEL;
}

/* -------------------- robust double array loader -------------------- */
/*
 * Correct libyaml behavior:
 * For "x_values: [1,2,3]" the value is a YAML_SEQUENCE_START_EVENT.
 * Then scalar events for the numbers, then YAML_SEQUENCE_END_EVENT.
 *
 * So: call this AFTER you see YAML_SEQUENCE_START_EVENT.
 */
static int
load_double_array (yaml_parser_t *parser, double **out, size_t *count)
{
    yaml_event_t event;
    size_t capacity = 8;

    *count = 0;
    *out = (double*)malloc(capacity * sizeof(double));
    if (!*out) return 0;

    while (1) {
        if (!yaml_parser_parse(parser, &event)) {
            free(*out); *out = NULL; *count = 0;
            return 0;
        }

        if (event.type == YAML_SEQUENCE_END_EVENT) {
            yaml_event_delete(&event);
            break;
        }

        if (event.type != YAML_SCALAR_EVENT) {
            // We only accept scalar numbers inside the sequence.
            yaml_event_delete(&event);
            free(*out); *out = NULL; *count = 0;
            return 0;
        }

        const char *sv = (const char *)event.data.scalar.value;
        double v = atof(sv);

        if (*count == capacity) {
            capacity *= 2;
            double *tmp = (double*)realloc(*out, capacity * sizeof(double));
            if (!tmp) {
                yaml_event_delete(&event);
                free(*out); *out = NULL; *count = 0;
                return 0;
            }

            *out = tmp;
        }

        (*out)[(*count)++] = v;
        yaml_event_delete(&event);
    }

    return 1;
}

/* -------------------- dispatch framework -------------------- */

typedef int (*yaml_setter_scalar_fn)(const char *v, void *obj);
typedef int (*yaml_setter_seq_fn)(yaml_parser_t *parser, void *obj);

typedef struct {
    const char *key;
    yaml_setter_scalar_fn scalar;
    yaml_setter_seq_fn    seq;
} kv_handler;

/* normalize keys (fixes accidental "temperature units" etc.) */
static void
normalize_key_inplace(char *k)
{
    // trim
    char *start = k;
    while (*start && g_ascii_isspace(*start)) start++;
    if (start != k) memmove(k, start, strlen(start) + 1);

    size_t n = strlen(k);
    while (n > 0 && g_ascii_isspace(k[n-1])) k[--n] = '\0';

    // spaces -> underscores
    for (char *p = k; *p; p++) {
        if (*p == ' ') *p = '_';
    }
}

/* -------------------- sensor setters -------------------- */

static int st_set_name(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    strncpy(st -> name, v, 63);
    st -> name[63] = '\0';
    return 1;
}
static int st_set_can_data_offset(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> can_data_offset = atoi(v);
    return 1;
}
static int st_set_can_data_width(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> can_data_width = atoi(v);
    return 1;
}
static int st_set_column_index(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> column_index = atoi(v);
    return 1;
}
static int st_set_row_index(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> row_index = atoi(v);
    return 1;
}
static int st_set_can_id(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> can_id = strtol(v, NULL, 16);
    return 1;
}
static int st_set_scale(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> scale = atof(v);
    return 1;
}
static int st_set_offset(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> offset = atof(v);
    return 1;
}
static int st_set_id(const char *v, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    st -> id = atoi(v);
    return 1;
}

static int st_set_x_values_seq(yaml_parser_t *parser, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    // overwrite if repeated
    free(st -> x_values);
    st -> x_values = NULL;
    st -> n_values = 0;
    return load_double_array(parser, &st -> x_values, &st -> n_values);
}

static int st_set_y_values_seq(yaml_parser_t *parser, void *obj) {
    SensorParameters *st = (SensorParameters*)obj;
    // overwrite if repeated
    free(st -> y_values);
    st -> y_values = NULL;
    st -> n_values = 0;
    return load_double_array(parser, &st -> y_values, &st -> n_values);
}

/* -------------------- panel setters -------------------- */

static int gt_set_type(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> type = enum_from_type_str(v);
    return 1;
}
static int gt_set_low_warn(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> low_warn = atof(v);
    return 1;
}
static int gt_set_high_warn(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> high_warn = atof(v);
    return 1;
}
static int gt_set_min_value(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> min = atof(v);
    return 1;
}
static int gt_set_max_value(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> max = atof(v);
    return 1;
}
static int gt_set_label(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    strncpy(gt -> label, v, 63);
    gt -> label[63] = '\0';
    return 1;
}
static int gt_set_border(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> border = atoi(v);
    return 1;
}
static int gt_set_column_index(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> column_index = atoi(v);
    return 1;
}
static int gt_set_row_index(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> row_index = atoi(v);
    return 1;
}
static int gt_set_layer_index(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> layer_index = atoi(v);
    return 1;
}
static int gt_set_timeout(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> timeout = atoi(v);
    return 1;
}
static int gt_set_foreground_color(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> foreground_color = strtol(v, NULL, 16);
    return 1;
}
static int gt_set_background_color(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> background_color = strtol(v, NULL, 16);
    return 1;
}
static int gt_set_low_warn_color(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> low_warn_color = strtol(v, NULL, 16);
    return 1;
}
static int gt_set_high_warn_color(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> high_warn_color = strtol(v, NULL, 16);
    return 1;
}
static int gt_set_id(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> id = atoi(v);
    return 1;
}
static int gt_set_output_format(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    strncpy(gt -> output_format, v, 63);
    gt -> output_format[63] = '\0';
    return 1;
}
static int gt_set_units(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> units = enum_from_unit_str(v);
    return 1;
}
static int gt_set_pressure_units(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> pressure_units = enum_from_unit_str(v);
    return 1;
}
static int gt_set_temperature_units(const char *v, void *obj) {
    PanelParameters *gt = (PanelParameters*)obj;
    gt -> temperature_units = enum_from_unit_str(v);
    return 1;
}

/* -------------------- key tables -------------------- */

#ifdef NOT_USED
static GHashTable *sensor_handlers(void)
{
    static GHashTable *ht = NULL;
    if (ht) return ht;

    ht = g_hash_table_new(g_str_hash, g_str_equal);

    // scalar
    g_hash_table_insert(ht, "name",            (gpointer)(kv_handler*) &(kv_handler){ "name", st_set_name, NULL });
    g_hash_table_insert(ht, "can_data_offset", (gpointer)(kv_handler*) &(kv_handler){ "can_data_offset", st_set_can_data_offset, NULL });
    g_hash_table_insert(ht, "can_data_width",  (gpointer)(kv_handler*) &(kv_handler){ "can_data_width",  st_set_can_data_width, NULL });
    g_hash_table_insert(ht, "column_index",    (gpointer)(kv_handler*) &(kv_handler){ "column_index", st_set_column_index, NULL });
    g_hash_table_insert(ht, "row_index",       (gpointer)(kv_handler*) &(kv_handler){ "row_index", st_set_row_index, NULL });
    g_hash_table_insert(ht, "can_id",          (gpointer)(kv_handler*) &(kv_handler){ "can_id",  st_set_can_id, NULL });
    g_hash_table_insert(ht, "scale",           (gpointer)(kv_handler*) &(kv_handler){ "scale",   st_set_scale, NULL });
    g_hash_table_insert(ht, "offset",          (gpointer)(kv_handler*) &(kv_handler){ "offset",  st_set_offset, NULL });
    g_hash_table_insert(ht, "id",              (gpointer)(kv_handler*) &(kv_handler){ "id",      st_set_id, NULL });

    // sequences
    g_hash_table_insert(ht, "x_values",        (gpointer)(kv_handler*) &(kv_handler){ "x_values", NULL, st_set_x_values_seq });
    g_hash_table_insert(ht, "y_values",        (gpointer)(kv_handler*) &(kv_handler){ "y_values", NULL, st_set_y_values_seq });

    return ht;
}

static GHashTable *panel_handlers(void)
{
    static GHashTable *ht = NULL;
    if (ht) return ht;

    ht = g_hash_table_new(g_str_hash, g_str_equal);

    g_hash_table_insert(ht, "type",              (gpointer)(kv_handler*) &(kv_handler){ "type", gt_set_type, NULL });
    g_hash_table_insert(ht, "low_warn",          (gpointer)(kv_handler*) &(kv_handler){ "low_warn", gt_set_low_warn, NULL });
    g_hash_table_insert(ht, "high_warn",         (gpointer)(kv_handler*) &(kv_handler){ "high_warn", gt_set_high_warn, NULL });
    g_hash_table_insert(ht, "min_value",         (gpointer)(kv_handler*) &(kv_handler){ "min_value", gt_set_min_value, NULL });
    g_hash_table_insert(ht, "max_value",         (gpointer)(kv_handler*) &(kv_handler){ "max_value", gt_set_max_value, NULL });
    g_hash_table_insert(ht, "label",             (gpointer)(kv_handler*) &(kv_handler){ "label", gt_set_label, NULL });
    g_hash_table_insert(ht, "border",            (gpointer)(kv_handler*) &(kv_handler){ "border", gt_set_border, NULL });
    g_hash_table_insert(ht, "column_index",      (gpointer)(kv_handler*) &(kv_handler){ "column_index", gt_set_column_index, NULL });
    g_hash_table_insert(ht, "row_index",         (gpointer)(kv_handler*) &(kv_handler){ "row_index", gt_set_row_index, NULL });
    g_hash_table_insert(ht, "layer_index",       (gpointer)(kv_handler*) &(kv_handler){ "layer_index", gt_set_layer_index, NULL });
    g_hash_table_insert(ht, "timeout",           (gpointer)(kv_handler*) &(kv_handler){ "timeout", gt_set_timeout, NULL });
    g_hash_table_insert(ht, "foreground_color",  (gpointer)(kv_handler*) &(kv_handler){ "foreground_color", gt_set_foreground_color, NULL });
    g_hash_table_insert(ht, "background_color",  (gpointer)(kv_handler*) &(kv_handler){ "background_color", gt_set_background_color, NULL });
    g_hash_table_insert(ht, "low_warn_color",    (gpointer)(kv_handler*) &(kv_handler){ "low_warn_color", gt_set_low_warn_color, NULL });
    g_hash_table_insert(ht, "high_warn_color",   (gpointer)(kv_handler*) &(kv_handler){ "high_warn_color", gt_set_high_warn_color, NULL });
    g_hash_table_insert(ht, "id",                (gpointer)(kv_handler*) &(kv_handler){ "id", gt_set_id, NULL });
    g_hash_table_insert(ht, "output_format",     (gpointer)(kv_handler*) &(kv_handler){ "output_format", gt_set_output_format, NULL });
    g_hash_table_insert(ht, "units",             (gpointer)(kv_handler*) &(kv_handler){ "units", gt_set_units, NULL });
    g_hash_table_insert(ht, "pressure_units",    (gpointer)(kv_handler*) &(kv_handler){ "pressure_units", gt_set_pressure_units, NULL });
    g_hash_table_insert(ht, "temperature_units", (gpointer)(kv_handler*) &(kv_handler){ "temperature_units", gt_set_temperature_units, NULL });

    return ht;
}
#endif

/* NOTE:
 * The above uses compound literals stored in the hash table as pointers.
 * Those have static storage duration only if they are at file scope.
 * We are inside a function, so this is NOT safe.
 *
 * To keep this single-file and correct, we instead build from static arrays below.
 * (Leaving the above as a cautionary note; DO NOT USE it.)
 */

/* Correct, safe static tables: */
static const kv_handler SENSOR_TABLE[] = {
    {"name",            st_set_name,            NULL},
    {"can_data_offset", st_set_can_data_offset, NULL},
    {"can_data_width",  st_set_can_data_width,  NULL},
    {"column_index",    st_set_column_index,    NULL},
    {"row_index",       st_set_row_index,       NULL},
    {"can_id",          st_set_can_id,          NULL},
    {"scale",           st_set_scale,           NULL},
    {"offset",          st_set_offset,          NULL},
    {"id",              st_set_id,              NULL},
    {"x_values",        NULL,                   st_set_x_values_seq},
    {"y_values",        NULL,                  st_set_y_values_seq},
};

static const kv_handler PANEL_TABLE[] = {
    {"type",              gt_set_type,              NULL},
    {"low_warn",          gt_set_low_warn,          NULL},
    {"high_warn",         gt_set_high_warn,         NULL},
    {"min_value",         gt_set_min_value,         NULL},
    {"max_value",         gt_set_max_value,         NULL},
    {"label",             gt_set_label,             NULL},
    {"border",            gt_set_border,            NULL},
    {"column_index",      gt_set_column_index,      NULL},
    {"row_index",         gt_set_row_index,         NULL},
    {"layer_index",       gt_set_layer_index,       NULL},
    {"timeout",           gt_set_timeout,           NULL},
    {"foreground_color",  gt_set_foreground_color,  NULL},
    {"background_color",  gt_set_background_color,  NULL},
    {"low_warn_color",    gt_set_low_warn_color,    NULL},
    {"high_warn_color",   gt_set_high_warn_color,   NULL},
    {"id",                gt_set_id,                NULL},
    {"output_format",     gt_set_output_format,     NULL},
    {"units",             gt_set_units,             NULL},
    {"pressure_units",    gt_set_pressure_units,    NULL},
    {"temperature_units", gt_set_temperature_units, NULL},
};

static GHashTable *build_handlers_from_static(const kv_handler *t, size_t n)
{
    GHashTable *ht = g_hash_table_new(g_str_hash, g_str_equal);
    for (size_t i = 0; i < n; i++) {
        g_hash_table_insert(ht, (gpointer)t[i].key, (gpointer)&t[i]);
    }
    return ht;
}

static GHashTable *get_sensor_ht(void)
{
    static GHashTable *ht = NULL;
    if (!ht) ht = build_handlers_from_static(SENSOR_TABLE, G_N_ELEMENTS(SENSOR_TABLE));
    return ht;
}

static GHashTable *get_panel_ht(void)
{
    static GHashTable *ht = NULL;
    if (!ht) ht = build_handlers_from_static(PANEL_TABLE, G_N_ELEMENTS(PANEL_TABLE));
    return ht;
}

/* -------------------- configuration_load_yaml -------------------- */

Configuration* configuration_load_yaml (const char *path)
{
    Configuration* d = (Configuration*) calloc (1, sizeof (Configuration));

    FILE *f = fopen (path, "r");
    if (!f) {
        free (d);
        return NULL;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize (&parser);
    yaml_parser_set_input_file (&parser, f);

    char key[80] = {0};
    int in_sensors = 0;
    int in_panels = 0;

    SensorParameters st = {0};
    PanelParameters  gt = {0};

    gt.background_color = gt.foreground_color = gt.high_warn_color = gt.low_warn_color = 1;
    gt.type = UNKNOWN_PANEL;
    gt.output_format[0] = '\0';

    st.scale = st.offset = NAN;

    while (yaml_parser_parse (&parser, &event)) {

        if (event.type == YAML_SCALAR_EVENT) {
            const char *v = (const char *)event.data.scalar.value;

            if (!strcmp (v, "sensors")) in_sensors = 1, in_panels = 0;
            else if (!strcmp (v, "panels")) in_panels = 1, in_sensors = 0;
            else if (!key[0]) {
                strncpy (key, v, sizeof key - 1);
                key[sizeof key - 1] = '\0';
                normalize_key_inplace(key);
            } else {
                // scalar value for current key
                if (in_sensors) {
                    const kv_handler *h = (const kv_handler*)g_hash_table_lookup(get_sensor_ht(), key);
                    if (h && h -> scalar) {
                        h -> scalar(v, &st);
                    } else {
                        // unknown or wrong-type key; ignore
                        // fprintf(stderr, "unknown sensor key '%s'\n", key);
                    }
                } else if (in_panels) {
                    // tolerate "temperature units" typo from your sample
                    if (0 == strcmp(key, "temperature_units") || 0 == strcmp(key, "temperature_units")) {
                        // no-op, already normalized
                    }
                    const kv_handler *h = (const kv_handler*)g_hash_table_lookup(get_panel_ht(), key);
                    if (h && h -> scalar) {
                        h -> scalar(v, &gt);
                    } else {
                        // fprintf(stderr, "unknown panel key '%s'\n", key);
                    }
                }

                key[0] = 0;
            }
        }

        else if (event.type == YAML_SEQUENCE_START_EVENT) {
            // This is how libyaml represents x_values/y_values.
            // We only know what it is by looking at `key`.
            if (key[0]) {
                if (in_sensors) {
                    const kv_handler *h = (const kv_handler*)g_hash_table_lookup(get_sensor_ht(), key);
                    if (h && h -> seq) {
                        if (!h -> seq(&parser, &st)) {
                            fprintf(stderr, "failed to parse sequence for sensor key '%s'\n", key);
                        }
                    }
                } else if (in_panels) {
                    const kv_handler *h = (const kv_handler*)g_hash_table_lookup(get_panel_ht(), key);
                    if (h && h -> seq) {
                        if (!h -> seq(&parser, &gt)) {
                            fprintf(stderr, "failed to parse sequence for panel key '%s'\n", key);
                        }
                    }
                }
                key[0] = 0;
            }
            // NOTE: we do NOT delete the event here (we will at end of loop),
            // and we consumed until YAML_SEQUENCE_END_EVENT inside load_double_array().
        }

        if (event.type == YAML_MAPPING_END_EVENT) {
            if (in_sensors && st.name[0] && (0 != st.can_id)) {
                interpolation_array_sort (st.x_values, st.y_values, st.n_values);
                Sensor *s = sensor_create (&st);

                d -> sensors = realloc (d -> sensors, sizeof *d -> sensors * (d -> sensor_count + 1));
                d -> sensors[d -> sensor_count++] = *s;

                memset (&st, 0, sizeof st);
                st.scale = st.offset = NAN;
            }

            if (in_panels && gt.type != UNKNOWN_PANEL) {
                Panel *g = NULL;
                switch (gt.type) {
                case RADIAL_PRESSURE_PANEL:
                case RADIAL_TEMPERATURE_PANEL:
                    g = create_radial_gauge_panel (&gt);
                    break;

                case LINEAR_PRESSURE_PANEL:
                case LINEAR_TEMPERATURE_PANEL:
                    g = create_linear_gauge_panel (&gt);
                    break;

                case INFO_PANEL:
                    g = create_info_panel (&gt);
                    break;

                case TPMS_PANEL:
                    g = create_tpms_panel (&gt);
                    break;

                case GPS_PANEL:
                    g = create_gps_panel (&gt);
                    break;

                default:
                    fprintf (stderr, "unknown panel type\n");
                    break;
                }

                if (g) {
                    d -> panels = realloc (d -> panels, sizeof *d -> panels * (d -> panel_count + 1));
                    d -> panels[d -> panel_count++] = g;
                }

                memset (&gt, 0, sizeof gt);
                gt.background_color = gt.foreground_color = gt.high_warn_color = gt.low_warn_color = 1;
                gt.output_format[0] = '\0';
                gt.type = UNKNOWN_PANEL;
            }
        }

        if (event.type == YAML_NO_EVENT) {
            fclose (f);
            return d;
        }

        yaml_event_delete (&event);
        if (event.type == YAML_STREAM_END_EVENT) break;
    }

    yaml_parser_delete (&parser);
    fclose (f);
    return d;
}

static int
comp_panels (const void* elem1, const void* elem2) 
{
    const Panel* p1 = *(const Panel**) elem1;
    const Panel *p2 = *(const Panel **)elem2;
    
    if (p1 -> row_index > p2 -> row_index) {
        return 1;
    }
  
    if (p1 -> row_index < p2 -> row_index) {
        return -1;
    }
  
    if (p1 -> column_index > p2 -> column_index) {
        return 1;
    }
  
    if (p1 -> column_index < p2 -> column_index) {
        return -1;
    }
  
    if (p1 -> layer_index > p2 -> layer_index) {
        return 1;
    }
  
    if (p1 -> layer_index < p2 -> layer_index) {
        return -1;
    }

    return 0;
}

static int
comp_sensors (const void* elem1, const void* elem2) 
{
    const Sensor* s1 = (const Sensor*) elem1;
    const Sensor* s2 = (const Sensor*) elem2;

    if (s1 -> row_index > s2 -> row_index) {
        return 1;
    }
  
    if (s1 -> row_index < s2 -> row_index) {
        return -1;
    }
  
    if (s1 -> column_index > s2 -> column_index) {
        return 1;
    }
  
    if (s1 -> column_index < s2 -> column_index) {
        return -1;
    }
  
    if (s1 -> can_id > s2 -> can_id) {
        return 1;
    }
  
    if (s1 -> can_id < s2 -> can_id) {
        return -1;
    }
  
    if (s1 -> can_data_offset > s2 -> can_data_offset) {
        return 1;
    }
  
    if (s1 -> can_data_offset < s2 -> can_data_offset) {
        return -1;
    }
  
    if (s1 -> id > s2 -> id) {
        return 1;
    }
  
    if (s1 -> id < s2 -> id) {
        return -1;
    }

    return 0;
}

panel_group *linked_panel_group(const int i, const int j, Configuration *cfg) {
    panel_group *pg = cfg -> panel_groups;
    
    while (pg < (cfg -> panel_groups + cfg -> panel_group_count)) {
      if ((i == panel_get_row_index(*pg -> first)) &&
          (j == panel_get_column_index(*pg -> first))) {
          return (pg);
      }

      pg++;
    }

    fprintf (stderr, "unable to locate linked_panel_group\n");
    return (NULL);
}

void cfg_build_tables (Configuration* cfg) {
    int i;
    int j;
      
    qsort (cfg -> panels, cfg -> panel_count, sizeof(Panel*), comp_panels);
    cfg -> panel_groups = (panel_group *)calloc(cfg -> panel_count, sizeof(panel_group));
    cfg -> panel_group_count = 0;

    Panel **p = cfg -> panels;
    
    while (p < (cfg -> panels + cfg -> panel_count)) {
        i = panel_get_row_index(*p);
        j = panel_get_column_index (*p);

        cfg -> panel_groups[cfg -> panel_group_count].first
            = cfg -> panel_groups[cfg -> panel_group_count].last
            = cfg -> panel_groups[cfg -> panel_group_count].current = p;

        int ii;
        int jj;

        p = p + 1;
        
        while (p < (cfg -> panels + cfg -> panel_count)) {
            ii = panel_get_row_index (*p);
            jj = panel_get_column_index (*p);

            if ((i == ii) && (j == jj)) {
                cfg -> panel_groups[cfg -> panel_group_count].last++;
                p++;
            } else {
                p = p - 1;
                break;
            }
        }

        p++;
        cfg -> panel_group_count++;
    }
    
    // Could realloc here.

    qsort (cfg -> sensors, cfg -> sensor_count, sizeof (Sensor), comp_sensors);
    cfg -> sensor_groups = (sensor_group *)calloc(cfg -> sensor_count, sizeof(sensor_group));
    cfg -> sensor_group_count = 0;

    Sensor *s = cfg -> sensors;
    
    while (s < cfg -> sensors + cfg -> sensor_count) {
        i = sensor_get_row_index (s);
        j = sensor_get_column_index(s);
        
        cfg -> sensor_groups[cfg -> sensor_group_count].can_id = s -> can_id; 
        cfg -> sensor_groups[cfg -> sensor_group_count].linked_panel_group = linked_panel_group (i, j, cfg);
        cfg -> sensor_groups[cfg -> sensor_group_count].first
            = cfg -> sensor_groups[cfg -> sensor_group_count].last = s;

        int ii;
        int jj;

        s = s + 1;

        while (s < cfg -> sensors + cfg -> sensor_count) {
            ii = sensor_get_row_index (s);
            jj = sensor_get_column_index (s);

            if ((i == ii) && (j == jj) && (s -> can_id == cfg -> sensor_groups[cfg -> sensor_group_count].can_id)) {
                cfg -> sensor_groups[cfg -> sensor_group_count].last++;
                s++;
            } else {
                s = s - 1;
                break;
            }
        }

        s++;
        cfg -> sensor_group_count++;
    }
}

void cfg_free (Configuration *d) {
    for (int i = 0; i < d -> panel_count; i++)
        panel_destroy (d -> panels[i]);
    free (d -> panels);
    free (d -> sensors);
}
