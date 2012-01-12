#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libtvout-ctl], [1], [])
AM_INIT_AUTOMAKE([])
AC_CONFIG_SRCDIR([src/tvout-ctl.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_MSG_CHECKING([which backend to build])
AC_ARG_WITH([backend],
	AC_HELP_STRING([--with-backend=BACKEND],
		[select the backend for the library. Possible]
		[backends are: xv, xrandr. @<:@default=xv@:>@]),
	[backend="$withval"], [backend=xv])
case x$backend in
	xxv)
		AC_MSG_RESULT([$backend])
		BACKEND_MODULES="x11 xv"
		;;
	xxrandr)
		AC_MSG_RESULT([$backend])
		BACKEND_MODULES="x11 xrandr"
		;;
	*)
		AC_MSG_ERROR([invalid backend '$backend'])
		;;
esac

PKG_CHECK_MODULES([BACKEND],[$BACKEND_MODULES])

AM_CONDITIONAL([BACKEND_XV], [test x$backend = xxv])
AM_CONDITIONAL([BACKEND_XRANDR], [test x$backend = xxrandr])

AC_SUBST([BACKEND_MODULES])

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 tvout-ctl.pc])
AC_OUTPUT
