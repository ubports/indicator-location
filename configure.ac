
AC_INIT(indicator-location, 0.0.0)

AC_PREREQ(2.53)
AM_INIT_AUTOMAKE([])

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])

IT_PROG_INTLTOOL([0.35.0])
GETTEXT_PACKAGE=indicator-location
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [gettext package])
AM_GLIB_GNU_GETTEXT

AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([
    Makefile
	data/Makefile
	src/Makefile
    po/Makefile.in
])

PKG_CHECK_MODULES(INDICATOR, glib-2.0
                  appindicator3-0.1)

###########################
# Local Install
###########################

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall],
                                           [install all files locally (for distcheck)]),
              with_localinstall=$enableval,
              with_localinstall=no)

###########################
# gcov coverage reporting
###########################

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AM_CONDITIONAL([HAVE_GCOV], [test "x$ac_cv_check_gcov" = xyes])
AM_CONDITIONAL([HAVE_LCOV], [test "x$ac_cv_check_lcov" = xyes])
AM_CONDITIONAL([HAVE_GCOVR], [test "x$ac_cv_check_gcovr" = xyes])
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

AC_SUBST(AM_CFLAGS, "-Wall -Werror")

AC_OUTPUT

