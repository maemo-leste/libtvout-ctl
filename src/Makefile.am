lib_LTLIBRARIES = \
	libtvout-ctl.la

libtvout_ctl_la_CFLAGS = \
	@BACKEND_CFLAGS@

libtvout_ctl_la_LIBADD = \
	@BACKEND_LIBS@

libtvout_ctl_la_LDFLAGS = \
	-version-info 0:0:0

if BACKEND_XV
libtvout_ctl_la_SOURCES = \
	tvout-ctl-xv.c
else
libtvout_ctl_la_SOURCES = \
	tvout-ctl-xrandr.c
endif

include_HEADERS = \
	tvout-ctl.h
