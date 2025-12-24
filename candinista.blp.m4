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

Window window {
  default-height: -1;
  default-width: -1;
  halign: start;
  height-request: 600;
  width-request: 1024;
  resizable: false;
  
  Grid {
    column-homogeneous: true;
    column-spacing: 15;
    row-spacing: 15;

    gridbox(box-0, label-0, value-0, 1, 0)
    gridbox(box-1, label-1, value-1, 1, 1)
    gridbox(box-2, label-2, value-2, 1, 2)
    gridbox(box-3, label-3, value-3, 2, 0)
    gridbox(box-4, label-4, value-4, 2, 1)
    gridbox(box-5, label-5, value-5, 2, 2)

    Box box-6 {
      valign: baseline;
      layout {
      column: 0;
      column-span: 3;
      row: 0;
      row-span: 1;
      }
      Label value-6 {
    label: "time";
	valign: center;
	halign: end;
	hexpand: true;
	hexpand-set: true;
	margin-end: 10;
        ellipsize: middle;
        styles["time-style"]
      }
      styles["box-style"]
    }

    styles["grid-style"]
    }

  styles["window-style"]
}
