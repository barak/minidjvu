AC_PREREQ([2.67])
AC_INIT([minidjvu],[0.8],[alexios@thessalonica.org.ru])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr/local)

LIBRARY_VERSION=`echo $PACKAGE_VERSION | sed -e 's/\./:/'`

AC_CANONICAL_HOST

target="$PACKAGE_NAME"
mdjvulib="lib$PACKAGE_NAME"

prefer_static='no'
prefer_gettext='yes'
AC_MSG_CHECKING([if we are building for Windows])
case "$host" in
  *-mingw32)
    prefer_static='yes'
    prefer_gettext='no'
    target="$target.exe"
    AC_MSG_RESULT(yes)
  ;;
  *)
    AC_MSG_RESULT(no)
  ;;
esac

AC_ARG_ENABLE(
    [static],
    [  --enable-static         Enable static building],
    [static=$enableval],
    [static=$prefer_static])

AC_ARG_ENABLE(
    [gettext],
    [  --enable-gettext        Enable localization via gettext],
    [gettext=$enableva],
    [gettext=$prefer_gettext])

AC_ARG_WITH(
    [tiff],
    [  --with-tiff             Enable static tiff support],
    [tiff=$enableval],
    [tiff=yes])

if test "$static" = "yes"; then
    if test "x$LDFLAGS" = "x"; then
        LDFLAGS="-static"
    fi
    mdjvulib="$mdjvulib.a"
    oext='o'
    SHARED_YES='#'
else
    mdjvulib="$mdjvulib.la"
    oext='lo'
    STATIC_YES='#'
fi

case "$host" in
  *-mingw32)
    if test "$static" != "yes"; then
      DLLFLAGS="$DLLFLAGS -Wl,--export-all-symbols"
    fi
  ;;
  *-darwin* | *-macos10*)
    if test -d /opt/local ; then
      INCS="-I/opt/local/include"
      LIBS="-L/opt/local/lib $LIBS"
    elif test -d /sw ; then
      INCS="-I/sw/include $CPPFLAGS"
      LIBS="-L/sw/lib $LIBS"
    fi
  ;;
esac

# Checks for programs.

if test "x$CFLAGS" = "x"; then
    CFLAGS="-g -pipe -O3"
fi
AC_PROG_MAKE_SET
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_AWK
LT_INIT
AC_PROG_INSTALL
AC_PATH_PROG(GZIP, gzip)
AC_PATH_PROG(RM, rm)
RM="$RM -f"

# Checks for libraries.

zlib_ok='no'
dnl Test for libz
AC_CHECK_LIB(z, inflate, 
    [z_libs="-lz"; zlib_ok='yes'])

jpeg_ok='no'
AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
    [jpeg_libs="-ljpeg"; jpeg_ok='yes'])

# Checks for header files.

if test "$tiff" != 'no'; then
    have_tiff_header='no'
    have_libtiff='no'
    
    if test "$static" != 'yes' || test "$zlib_ok" = 'yes' -a "$jpeg_ok" = 'yes'; then
        AC_CHECK_HEADER(tiff.h,  have_tiff_header='yes')
        AC_CHECK_HEADER(tiffio.h,have_tiff_header='yes',have_tiff_header='no')
        if test "$have_tiff_header" = 'yes'; then
            saveLIBS="$LIBS"
            if test "$static" = 'yes'; then
                LIBS="$z_libs $jpeg_libs $LIBS"
            fi
            
            have_libtiff='no'
            AC_CHECK_LIB(tiff,TIFFOpen,have_libtiff='yes',have_libtiff='no')
            if test "$have_libtiff" != 'no'; then
                LIBS="-ltiff $LIBS"
                AC_DEFINE(HAVE_TIFF,1,Define if you have TIFF library)
            else
                LIBS=$saveLIBS
            fi
        fi
    fi

    if test "$have_libtiff" != 'yes'; then
        AC_MSG_WARN(*** Native TIFF support will not be included ***)
    fi
fi

PO_YES='#'
if test "$gettext" != 'no'; then
    have_intl_header='no'
    gettext_ok='no'
    needs_lintl='no'
    
    AC_CHECK_HEADER(libintl.h, have_intl_header='yes')
    if test "$have_intl_header" = 'yes'; then
        AC_CHECK_FUNC(gettext,gettext_ok='yes')
        
        if test "$gettext_ok" = 'no'; then
            AC_CHECK_LIB(intl,gettext,[gettext_ok='yes'; needs_lintl='yes'])
        fi
    fi
    
    iconv_ok='no'
    needs_liconv='no'
    AC_CHECK_FUNC(iconv, [iconv_ok='yes'])
    if test "$iconv_ok" = "no"; then
        AC_CHECK_LIB([iconv], [iconv], [iconv_ok='yes'; needs_liconv='yes'])
    fi
    
    AC_CHECK_PROG(MSGFMT, msgfmt, msgfmt)
    if test "$MSGFMT" != ""; then 
    if msgfmt --help 2>&1 | grep illegal >/dev/null ; then
        echo "msgfmt isn't GNU version"
        MSGFMT=""
    fi
    fi

    if test "$MSGFMT" != "" -a "$gettext_ok" != "no" -a "$iconv_ok" != "no" ; then
        echo "msgfmt, iconv() and gettext() exist; will build i18n support"
        AC_DEFINE(HAVE_GETTEXT,,Define if we are building i18n support)
        PO_YES=''
        if test "$needs_lintl" != "no"; then
            intl_libs="-lintl"
            LIBS="$intl_libs $LIBS"
        fi
        if test "$needs_liconv" != "no"; then
            iconv_libs="-liconv"
            LIBS="$iconv_libs $LIBS"
        fi
    else
        echo "msgfmt and libintl don't both exist; will not build i18n support"
    fi
fi

AC_CHECK_HEADERS([libintl.h locale.h stdint.h stdlib.h string.h])

AC_SUBST(target)
AC_SUBST(mdjvulib)
AC_SUBST(oext)
AC_SUBST(OPTS)
AC_SUBST(INCS)
AC_SUBST(MSGFMT)
AC_SUBST(LIBRARY_VERSION)
AC_SUBST(DLLFLAGS)
AC_SUBST(PO_YES)
AC_SUBST(STATIC_YES)
AC_SUBST(SHARED_YES)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

CFLAGS="$CFLAGS -Wall -Wshadow"
CFLAGS="$CFLAGS -pedantic-errors -Wpointer-arith -Waggregate-return"
CFLAGS="$CFLAGS -Wlong-long -Wredundant-decls -Wcast-qual -Wcast-align"
DEFS="$DEFS -D__STRICT_ANSI__ -DNDEBUG"
OPTS="-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations"

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset pow setlocale strcspn strrchr])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([tools/Makefile])
AC_CONFIG_FILES([po/Makefile])
AC_OUTPUT
