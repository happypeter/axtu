#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(axtu,1.2,yusop@haansoft.com)

AC_MSG_NOTICE([configuring for $PACKAGE_NAME $PACKAGE_VERSION])
AC_MSG_NOTICE([You can send e-mail($PACKAGE_BUGREPORT) for bug])

dnl
dnl Set the prefix
dnl
AC_SUBST(DEFAULT_PREFIX,/usr/share)

if (echo $prefix | grep -q NONE && true || false ) then
	AC_SUBST(prefix,$DEFAULT_PREFIX)
fi

AC_PREFIX_PROGRAM($prefix/$PACKAGE_NAME)

AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED([OS_TYPE], "$host_os",
                   [Define to be the name of the operating system.])

dnl
dnl Set the libdir
dnl
if test -d "$libdir"; then
	AC_MSG_NOTICE([Using inputted value - $libdir])
else
	searchpath=`(LC_ALL=C /usr/bin/ldd /usr/bin/cc) 2>/dev/null |grep lib64`
	if test -z "$searchpath"; then
		AC_SUBST(libdir,/usr/lib)
	else
		AC_SUBST(libdir,/usr/lib64)
	fi
	AC_MSG_NOTICE([Using default value - $libdir])
fi

	
dnl
dnl Process features.
dnl

AC_ARG_WITH(ssl,
[[  --without-ssl           disable SSL autodetection]])

AC_ARG_ENABLE(opie,
[  --disable-opie          disable support for opie or s/key FTP login],
ENABLE_OPIE=$enableval, ENABLE_OPIE=yes)
test x"${ENABLE_OPIE}" = xyes && AC_DEFINE([ENABLE_OPIE], 1,
   [Define if you want the Opie support for FTP compiled in.])

AC_ARG_ENABLE(digest,
[  --disable-digest        disable support for HTTP digest authorization],
ENABLE_DIGEST=$enableval, ENABLE_DIGEST=yes)
test x"${ENABLE_DIGEST}" = xyes && AC_DEFINE([ENABLE_DIGEST], 1,
   [Define if you want the HTTP Digest Authorization compiled in.])

AC_ARG_ENABLE(ntlm,
[  --disable-ntlm          disable support for NTLM authorization],
[ENABLE_NTLM=$enableval], [ENABLE_NTLM=auto])

AC_ARG_ENABLE(debug,
[  --disable-debug         disable support for debugging output],
ENABLE_DEBUG=$enableval, ENABLE_DEBUG=yes)
test x"${ENABLE_DEBUG}" = xyes && AC_DEFINE([ENABLE_DEBUG], 1,
   [Define if you want the debug output support compiled in.])

wget_need_md5=no

case "${ENABLE_OPIE}${ENABLE_DIGEST}" in
*yes*)
        wget_need_md5=yes
esac
if test x"$ENABLE_OPIE" = xyes; then
  OPIE_OBJ='ftp-opie$o'
fi
AC_SUBST(OPIE_OBJ)

dnl
dnl Whether make sets $(MAKE)...
dnl
AC_PROG_MAKE_SET


dnl
dnl Find a good install
dnl
AC_PROG_INSTALL

dnl
dnl Find the compiler
dnl

dnl We want these before the checks, so the checks can modify their values.
test -z "$CFLAGS"  && CFLAGS= auto_cflags=1
test -z "$CC" && cc_specified=yes

AC_AIX
AM_PROG_CC_STDC

dnl Turn on optimization by default.  Specifically:
dnl
dnl if the user hasn't specified CFLAGS, then
dnl   if compiler is gcc, then
dnl     use -O2 and some warning flags
dnl   else
dnl     use os-specific flags or -O
if test -n "$auto_cflags"; then
  if test -n "$GCC"; then
    CFLAGS="$CFLAGS -O2 -Wall -Wno-implicit"
  else
    case "$host_os" in
      *hpux*)  CFLAGS="$CFLAGS +O3"                      ;;
      *ultrix* | *osf*) CFLAGS="$CFLAGS -O -Olimit 2000" ;;
      *)       CFLAGS="$CFLAGS -O" ;;
    esac
  fi
fi

dnl
dnl In case of {cyg,gnu}win32.  Should be a _target_ test.
dnl Might also be erelevant for DJGPP.
dnl
case "$host_os" in
  *win32) exeext='.exe';;
  *) exeext='';;
esac
AC_SUBST(exeext)

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

if test -f '/etc/asianux-release' ;  then
        RELEASEFILE='/etc/asianux-release';
elif test -f '/etc/Haansoft-release' ;  then
        RELEASEFILE='/etc/Haansoft-release';
elif test -f '/etc/haansoft-release' ; then
        RELEASEFILE='/etc/haansoft-release';
fi


if (cat $RELEASEFILE | grep -q  Quartet && true || false ) then
        ProductName="AXS3";
elif (cat $RELEASEFILE | grep -q  nanumi && true || false ) then
        ProductName="OE3"
elif (cat $RELEASEFILE | grep -q  binari && true || false ) then
        ProductName="DT3"
