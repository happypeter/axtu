bindir = $(prefix)/$(PACKAGE_NAME)/tui

bin_PROGRAMS = axtu-tui

axtu_tui_SOURCES = classTui.cpp classTui.h main.cpp 

axtu_tui_LDADD = ../../libs/libaxtu_shared.la

DEPENDENCIES = ../../libs/libaxtu_shared.la

LIBS = -L$(libdir) -lpopt -lnewt -lrpm -lrt -lssl

INCLUDES = -I../../libs -I/usr/include/hswget -include../../libs/wget/config.h
