#AUTOMAKE_OPTIONS = foreign 1.5
#DISTCLEANFILES = inst-apps 
# COMPILE_BEFORE_dcoprss= librss
# COMPILE_BEFORE_knewsticker= librss

#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 SUBDIRS
confdir = /var/axtu
bindir = /usr/bin
sbindir = /sbin

conf_DATA = conf/.axtu.new
bin_SCRIPTS = sh/axtu
sbin_SCRIPTS = sh/axtud

SUBDIRS= libs gui tui daemon man
#$(TOPSUBDIRS)

distclean: distclean-recursive
	-rm -f $(am__CONFIG_DISTCLEAN_FILES)
	-rm -f Makefile
	-rm -f gui/axtu/Makefile 
	-rm -r gui/axtu/qm/*.qm
	-rm -f gui/launcher/Makefile 
	-rm -r gui/launcher/qm/*.qm
	-rm -f gui/notifier/Makefile  
	-rm -f gui/setup/Makefile
	-rm -r gui/setup/qm/*.qm
	-rm -f conf/.axtu.new	 
	-rm -r /var/axtu/tmp/
	-rm -r /var/run/axtu/
	
install-exec-hook :
	mkdir -p /var/axtu/tmp
	mkdir -p /var/run/axtu
	mkdir -p /etc/axtu
