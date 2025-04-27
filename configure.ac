#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([SDL2 Game Controller Test], [0.1], [])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_RANLIB

AC_PROG_CXX
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])
AC_LANG([C++])

PKG_CHECK_MODULES([SDL2], [sdl2])


AX_SUBDIRS_CONFIGURE([external/sdl2xx],
                     [[--disable-examples],[--disable-mixer],[--disable-image],[--disable-ttf]])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