else
        ProductName="DEFAULT"
fi


echo "Your Product is $ProductName"

AC_CANONICAL_SYSTEM

case "$target" in
        *x86_64*)
                cp conf/axtu.conf.$ProductName-x86_64 conf/.axtu.new
                ;;
        *i?86*)
                cp conf/axtu.conf.$ProductName-x86 conf/.axtu.new
                ;;
        *athlon*)
                cp conf/axtu.conf.$ProductName-x86 conf/.axtu.new
                ;;
        *powerpc*)
                cp conf/axtu.conf.$ProductName-ppc conf/.axtu.new
                ;;
        *ppc*)
                cp conf/axtu.conf.$ProductName-ppc conf/.axtu.new
                ;;
        *ia64*)
                cp conf/axtu.conf.$ProductName-ia64 conf/.axtu.new
                ;;
        *)
                cp conf/axtu.conf.$ProductName conf/.axtu.new
                ;;
esac

if test "$ProductName"="DEFAULT"; then
	cp conf/axtu.conf.$ProductName conf/.axtu.new;
fi


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LEX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_YACC

#AM_INIT_AUTOMAKE($PACKAGE_NAME,$PACKAGE_VERSION,[foreign])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE

#AC_PATH_KDE
# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main],[],AC_MSG_ERROR([libm not found!]))
# FIXME: Replace `main' with a function in `-lqt':
AC_CHECK_LIB([newt], [main],[],AC_MSG_ERROR([libnewt not found!]))
# FIXME: Replace `main' with a function in `-lpopt':
AC_CHECK_LIB([popt], [main],[],AC_MSG_ERROR([libpopt not found!]))
# FIXME: Replace `main' with a function in `-lrpm':
AC_CHECK_LIB([rpm], [main],[],AC_MSG_ERROR([librpm not found!]))
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main],[],AC_MSG_ERROR([librt not found!]))
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [main],[],AC_MSG_ERROR([libssl not found!]))

# Checks for header files.
AC_FUNC_ALLOCA
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h locale.h stdlib.h string.h strings.h unistd.h],[],AC_MSG_ERROR([header files not found!]))

AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h utime.h stdarg.h])
AC_CHECK_HEADERS(sys/types.h sys/stat.h sys/select.h sys/utime.h)

dnl Now check for the others.
AC_CHECK_HEADERS(stdint.h signal.h setjmp.h pwd.h)


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_HEADER_TIME
AC_C_VOLATILE
AC_C_BIGENDIAN
AC_C_PROTOTYPES
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

if test x"$am_cv_prog_cc_stdc" != xno; then :
  AC_SUBST(U, [])
  AC_SUBST(ANSI2KNR, [])
  AC_SUBST(ALL_LINGUAS)
else
  AC_SUBST(U, [_])
  AC_SUBST(ANSI2KNR, [./ansi2knr])
  AC_SUBST(ALL_LINGUAS)
fi

dnl
dnl Check integral type sizes.
dnl
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)

dnl
dnl Check for large file support.  This check needs to come fairly
dnl early because it could (in principle) affect whether functions and
dnl headers are available, whether they work, etc.
dnl
AC_SYS_LARGEFILE
AC_CHECK_SIZEOF(off_t)

dnl
dnl Checks for non-universal or system-specific types.
dnl
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_CHECK_TYPES(uint32_t)
AC_TYPE_SIGNAL
AC_CHECK_TYPES(sig_atomic_t, [], [], [
#include <stdio.h>
#include <sys/types.h>
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
])


# Checks for library functions.
AC_REPLACE_FNMATCH
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_PROG_GCC_TRADITIONAL
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([alarm clock_gettime gethostbyname gettimeofday inet_ntoa localeconv memchr memmove memset mkdir munmap select setlocale socket strcasecmp strchr strdup strerror strncasecmp strpbrk strrchr strstr utime],[],AC_MSG_ERROR([functions not found!]))
AC_CHECK_FUNCS(mktime strptime timegm snprintf vsnprintf,[],AC_MSG_ERROR([functions not found!]))
AC_CHECK_FUNCS(usleep ftello sigblock signal,[],AC_MSG_ERROR([functions not found!]))
AC_CHECK_FUNCS(symlink access isatty strtoll strtoimax,[],AC_MSG_ERROR([functions not found!]))

dnl
dnl Checks for libraries.
dnl
dnl Check for OpenSSL
if test x"$with_ssl" != x"no"
then
  dnl As of this writing (OpenSSL 0.9.6), the libcrypto shared library
  dnl doesn't record its dependency on libdl, so we need to make sure
  dnl -ldl ends up in LIBS on systems that have it.  Most OSes use
  dnl dlopen(), but HP-UX uses shl_load().
  AC_CHECK_LIB(dl, dlopen, [], [
    AC_CHECK_LIB(dl, shl_load)
  ])

  dnl Now actually check for -lssl
  AC_LIB_HAVE_LINKFLAGS([ssl], [crypto], [
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/des.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
  ], [SSL_library_init ()])
  if test x"$LIBSSL" != x
  then
    AC_MSG_NOTICE([compiling in support for SSL])
    AC_DEFINE([HAVE_SSL], 1,
              [Define if SSL support is being compiled in.])
    SSL_OBJ='openssl$o'
  elif test x"$with_ssl" != x
  then
    AC_MSG_ERROR([--with-ssl was given, but OpenSSL is not available.])
  fi
