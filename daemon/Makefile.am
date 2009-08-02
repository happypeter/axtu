bindir = $(prefix)/$(PACKAGE_NAME)/daemon

bin_PROGRAMS = axtu-daemon

axtu_daemon_SOURCES = axtu-daemon.cpp

axtu_daemon_LDADD = ../libs/libaxtu_shared.la

DEPENDENCIES = ../libs/libaxtu_shared.la

LIBS = @LIBS@ -lpopt -lnewt -lrpm -lrt -lssl

INCLUDES = -I../libs -I../libs/wget
