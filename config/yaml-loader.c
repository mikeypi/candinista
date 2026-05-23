// yaml-loader.c  — tree-based rewrite using yaml_document_t
//
// Instead of a hand-rolled state machine we:
//   1. Load the whole document into a yaml_document_t node tree.
//   2. Walk the top-level mapping to find "sensors" and "panels".
//   3. Iterate the sequence under each key and map each entry's
//      key/value pairs directly onto SensorParameters / PanelParameters.
//
// Helper convention:
//   node_key(doc, n)   — returns the char* of a YAML_SCALAR_NODE
//   mapping_lookup(doc, map_node, "key") — finds a value node by key name

#include <yaml.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "units.h"
#include "cairo-misc.h"
#include "sensor.h"
#include "panel.h"
#include "yaml-loader.h"
#include "candinista.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Safe scalar value from a node (returns "" if node is wrong type). */
static const char *
node_str (yaml_document_t *doc, yaml_node_t *n)
{
    if (!n || n -> type != YAML_SCALAR_NODE) return "";
    return (const char *)n -> data.scalar.value;
}

/* Look up a key in a YAML_MAPPING_NODE; returns the value node or NULL. */
static yaml_node_t *
mapping_lookup (yaml_document_t *doc, yaml_node_t *map, const char *key)
{
    if (!map || map -> type != YAML_MAPPING_NODE) return NULL;

    yaml_node_pair_t *pair;
    for (pair = map -> data.mapping.pairs.start;
         pair < map -> data.mapping.pairs.top;
         pair++)
    {
        yaml_node_t *k = yaml_document_get_node(doc, pair -> key);
        if (k && k -> type == YAML_SCALAR_NODE) {
            if (0 == strcmp((const char *)k -> data.scalar.value, key))
                return yaml_document_get_node(doc, pair -> value);
        }
    }
    return NULL;
}

