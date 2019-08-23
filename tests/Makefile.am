AUTOMAKE_OPTIONS = serial-tests
check_PROGRAMS = resample_test
AM_CXXFLAGS = -Wall -I$(top_builddir)/src

resample_test_SOURCES = resample_test.cpp
resample_test_LDADD = $(top_builddir)/src/libresample.la

TESTS = $(check_PROGRAMS)
