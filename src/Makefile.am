AM_CXXFLAGS = -Wall

lib_LTLIBRARIES = libresample.la
libresample_la_SOURCES = Resampler.cpp

bin_PROGRAMS = resample

resample_SOURCES = main.cpp
resample_LDADD = $(top_builddir)/src/libresample.la

noinst_HEADERS = \
	Resampler.h

