bindir = $(prefix)/$(PACKAGE_NAME)/tui

bin_PROGRAMS = axtu-setup-tui

axtu_setup_tui_SOURCES = axtu_setup.cpp main.cpp

axtu_setup_tui_LDADD = ../../libs/libaxtu_shared.la

DEPENDENCIES = ../../libs/libaxtu_shared.la

LIBS = @LIBS@ -lpopt -lnewt -lrpm

INCLUDES = -I../../libs
