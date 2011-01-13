homeplugindir = @homeplugindir@

homeplugin_LTLIBRARIES = \
	tvout-home-plugin.la

tvout_home_plugin_la_CFLAGS = \
	@HILDON_CFLAGS@ \
	@XV_CFLAGS@

tvout_home_plugin_la_LIBADD = \
	@HILDON_LIBS@ \
	@XV_LIBS@

tvout_home_plugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-shared

tvout_home_plugin_la_SOURCES = \
	tvout-ctl.c \
	tvout-ctl.h \
	tvout-home-plugin.c \
	tvout-home-plugin.h
