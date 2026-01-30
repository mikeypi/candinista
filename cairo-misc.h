#ifndef CAIROMISC_H
#define CAIROMISC

typedef enum warning_level {NO_WARN, LOW_WARN, HIGH_WARN} warning_level;

#define XRED_RGB    0xff0808
#define XBLUE_RGB   0xe0e0e0
#define XORANGE_RGB 0xffa600
#define XBLACK_RGB  0x000000
#define XGRAY_RGB   0x333333

#define DEFAULT_HIGH_WARN_RGB XRED_RGB
#define DEFAULT_LOW_WARN_RGB XBLUE_RGB
#define DEFAULT_FOREGROUND_RGB XORANGE_RGB
#define DEFAULT_BACKGROUND_RGB XBLACK_RGB

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

extern void draw_cairo_info_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

extern gboolean update_cairo_gauge_panel_value (gpointer);
extern void draw_cairo_gauge_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

extern gboolean update_cairo_bargraph_panel_value (gpointer);
extern void draw_cairo_bargraph_panel (GtkDrawingArea*, cairo_t*, int, int, gpointer);

extern void set_rgba_for_burn_in (cairo_t*, warning_level);
extern void set_rgba_for_foreground (cairo_t*, warning_level);
extern void set_rgba_for_background (cairo_t*);
extern double show_text_unjustified (cairo_t*,  int, int, char*);
extern double show_text_left_justified (cairo_t*, int, int, char*);
extern double show_text_right_justified (cairo_t*, int, int, char*, int);
extern int show_text_burn_in (cairo_t* cr, int x, int y, char* buffer, int width);
extern int show_text_box (cairo_t* cr, int x, int y, char* buffer, int width);

extern void rounded_rectangle(cairo_t*, double, double, double, double, double);

extern void set_rgba (cairo_t* cr, unsigned int color, double alpha);
#endif
