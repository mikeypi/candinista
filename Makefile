CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk4 json-c)
LIBS = $(shell $(PKGCONFIG) --libs gtk4 json-c)
#GLIB_COMPILE_RESOURCES = $(shell $(PKGCONFIG) --variable=glib_compile_resources gio-2.0)
#GLIB_COMPILE_SCHEMAS = $(shell $(PKGCONFIG) --variable=glib_compile_schemas gio-2.0)

SRC = candinista.c datalogging.c interpolate.c environ.c readjson.c printjson.c


OBJS =  $(SRC:.c=.o)

BACKUPS = $(SRC:.c=.c~) $(SRC:.h=.h~)

all: candinista

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

candinista: $(OBJS) 
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -f candinista




