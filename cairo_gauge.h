/* Simple gauge widget implemented with GtkDrawingArea + Cairo (GTK4)
 * Compile with:
 * cc gauge.c `pkg-config --cflags --libs gtk4` -lm -o gauge
 */

typedef struct
{
  double arc_start_angle;
  double arc_end_angle;
  int illuminated;
} arc_segment;


typedef struct
{
  double radius;
  double start_angle;
  double end_angle;

  int segment_count;
  arc_segment* arc_segments;
  double segment_gap_size;
  
  double value;
  double min;
  double max;
  double low_warn;
  double high_warn;

  char* label;
  char* legend;
} cairo_gauge;


#define RED 0xff/255.0, 0x0/255.0, 0x0/255.0
#define ORANGE 0xff/255.0, 0xa6/255.0, 0x0/255.0
#define BLACK 0x0/255.0, 0x0/255.0, 0x0/255.0
#define GRAY 0x33/255.0, 0x33/255.0, 0x33/255.0

//#define WARN RED, 0.9
//#define FOREGROUND BLACK, 0.75
//#define BACKGROUND ORANGE, 1.0
//#define BURNIN BLACK, 0.07

#define WARN RED, 0.9
#define FOREGROUND ORANGE, 0.9
#define BACKGROUND BLACK, 1.0
#define BURNIN ORANGE, 0.14
#define WARN_BURNIN RED, 0.14

extern void 
draw_cairo_gauge (GtkDrawingArea*,
	 cairo_t*,
         int,
         int,
	 gpointer);

#define DEFAULT_GAUGE_HEIGHT 180
#define DEFAULT_GAUGE_WIDTH 180
#define DEFAULT_START_ANGLE (4 * M_PI / 4)
#define DEFAULT_END_ANGLE (9 * M_PI / 4)
#define DEFAULT_SEGMENT_COUNT (20)
#define DEFAULT_SEGMENT_GAP_SIZE (0.03)
#define DEFAULT_RADIUS (MIN(DEFAULT_GAUGE_HEIGHT - 20, DEFAULT_GAUGE_WIDTH - 20) * 0.70)
#define DEFAULT_LABEL_FONT_SIZE 30
//#define DEFAULT_VALUE_FONT_SIZE 75
#define DEFAULT_VALUE_FONT_SIZE 65

extern cairo_gauge* new_cairo_gauge ();
extern gboolean update_cairo_gauge_value (gpointer);
