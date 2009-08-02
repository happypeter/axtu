SUBDIRS = wget

lib_LTLIBRARIES = libaxtu_shared.la

libaxtu_shared_la_LDFLAGS = -fPIC -L$(libdir)

libaxtu_shared_la_LIBADD = wget/libhswget.la

libaxtu_shared_la_SOURCES =	\
	smp.h 			\
	smp.c 			\
	classAuthen.cpp		\
        classDownloader.cpp  	\
	classNetwork.cpp    	\
	hsCommon.h		\
	classAuthen.h          	\
	classDownloader.h    	\
	classNetwork.h		\
	classConfigParser.cpp  	\
	classConfCtl.cpp  	\
	classLogger.cpp      	\
	classRpmEngine.cpp	\
	classConfigParser.h 	\
	classLogger.h        	\
	classRpmEngine.h	\
	semun.h			\
	classLock.h 		\
	minifind.c		\
	minifind.h		\
	commondef.h		\
	classRedcastle.h	\
	classRedcastle.cpp	

includedir=/usr/include/axtu
include_HEADERS = ./*.h
##libaxtu_shared_la_CXXFLAGS = -DDEBUG