fi
AC_SUBST(SSL_OBJ)

dnl Enable NTLM if requested and if SSL is available.
NTLM_OBJ=''
if test x"$LIBSSL" != x
then
  if test x"$ENABLE_NTLM" != xno
  then
    AC_DEFINE([ENABLE_NTLM], 1,
     [Define if you want the NTLM authorization support compiled in.])
    NTLM_OBJ='http-ntlm$o'
  fi
else
  dnl If SSL is unavailable and the user explicitly requested NTLM,
  dnl abort.
  if test x"$ENABLE_NTLM" = xyes
  then
    AC_MSG_ERROR([NTLM authorization requested and OpenSSL not found; aborting])
  fi
fi

AC_SUBST(NTLM_OBJ)

dnl
dnl Find an MD5 implementation.  Since Wget rarely needs MD5, we try
dnl to use an existing library implementation to save on code size.
dnl

if test x"$wget_need_md5" = xyes
then
  dnl This should be moved to an AC_DEFUN, but I'm not sure how to
  dnl manipulate MD5_OBJ from the defun.

  MD5_OBJ='gen-md5$o'
  found_md5=no

  dnl Check for the system MD5 library on Solaris.  We don't check for
  dnl something simple like "MD5Update" because there are a number of
  dnl MD5 implementations that use that name, but have an otherwise
  dnl incompatible interface.  md5_calc is, hopefully, specific to the
  dnl Solaris MD5 library.
  if test x"$found_md5" = xno; then
    AC_CHECK_LIB(md5, md5_calc, [
      dnl Some installations have bogus <md5.h> in the compiler's
      dnl include path, making the system md5 library useless.
      AC_MSG_CHECKING([for working md5.h])
      AC_COMPILE_IFELSE([#include <md5.h>
                        ], [
        AC_MSG_RESULT(yes)
        AC_DEFINE([HAVE_SOLARIS_MD5], 1, [Define to use Solaris MD5.])
        LIBS="-lmd5 $LIBS"
        found_md5=yes
        AC_MSG_NOTICE([using the Solaris MD5 implementation])
      ], [AC_MSG_RESULT(no)])
    ])
  fi

  dnl Then see if we're linking OpenSSL anyway; if yes, use its md5
  dnl implementation.
  if test x"$found_md5" = xno; then
    if test x"$LIBSSL" != x; then
      AC_DEFINE([HAVE_OPENSSL_MD5], 1, [Define to use OpenSSL MD5.])
      found_md5=yes
      AC_MSG_NOTICE([using the OpenSSL MD5 implementation])
    fi
  fi

  dnl If none of the above worked, use the one we ship with Wget.
  if test x"$found_md5" = xno; then
    AC_DEFINE([HAVE_BUILTIN_MD5], 1, [Define to use built-in MD5.])
    MD5_OBJ="$MD5_OBJ gnu-md5\$o"
    found_md5=yes
    AC_MSG_NOTICE([using the GNU MD5 implementation])
  fi
fi
AC_DEFINE([HAVE_MD5], 1, [Define if we're compiling support for MD5.])
AC_SUBST(MD5_OBJ)

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=axtu-notifier-gui
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

COMAN_REQUIRES="\
                    gtk+-2.0 >= 2.4.0 \
            gthread-2.0"
PKG_CHECK_MODULES(COMAN, $COMAN_REQUIRES)
AC_SUBST(COMAN_REQUIRES)

dnl
dnl configure in the qt directory
dnl

lrelease gui/setup/setup.pro
lrelease gui/axtu/axtu.pro
lrelease gui/launcher/launcher.pro

qmake DESTDIR=$prefix/$PACKAGE_NAME/gui gui/setup/setup.pro -o gui/setup/Makefile
qmake DESTDIR=$prefix/$PACKAGE_NAME/gui gui/axtu/axtu.pro -o gui/axtu/Makefile
qmake DESTDIR=$prefix/$PACKAGE_NAME/gui gui/launcher/launcher.pro -o gui/launcher/Makefile
	

AC_CONFIG_FILES([Makefile
		 libs/Makefile
		 libs/wget/Makefile
		 libs/wget/conf/Makefile
		 daemon/Makefile
         gui/Makefile
         gui/notifier/Makefile
         tui/Makefile
		 tui/axtu/Makefile
		 tui/axtu-cl/Makefile
		 tui/launcher/Makefile
		 tui/setup/Makefile
		 man/Makefile])

AC_CONFIG_HEADERS([libs/wget/config.h])
AH_BOTTOM([
#include "config-post.h"
])
AC_OUTPUT
