using Gtk 4.0;

define(`gridbox', `
      Box $1 {
      hexpand: true;
      hexpand-set: true;
      orientation: vertical;
      valign: baseline;
      vexpand: true;
      vexpand-set: true;
      layout {
        column: $5;
        column-span: 1;
        row: $4;
        row-span: 1;
      }
      Label $2 {
	halign: start;
	margin-start: 5;
	margin-top: 5;
	valign: start;
	vexpand: true;
	vexpand-set: true;
        styles["label-style"]
      }
      Label $3 {
	halign: end;
	margin-bottom: 5;
	margin-end: 5;
	valign: end;
	vexpand: true;
	vexpand-set: true;
        styles["value-style"]
        }
        styles["box-style"]
      }
')


define(`grid_drawing_area', `
      Gtk.DrawingArea $1 {
      hexpand: true;
      vexpand: true;
      layout {
        row: $2;
        row-span: 1;
        column: $3;
        column-span: 1;
        }
      styles["drawing-area-style"]

      }
')


Window window {
  default-height: 600;
  default-width: 1024;
  hexpand: true;
  vexpand: true;
  
  Grid grid-0 {
    hexpand: true;
    vexpand: true;
    column-homogeneous: true;
    grid_drawing_area(da-0-0, 0, 0)
    grid_drawing_area(da-0-1, 0, 1)
    grid_drawing_area(da-0-2, 0, 2)
    grid_drawing_area(da-1-0, 1, 0)
    grid_drawing_area(da-1-1, 1, 1)
    grid_drawing_area(da-1-2, 1, 2)
    styles["grid-style"]
    }

  styles["window-style"]
}
