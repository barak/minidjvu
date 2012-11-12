AC_PREREQ([2.67])
AC_INIT([minidjvu],[0.8],[alexios@thessalonica.org.ru])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr/local)

AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_LANG([C++])

LT_INIT

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PATH_PROG(GZIP, gzip)
AC_PATH_PROG(RM, rm)

# Checks for libraries.

AC_CHECK_LIB(z, inflate)
AC_CHECK_LIB(jpeg, jpeg_destroy_decompress)
AC_CHECK_LIB(tiff, TIFFOpen)

# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

dnl CFLAGS="$CFLAGS -Wall -Wshadow"
dnl CFLAGS="$CFLAGS -pedantic-errors -Wpointer-arith -Waggregate-return"
dnl CFLAGS="$CFLAGS -Wlong-long -Wredundant-decls -Wcast-qual -Wcast-align"
dnl DEFS="$DEFS -D__STRICT_ANSI__ -DNDEBUG"
dnl OPTS="-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations"

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow setlocale strcspn strrchr])

# Trailer
AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/ru/Makefile
                 include/Makefile
		 po/Makefile.in])
AC_OUTPUT
