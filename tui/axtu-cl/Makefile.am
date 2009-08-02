bindir = $(prefix)/$(PACKAGE_NAME)/tui

bin_PROGRAMS = axtu-cl

axtu_cl_SOURCES = main.cpp

axtu_cl_LDADD = ../../libs/libaxtu_shared.la

DEPENDENCIES = ../../libs/libaxtu_shared.la

LIBS = @LIBS@ -lpopt -lrpm

INCLUDES = -I../../libs
