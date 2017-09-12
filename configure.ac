AC_PREREQ([2.69])
AC_INIT([nano-video], [0.1], [fatwildcat@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/nanovideo.c])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
# GLIB_GSETTINGS - not yet

# Checks for libraries.
PKG_CHECK_MODULES(LIBGTK, [gtk+-3.0])
PKG_CHECK_MODULES(LIBGST, [gstreamer-1.0])
PKG_CHECK_MODULES(LIBGSTVIDEO, [gstreamer-video-1.0])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([atexit floor modf sqrt strcasecmp strrchr])

AC_CONFIG_FILES([Makefile src/Makefile debian/Makefile])
AC_OUTPUT
