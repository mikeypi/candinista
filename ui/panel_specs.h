#ifndef PANEL_SPECS_H
#define PANEL_SPECS_H

typedef struct
{
  double arc_start_angle;
  double arc_end_angle;
  int illuminated;
} arc_segment;

typedef struct {
  Panel base;
  double value;
  double min;
  double max;
  unit_type units;
  char label[64];
  char* output_format;
  
  double radius;
  double start_angle;
  double end_angle;

  int segment_count;
  arc_segment* arc_segments;
  double segment_gap_size;
} RadialPanel;

typedef struct
{
  double min;
  double max;
  double start_x;
  double start_y;
  double height;
  double width;
  int illuminated;
} bargraph_segment;

/* concrete type */
typedef struct {
  Panel base;
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  int high_warn_color;
  int low_warn_color;
  unit_type units;
  char label[64];
  char* output_format;  

  double bargraph_origin_x;
  double bargraph_origin_y;
  double bargraph_width;
  double bargraph_height;
  int bargraph_segment_count;
  bargraph_segment* bargraph_segments;
} LinearPanel;

/* concrete type */
typedef struct {
  Panel base;
} InfoPanel;

/* concrete type */
typedef struct {
  Panel base;
  double lattitude;
  double longitude;
  double speed;
  double altitude;
  double heading_motion;
  double vehicle_motion;
  double x_acceleration;
  double y_acceleration;
  double z_acceleration;
  int sat_count;
  int gps_status;
  int utc_year;
  int utc_month;
  int utc_day;
  int utc_hour;
  int utc_minute;
  int utc_second;
  unit_type speed_units;
  unit_type altitude_units;
} GPSPanel;

/* concrete type */
typedef struct {
  Panel base;
  int multiplexor;
  double pressure[4];
  double temperature[4];
  double voltage[4];
  int sign[4];
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;
  unit_type pressure_units;
  unit_type temperature_units;
  char label[64];
  char* output_format;  
} TPMSPanel;

#endif
