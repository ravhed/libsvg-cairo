lib_LTLIBRARIES = libsvg-cairo.la
include_HEADERS = svg-cairo.h

libsvg_cairo_la_SOURCES = \
	svg_cairo.c \
	svg-cairo.h \
	svg-cairo-internal.h \
	svg_cairo_sprintf_alloc.c \
	svg_cairo_state.c

libsvg_cairo_la_LDFLAGS = -version-info @VERSION_INFO@

INCLUDES = $(WARN_CFLAGS) $(PANGOCAIRO_CFLAGS) $(LIBSVG_CAIRO_CFLAGS) $(X_CFLAGS)

libsvg_cairo_la_LIBADD = $(PANGOCAIRO_LIBS) $(LIBSVG_CAIRO_LIBS) $(X_LIBS)
