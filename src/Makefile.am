AM_CXXFLAGS = -Wall -I$(top_srcdir)/src/lib
SUBDIRS = lib

bin_PROGRAMS = resample

resample_SOURCES = main.cpp
resample_LDADD = $(top_builddir)/src/lib/libresample.la
