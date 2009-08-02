## Process this file with automake to produce Makefile.in
bindir = $(prefix)/$(PACKAGE_NAME)/gui

bin_PROGRAMS = axtu-notifier-gui

axtu_notifier_gui_SOURCES = main.c eggtrayicon.c eggtrayicon.h

axtu_notifier_gui_CPPFLAGS=-DDATADIR='"$(datadir)"' -DPIXMAPS_DIR='"$(datadir)/pixmaps"' -DLOCALEDIR="\"@datadir@\"" @CPPFLAGS@

axtu_notifier_gui_LDADD = @COMAN_LIBS@

INCLUDES = @COMAN_CFLAGS@ -I../../libs 

imagesdir = $(bindir)/images

images_DATA = images/*.png

$(posja_DATA): po/ja/axtu-notifier-gui.mo
$(posko_DATA): po/ko/axtu-notifier-gui.mo
$(poszhcn_DATA): po/zh_CN/axtu-notifier-gui.mo
$(poszhtw_DATA): po/zh_TW/axtu-notifier-gui.mo

po/ja/axtu-notifier-gui.mo:
	msgfmt po/ja/axtu-notifier-gui.po -o $@

po/ko/axtu-notifier-gui.mo:
	msgfmt po/ko/axtu-notifier-gui.po -o $@

po/zh_CN/axtu-notifier-gui.mo:
	msgfmt po/zh_CN/axtu-notifier-gui.po -o $@

po/zh_TW/axtu-notifier-gui.mo:
	msgfmt po/zh_TW/axtu-notifier-gui.po -o $@
	
poskodir = $(prefix)/locale/ko/LC_MESSAGES

posko_DATA = po/ko/*.mo

posjadir = $(prefix)/locale/ja/LC_MESSAGES

posja_DATA = po/ja/*.mo

poszhcndir = $(prefix)/locale/zh_CN/LC_MESSAGES

poszhcn_DATA = po/zh_CN/*.mo

poszhtwdir = $(prefix)/locale/zh_TW/LC_MESSAGES

poszhtw_DATA = po/zh_TW/*.mo
