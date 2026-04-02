#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([SDL2 Game Controller Test], [0.1], [])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AC_CANONICAL_HOST

DEVKITPRO_WUT_OPT_INIT

AM_INIT_AUTOMAKE([foreign subdir-objects])

LT_INIT

AC_PROG_CXX
AX_APPEND_COMPILE_FLAGS([-std=c++23], [CXX])
AC_LANG([C++])

AS_VAR_IF([enable_wiiu], [yes],
          [
              AC_MSG_NOTICE([building Wii U app])
              DEVKITPRO_WUT_SETUP
          ])
AM_CONDITIONAL([ENABLE_WIIU], [test x$enable_wiiu = xyes])


AX_PTHREAD([
                AX_APPEND_FLAG([$PTHREAD_CFLAGS], [CFLAGS])
                AX_APPEND_FLAG([$PTHREAD_CFLAGS], [CXXFLAGS])
                AX_APPEND_FLAG([$PTHREAD_LIBS],   [LIBS])
           ])

PKG_CHECK_MODULES([FT], [freetype2])
PKG_CHECK_MODULES([SDL], [sdl2])


AX_SUBDIRS_CONFIGURE([external/sdl2xx],
                     [[--disable-examples],
                      [--disable-mixer],
                      [--disable-image],
                      [--disable-ttf],
                      [CPPFLAGS=$CPPFLAGS],
                      [CFLAGS=$CFLAGS],
                      [CXXFLAGS=$CXXFLAGS],
                      [LDFLAGS=$LDFLAGS],
                      [LIBS=$LIBS],
                      [PATH=$PATH]])


AC_CONFIG_FILES([Makefile
                 external/Makefile])
AC_OUTPUT
