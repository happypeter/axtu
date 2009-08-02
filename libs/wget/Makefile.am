SUBDIRS = conf
##libdir = /usr/lib

noinst_LTLIBRARIES = libhswget.la

libhswget_la_CFLAGS = -fPIC

libhswget_la_SOURCES = config-post.h \
		connect.h \
		cookies.c \
		hash.c	\
		host.h  \
		http.c	\
		log.c   	\
		netrc.c	\
		progress.c	\
		ptimer.h 	\
		res.c   	\
		retr.h      \
		ssl.h     	\
		url.h	\
		version.c  	\
		xmalloc.h 	\
		config.h    \
		convert.c  	\
		cookies.h  	\
		gen-md5.c  	\
		hash.h  	\
		http-ntlm.c \
		init.c  	\
		log.h   	\
		netrc.h    	\
		progress.h  \
		recur.c   	\
		res.h   	\
		safe-ctype.c\
		sysdep.h  	\
		utils.c  	\
		wget.h 	\
		connect.c   \
		convert.h   \
		gen-md5.h  	\
		host.c  	\
		http-ntlm.h \
		init.h  	\
		options.h  	\
		ptimer.c    \
		recur.h   	\
		retr.c  	\
		safe-ctype.h\
		url.c     	\
		utils.h  	\
		xmalloc.c 	\
		openssl.c 	\
		html-url.c 	\
		html-url.h 	\
		html-parse.c\
		html-parse.h\
		gnu-md5.c 	\
		gnu-md5.h 	\
		wgetlib.h

includedir=/usr/include/axtu/wget
include_HEADERS = ./*.h

clean:
	rm -rf *.o *.a 
	@echo deleted library files