/* Read a YAML_SEQUENCE_NODE of scalars into a freshly-malloc'd double array. */
static int
read_double_sequence (yaml_document_t *doc, yaml_node_t *seq,
                      double **out, size_t *count)
{
    *out = NULL;
    *count = 0;

    if (!seq || seq -> type != YAML_SEQUENCE_NODE) return 0;

    size_t n = (size_t)(seq -> data.sequence.items.top -
                        seq -> data.sequence.items.start);
    if (n == 0) return 1;

    *out = malloc(n * sizeof(double));
    if (!*out) return 0;

    yaml_node_item_t *item;
    for (item = seq -> data.sequence.items.start;
         item < seq -> data.sequence.items.top;
         item++)
    {
        yaml_node_t *v = yaml_document_get_node(doc, *item);
        (*out)[(*count)++] = atof(node_str(doc, v));
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Enum helpers (unchanged logic, just moved here)                     */
/* ------------------------------------------------------------------ */

static panel_type
enum_from_type_str (const char *temp)
{
    char buffer[80];
    size_t i;

    if (strlen(temp) > 79) return UNKNOWN_PANEL;
    for (i = 0; i < strlen(temp); i++)
        buffer[i] = (char)tolower((unsigned char)temp[i]);
    buffer[i] = '\0';

    if (0 == strcmp(buffer, "radial_pressure"))    return RADIAL_PRESSURE_PANEL;
    if (0 == strcmp(buffer, "linear_pressure"))    return LINEAR_PRESSURE_PANEL;
    if (0 == strcmp(buffer, "radial_temperature")) return RADIAL_TEMPERATURE_PANEL;
    if (0 == strcmp(buffer, "linear_temperature")) return LINEAR_TEMPERATURE_PANEL;
    if (0 == strcmp(buffer, "info"))               return INFO_PANEL;
    if (0 == strcmp(buffer, "tpms"))               return TPMS_PANEL;
    if (0 == strcmp(buffer, "gps"))                return GPS_PANEL;
    if (0 == strcmp(buffer, "text"))               return TEXT_PANEL;

    fprintf(stderr, "unknown panel type '%s'\n", temp);
    return UNKNOWN_PANEL;
}

/* ------------------------------------------------------------------ */
/* Parse one sensor mapping node -> SensorParameters                   */
/* ------------------------------------------------------------------ */

static int
parse_sensor (yaml_document_t *doc, yaml_node_t *map, SensorParameters *st)
{
    memset(st, 0, sizeof *st);
    st -> scale  = NAN;
    st -> offset = NAN;

    if (!map || map -> type != YAML_MAPPING_NODE) return 0;

    yaml_node_pair_t *pair;
    for (pair = map -> data.mapping.pairs.start;
         pair < map -> data.mapping.pairs.top;
         pair++)
    {
        yaml_node_t *kn = yaml_document_get_node(doc, pair -> key);
        yaml_node_t *vn = yaml_document_get_node(doc, pair -> value);
        const char  *k  = node_str(doc, kn);
        const char  *v  = node_str(doc, vn);   /* empty for sequences */

        if      (0 == strcmp(k, "name"))            { strncpy(st -> name, v, 63); st -> name[63] = '\0'; }
        else if (0 == strcmp(k, "can_data_offset")) { st -> can_data_offset = atoi(v); }
        else if (0 == strcmp(k, "can_data_width"))  { st -> can_data_width  = atoi(v); }
        else if (0 == strcmp(k, "column_index"))    { st -> column_index    = atoi(v); }
        else if (0 == strcmp(k, "row_index"))       { st -> row_index       = atoi(v); }
        else if (0 == strcmp(k, "can_id"))          { st -> can_id          = strtol(v, NULL, 16); }
        else if (0 == strcmp(k, "scale"))           { st -> scale           = atof(v); }
        else if (0 == strcmp(k, "offset"))          { st -> offset          = atof(v); }
        else if (0 == strcmp(k, "id"))              { st -> id              = atoi(v); }
        else if (0 == strcmp(k, "x_values")) {
            read_double_sequence(doc, vn, &st -> x_values, &st -> n_values);
        }
        else if (0 == strcmp(k, "y_values")) {
            size_t dummy;
            read_double_sequence(doc, vn, &st -> y_values, &dummy);
        }
        else {
            fprintf(stderr, "unknown sensor key '%s' — ignored\n", k);
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* Parse one panel mapping node -> PanelParameters                     */
/* ------------------------------------------------------------------ */

static int
parse_panel (yaml_document_t *doc, yaml_node_t *map, PanelParameters *gt)
{
    memset(gt, 0, sizeof *gt);
    gt -> background_color = gt -> foreground_color =
    gt -> high_warn_color  = gt -> low_warn_color   = 1;
    gt -> type = UNKNOWN_PANEL;
    gt -> output_format[0] = '\0';

    if (!map || map -> type != YAML_MAPPING_NODE) return 0;

    yaml_node_pair_t *pair;
    for (pair = map -> data.mapping.pairs.start;
         pair < map -> data.mapping.pairs.top;
         pair++)
    {
        yaml_node_t *kn = yaml_document_get_node(doc, pair -> key);
        yaml_node_t *vn = yaml_document_get_node(doc, pair -> value);
        const char  *k  = node_str(doc, kn);
        const char  *v  = node_str(doc, vn);

        if      (0 == strcmp(k, "type"))              { gt -> type             = enum_from_type_str(v); }
        else if (0 == strcmp(k, "low_warn"))          { gt -> low_warn         = atof(v); }
        else if (0 == strcmp(k, "high_warn"))         { gt -> high_warn        = atof(v); }
        else if (0 == strcmp(k, "min_value"))         { gt -> min              = atof(v); }
        else if (0 == strcmp(k, "max_value"))         { gt -> max              = atof(v); }
        else if (0 == strcmp(k, "label"))             { strncpy(gt -> label, v, 63); gt -> label[63] = '\0'; }
        else if (0 == strcmp(k, "border"))            { gt -> border           = atoi(v); }
        else if (0 == strcmp(k, "column_index"))      { gt -> column_index     = atoi(v); }
        else if (0 == strcmp(k, "row_index"))         { gt -> row_index        = atoi(v); }
        else if (0 == strcmp(k, "layer_index"))       { gt -> layer_index      = atoi(v); }
        else if (0 == strcmp(k, "timeout"))           { gt -> timeout          = atoi(v); }
        else if (0 == strcmp(k, "foreground_color"))  { gt -> foreground_color = strtol(v, NULL, 16); }
        else if (0 == strcmp(k, "background_color"))  { gt -> background_color = strtol(v, NULL, 16); }
        else if (0 == strcmp(k, "low_warn_color"))    { gt -> low_warn_color   = strtol(v, NULL, 16); }
        else if (0 == strcmp(k, "high_warn_color"))   { gt -> high_warn_color  = strtol(v, NULL, 16); }
        else if (0 == strcmp(k, "id"))                { gt -> id               = atoi(v); }
        else if (0 == strcmp(k, "output_format"))     { strncpy(gt -> output_format, v, 63); gt -> output_format[63] = '\0'; }
        else if (0 == strcmp(k, "units"))             { gt -> units            = enum_from_unit_str(v); }
        else if (0 == strcmp(k, "pressure_units"))    { gt -> pressure_units   = enum_from_unit_str(v); }
        else if (0 == strcmp(k, "temperature_units")) { gt -> temperature_units= enum_from_unit_str(v); }
        else {
            fprintf(stderr, "unknown panel key '%s' — ignored\n", k);
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/* configuration_load_yaml                                             */
/* ------------------------------------------------------------------ */

Configuration *
configuration_load_yaml (const char *path)
{
    Configuration *d = calloc(1, sizeof(Configuration));
    if (!d) return NULL;

    FILE *f = fopen(path, "r");
    if (!f) { free(d); return NULL; }

    yaml_parser_t   parser;
    yaml_document_t document;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, f);

    if (!yaml_parser_load(&parser, &document)) {
        fprintf(stderr, "yaml parse error: %s at line %zu col %zu\n",
                parser.problem,
                parser.problem_mark.line   + 1,
                parser.problem_mark.column + 1);
        yaml_parser_delete(&parser);
        fclose(f);
        free(d);
        return NULL;
    }

    yaml_parser_delete(&parser);
    fclose(f);

    /* The root node must be a mapping. */
    yaml_node_t *root = yaml_document_get_root_node(&document);
    if (!root || root -> type != YAML_MAPPING_NODE) {
        fprintf(stderr, "yaml: root is not a mapping\n");
        yaml_document_delete(&document);
        free(d);
        return NULL;
    }

    /* ---- sensors ---- */
    yaml_node_t *sensors_node = mapping_lookup(&document, root, "sensors");
    if (sensors_node && sensors_node -> type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item  = sensors_node -> data.sequence.items.start;
             item  < sensors_node -> data.sequence.items.top;
             item++)
        {
            yaml_node_t    *entry = yaml_document_get_node(&document, *item);
            SensorParameters st;

            if (!parse_sensor(&document, entry, &st)) continue;
            if (!st.name[0] || st.can_id == 0)        continue;

            interpolation_array_sort(st.x_values, st.y_values, st.n_values);
            Sensor *s = sensor_create(&st);

            d -> sensors = realloc(d -> sensors,
                                 sizeof(*d -> sensors) * (d -> sensor_count + 1));
            d -> sensors[d -> sensor_count++] = *s;
        }
    }

    /* ---- panels ---- */
    yaml_node_t *panels_node = mapping_lookup(&document, root, "panels");
    if (panels_node && panels_node -> type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item  = panels_node -> data.sequence.items.start;
             item  < panels_node -> data.sequence.items.top;
             item++)
        {
            yaml_node_t   *entry = yaml_document_get_node(&document, *item);
            PanelParameters gt;

            if (!parse_panel(&document, entry, &gt)) continue;
            if (gt.type == UNKNOWN_PANEL)             continue;

            Panel *g = NULL;
            switch (gt.type) {
            case RADIAL_PRESSURE_PANEL:
            case RADIAL_TEMPERATURE_PANEL:  g = create_radial_gauge_panel(&gt); break;
            case LINEAR_PRESSURE_PANEL:
            case LINEAR_TEMPERATURE_PANEL:  g = create_linear_gauge_panel(&gt); break;
            case INFO_PANEL:                g = create_info_panel(&gt);         break;
            case TPMS_PANEL:                g = create_tpms_panel(&gt);         break;
            case GPS_PANEL:                 g = create_gps_panel(&gt);          break;
            case TEXT_PANEL:                g = create_text_panel(&gt);         break;
            default:
                fprintf(stderr, "unhandled panel type %d\n", gt.type);
                break;
            }

            if (g) {
                d -> panels = realloc(d -> panels,
                                    sizeof(*d -> panels) * (d -> panel_count + 1));
                d -> panels[d -> panel_count++] = g;
            }
        }
    }

    yaml_document_delete(&document);
    return d;
}

/* ------------------------------------------------------------------ */
/* Everything below is unchanged from original                         */
/* ------------------------------------------------------------------ */

static int
comp_panels (const void *elem1, const void *elem2)
{
    const Panel *p1 = *(const Panel **)elem1;
    const Panel *p2 = *(const Panel **)elem2;

    if (p1 -> row_index    != p2 -> row_index)    return p1 -> row_index    > p2 -> row_index    ? 1 : -1;
    if (p1 -> column_index != p2 -> column_index) return p1 -> column_index > p2 -> column_index ? 1 : -1;
    if (p1 -> layer_index  != p2 -> layer_index)  return p1 -> layer_index  > p2 -> layer_index  ? 1 : -1;
    return 0;
}

static int
comp_sensors (const void *elem1, const void *elem2)
{
    const Sensor *s1 = (const Sensor *)elem1;
    const Sensor *s2 = (const Sensor *)elem2;

    if (s1 -> row_index       != s2 -> row_index)       return s1 -> row_index       > s2 -> row_index       ? 1 : -1;
    if (s1 -> column_index    != s2 -> column_index)     return s1 -> column_index    > s2 -> column_index    ? 1 : -1;
    if (s1 -> can_id          != s2 -> can_id)           return s1 -> can_id          > s2 -> can_id          ? 1 : -1;
    if (s1 -> can_data_offset != s2 -> can_data_offset)  return s1 -> can_data_offset > s2 -> can_data_offset ? 1 : -1;
    if (s1 -> id              != s2 -> id)               return s1 -> id              > s2 -> id              ? 1 : -1;
    return 0;
}

panel_group *
linked_panel_group (const int i, const int j, Configuration *cfg)
{
    panel_group *pg = cfg -> panel_groups;
    while (pg < cfg -> panel_groups + cfg -> panel_group_count) {
        if (i == panel_get_row_index(*pg -> first) &&
            j == panel_get_column_index(*pg -> first))
            return pg;
        pg++;
    }
    return NULL;
}

void
cfg_build_tables (Configuration *cfg)
{
    qsort(cfg -> panels, cfg -> panel_count, sizeof(Panel *), comp_panels);
    cfg -> panel_groups      = calloc(cfg -> panel_count, sizeof(panel_group));
    cfg -> panel_group_count = 0;

    Panel **p = cfg -> panels;
    while (p < cfg -> panels + cfg -> panel_count) {
        int i = panel_get_row_index(*p);
        int j = panel_get_column_index(*p);

        cfg -> panel_groups[cfg -> panel_group_count].first   =
        cfg -> panel_groups[cfg -> panel_group_count].last    =
        cfg -> panel_groups[cfg -> panel_group_count].current = p;

        p++;

        while (p < cfg -> panels + cfg -> panel_count) {
            if (i == panel_get_row_index(*p) &&
                j == panel_get_column_index(*p)) {
                cfg -> panel_groups[cfg -> panel_group_count].last++;
                p++;
            } else {
                p--;
                break;
            }
        }
        p++;
        cfg -> panel_group_count++;
    }

    qsort(cfg -> sensors, cfg -> sensor_count, sizeof(Sensor), comp_sensors);
    cfg -> sensor_groups      = calloc(cfg -> sensor_count, sizeof(sensor_group));
    cfg -> sensor_group_count = 0;

    Sensor *s = cfg -> sensors;
    while (s < cfg -> sensors + cfg -> sensor_count) {
        int i = sensor_get_row_index(s);
        int j = sensor_get_column_index(s);

        cfg -> sensor_groups[cfg -> sensor_group_count].can_id = s -> can_id;
        cfg -> sensor_groups[cfg -> sensor_group_count].linked_panel_group =
            linked_panel_group(i, j, cfg);

        if (!cfg -> sensor_groups[cfg -> sensor_group_count].linked_panel_group) {
            fprintf(stderr, "unable to find linked panel group for sensor %s\n", s -> name);
        }
        
        cfg -> sensor_groups[cfg -> sensor_group_count].first =
        cfg -> sensor_groups[cfg -> sensor_group_count].last  = s;

        s++;
        while (s < cfg -> sensors + cfg -> sensor_count) {
            if (sensor_get_row_index(s)    == i &&
                sensor_get_column_index(s) == j &&
                s -> can_id == cfg -> sensor_groups[cfg -> sensor_group_count].can_id) {
                cfg -> sensor_groups[cfg -> sensor_group_count].last++;
                s++;
            } else {
                s--;
                break;
            }
        }
        s++;
        cfg -> sensor_group_count++;
    }
}

void
cfg_free (Configuration *d)
{
    for (int i = 0; i < d -> panel_count; i++)
        panel_destroy(d -> panels[i]);
    free(d -> panels);
    free(d -> sensors);
}
