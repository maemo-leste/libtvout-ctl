#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libtvout-ctl], [1], [])
AM_INIT_AUTOMAKE([])
AC_CONFIG_SRCDIR([src/tvout-ctl.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([XV],[x11 xv])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
