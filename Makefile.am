AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tvout-ctl.pc

install-exec-hook:
	find $(DESTDIR)$(libdir) -type f -name \*.la -delete
