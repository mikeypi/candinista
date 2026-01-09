typedef enum warning_level {NO_WARN = 6, LOW_WARN = 7, HIGH_WARN = 8} warning_level;

#define RED_RGB 255/255.0, 8/255.0, 8/255.0
#define BLUE_RGB 224/255.0, 224/255.0, 224/255.0
#define ORANGE_RGB 0xff/255.0, 0xa6/255.0, 0x0/255.0
#define BLACK_RGB 0x0/255.0, 0x0/255.0, 0x0/255.0
#define GRAY_RGB 0x33/255.0, 0x33/255.0, 0x33/255.0

//#define WARN_RGBA RED_RGB, 0.9
//#define FOREGROUND_RGBA BLACK_RGB, 0.75
//#define BACKGROUND_RGBA ORANGE_RGB, 1.0
//#define BURN_IN_RGBA BLACK_RGB, 0.07

#define HIGH_WARN_RGBA RED_RGB, 0.9
#define LOW_WARN_RGBA BLUE_RGB, 0.9
#define FOREGROUND_RGBA ORANGE_RGB, 0.9
#define BACKGROUND_RGBA BLACK_RGB, 1.0
#define BURN_IN_RGBA ORANGE_RGB, 0.14
#define HIGH_WARN_BURN_IN_RGBA RED_RGB, 0.14
#define LOW_WARN_BURN_IN_RGBA BLUE_RGB, 0.14

#define DEFAULT_START_ANGLE (4 * M_PI / 4)
#define DEFAULT_END_ANGLE (9 * M_PI / 4)
#define DEFAULT_SEGMENT_COUNT (20)
#define DEFAULT_SEGMENT_GAP_SIZE (0.03)
#define DEFAULT_RADIUS 120
#define DEFAULT_LABEL_FONT_SIZE 30
#define DEFAULT_VALUE_FONT_SIZE 65

#define DEFAULT_BARGRAPH_ORIGIN_X 20
#define DEFAULT_BARGRAPH_ORIGIN_Y 15
#define DEFAULT_BARGRAPH_WIDTH 300
#define DEFAULT_BARGRAPH_HEIGHT 160
#define DEFAULT_BARGRPAH_SEGMENT_COUNT 16


//extern cairo_info_panel* new_cairo_info_panel ();
extern void draw_cairo_info_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

//extern cairo_gauge_panel* new_cairo_gauge_panel ();
extern gboolean update_cairo_gauge_panel_value (gpointer);
extern void draw_cairo_gauge_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

//extern cairo_bargraph_panel* new_cairo_bargraph_panel ();
extern gboolean update_cairo_bargraph_panel_value (gpointer);
extern void draw_cairo_bargraph_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

extern void set_rgba_for_burn_in (cairo_t*, warning_level);
extern void set_rgba_for_foreground (cairo_t*, warning_level);
extern void set_rgba_for_background (cairo_t*);
extern void show_text_unjustified (cairo_t*,  int, int, char*);
extern void show_text_left_justified (cairo_t*, int, int, char*);
extern int show_text_right_justified (cairo_t*, int, int, char*, int, warning_level, bool, bool);
extern void rounded_rectangle(cairo_t*, double, double, double, double, double);

extern warning_level get_warning_level (double, double, double);
