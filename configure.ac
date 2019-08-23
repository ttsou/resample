AC_INIT([resample], [0.1], [tom@tsou.cc])
AC_CONFIG_SRCDIR([src/Resampler.cpp])
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-define 1.6])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_14
AM_PROG_AS
AC_PROG_INSTALL
LT_INIT([pic-only])
AC_CONFIG_MACRO_DIR([m4])
AC_CHECK_LIB([m],[sincos])

AC_OUTPUT(
	src/Makefile
	Makefile)
