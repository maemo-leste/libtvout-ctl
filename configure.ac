#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([tvout-home-plugin], [3], [])
AM_INIT_AUTOMAKE([])
AC_CONFIG_SRCDIR([src/tvout-home-plugin.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([HILDON],[libhildondesktop-1])
PKG_CHECK_MODULES([XV],[x11 xv])

homedesktopdir=`$PKG_CONFIG --variable=hildonhomedesktopentrydir libhildondesktop-1`
AC_SUBST(homedesktopdir)

homeplugindir=`$PKG_CONFIG --variable=hildondesktoplibdir libhildondesktop-1`
AC_SUBST(homeplugindir)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
