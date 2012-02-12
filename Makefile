# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

####### kdevelop will overwrite this part!!! (begin)##########


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/gta2k4lin
pkglibdir = $(libdir)/gta2k4lin
pkgincludedir = $(includedir)/gta2k4lin

top_builddir = ..

ACLOCAL = aclocal
AUTOCONF = autoconf
AUTOMAKE = automake
AUTOHEADER = autoheader

INSTALL = /usr/bin/ginstall -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
build_alias = i686-pc-linux-gnu
build_triplet = i686-pc-linux-gnu
host_alias = i686-pc-linux-gnu
host_triplet = i686-pc-linux-gnu
target_alias = i686-pc-linux-gnu
target_triplet = i686-pc-linux-gnu
AS = @AS@
AUTODIRS = 
CC = gcc
CPP = gcc -E
CXX = g++
CXXCPP = @CXXCPP@
DCOPIDL = @DCOPIDL@
DCOPIDL2CPP = @DCOPIDL2CPP@
DCOP_DEPENDENCIES = @DCOP_DEPENDENCIES@
DLLTOOL = @DLLTOOL@
DPMSINC = @DPMSINC@
DPMSLIB = @DPMSLIB@
EXEEXT = 
GLINC = @GLINC@
GLLIB = @GLLIB@
GMSGFMT = @GMSGFMT@
IDL = @IDL@
IDL_DEPENDENCIES = @IDL_DEPENDENCIES@
KDE_CXXFLAGS = 
KDE_EXTRA_RPATH = @KDE_EXTRA_RPATH@
KDE_INCLUDES = @KDE_INCLUDES@
KDE_LDFLAGS = @KDE_LDFLAGS@
KDE_RPATH = @KDE_RPATH@
KDE_USE_CLOSURE_FALSE = #
KDE_USE_CLOSURE_TRUE = 
KDE_USE_FINAL_FALSE = 
KDE_USE_FINAL_TRUE = #
LIBCOMPAT = @LIBCOMPAT@
LIBCRYPT = @LIBCRYPT@
LIBDL = -ldl
LIBJPEG = @LIBJPEG@
LIBMICO = @LIBMICO@
LIBOBJS = @LIBOBJS@
LIBPNG = @LIBPNG@
LIBPTHREAD = @LIBPTHREAD@
LIBPYTHON = @LIBPYTHON@
LIBQIMGIO = @LIBQIMGIO@
LIBSM = @LIBSM@
LIBSOCKET = @LIBSOCKET@
LIBTIFF = @LIBTIFF@
LIBTOOL = libtool
LIBUCB = @LIBUCB@
LIBZ = @LIBZ@
LIB_KAB = @LIB_KAB@
LIB_KDECORE = @LIB_KDECORE@
LIB_KDEUI = @LIB_KDEUI@
LIB_KFILE = @LIB_KFILE@
LIB_KFM = @LIB_KFM@
LIB_KFORMULA = @LIB_KFORMULA@
LIB_KHTML = @LIB_KHTML@
LIB_KIMGIO = @LIB_KIMGIO@
LIB_KIO = @LIB_KIO@
LIB_KPARTS = @LIB_KPARTS@
LIB_KSPELL = @LIB_KSPELL@
LIB_KSYCOCA = @LIB_KSYCOCA@
LIB_MEDIATOOL = @LIB_MEDIATOOL@
LIB_QT = @LIB_QT@
LIB_SMB = @LIB_SMB@
LIB_X11 = @LIB_X11@
LN_S = ln -s
MAKEINFO = makeinfo
MICO_INCLUDES = @MICO_INCLUDES@
MICO_LDFLAGS = @MICO_LDFLAGS@
MOC = @MOC@
MSGFMT = @MSGFMT@
NOOPT_CXXFLAGS =  -fno-exceptions -fno-rtti -fno-check-new
NOREPO = -fno-repo
OBJDUMP = @OBJDUMP@
OBJEXT = o
PACKAGE = gta2k4lin
PAMINC = @PAMINC@
PAMLIBPATHS = @PAMLIBPATHS@
PAMLIBS = @PAMLIBS@
PYTHONINC = @PYTHONINC@
PYTHONLIB = @PYTHONLIB@
QT_INCLUDES = @QT_INCLUDES@
QT_LDFLAGS = @QT_LDFLAGS@
RANLIB = ranlib
REPO = -frepo
STRIP = strip
TOPSUBDIRS = @TOPSUBDIRS@
USER_INCLUDES = 
USER_LDFLAGS = 
USE_EXCEPTIONS = -fexceptions
USE_NLS = @USE_NLS@
USE_RTTI = -frtti
VERSION = 0.1
XGETTEXT = @XGETTEXT@
XPMINC = @XPMINC@
XPMLIB = @XPMLIB@
X_EXTRA_LIBS = @X_EXTRA_LIBS@
X_INCLUDES = @X_INCLUDES@
X_LDFLAGS = @X_LDFLAGS@
X_PRE_LIBS = @X_PRE_LIBS@
all_includes =  
all_libraries =  
idldir = @idldir@
kde_appsdir = @kde_appsdir@
kde_bindir = @kde_bindir@
kde_cgidir = @kde_cgidir@
kde_confdir = @kde_confdir@
kde_datadir = @kde_datadir@
kde_htmldir = @kde_htmldir@
kde_icondir = @kde_icondir@
kde_includes = @kde_includes@
kde_libraries = @kde_libraries@
kde_locale = @kde_locale@
kde_mimedir = @kde_mimedir@
kde_minidir = @kde_minidir@
kde_servicesdir = @kde_servicesdir@
kde_servicetypesdir = @kde_servicetypesdir@
kde_sounddir = @kde_sounddir@
kde_toolbardir = @kde_toolbardir@
kde_wallpaperdir = @kde_wallpaperdir@
micodir = @micodir@
qt_includes = @qt_includes@
qt_libraries = @qt_libraries@
x_includes = @x_includes@
x_libraries = @x_libraries@

####### kdevelop will overwrite this part!!! (end)############
bin_PROGRAMS = gta2k4lin
gta2k4lin_SOURCES = TGALoader.cpp main.cpp 
gta2k4lin_LDADD = -lSDL -lSDL_mixer -lpthread -lGL -lGLU -lX11

SUBDIRS = data 

EXTRA_DIST = main.cpp Tga.h TGALoader.cpp 

# set the include path found by configure
INCLUDES = $(all_includes) -I/usr/include -I/usr/include/SDL

# the library search path.
gta2k4lin_LDFLAGS = $(all_libraries) -L/usr/local/lib
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
bin_PROGRAMS =  gta2k4lin$(EXEEXT)
PROGRAMS =  $(bin_PROGRAMS)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
LDFLAGS = -s
LIBS = 
gta2k4lin_OBJECTS =  TGALoader.$(OBJEXT) main.$(OBJEXT)
gta2k4lin_DEPENDENCIES = 
CXXFLAGS = -O2 -fno-exceptions -fno-rtti -fno-check-new -Wwrite-strings -DSOUND -fpermissive
CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
LTCXXCOMPILE = $(LIBTOOL) --mode=compile $(CXX) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)
CXXLD = $(CXX)
CXXLINK = $(LIBTOOL) --mode=link $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@
DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = tar
GZIP_ENV = --best
SOURCES = $(gta2k4lin_SOURCES)
OBJECTS = $(gta2k4lin_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .cpp .lo .o .obj .s
#$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
#	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps gta2k4lin/Makefile

#Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
#	cd $(top_builddir) \
#	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

.c.o:
	$(COMPILE) -c $<

# FIXME: We should only use cygpath when building on Windows,
# and only if it is available.
.c.obj:
	$(COMPILE) -c `cygpath -w $<`

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core
	-rm -f *.$(OBJEXT)

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.c.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

gta2k4lin$(EXEEXT): $(gta2k4lin_OBJECTS) $(gta2k4lin_DEPENDENCIES)
	@rm -f gta2k4lin$(EXEEXT)
	$(CXXLINK) $(gta2k4lin_LDFLAGS) $(gta2k4lin_OBJECTS) $(gta2k4lin_LDADD) $(LIBS)
.cpp.o:
	$(CXXCOMPILE) -c $<
.cpp.obj:
	$(CXXCOMPILE) -c `cygpath -w $<`
.cpp.lo:
	$(LTCXXCOMPILE) -c $<

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" = "." && dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = gta2k4lin

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	for subdir in $(SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(top_distdir) distdir=../$(distdir)/$$subdir distdir) \
	      || exit 1; \
	  fi; \
	done

info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
install-exec-am: install-binPROGRAMS
install-exec: install-exec-recursive

install-data-am:
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am: uninstall-binPROGRAMS
uninstall: uninstall-recursive
all-am: Makefile $(PROGRAMS)
all-redirect: all-recursive
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:
	$(mkinstalldirs)  $(DESTDIR)$(bindir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-libtool mostlyclean-tags \
		mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-binPROGRAMS clean-compile clean-libtool clean-tags \
		clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-binPROGRAMS distclean-compile distclean-libtool \
		distclean-tags distclean-generic clean-am
	-rm -f libtool

distclean: distclean-recursive

maintainer-clean-am:  maintainer-clean-binPROGRAMS \
		maintainer-clean-compile maintainer-clean-libtool \
		maintainer-clean-tags maintainer-clean-generic \
		distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive

.PHONY: mostlyclean-binPROGRAMS distclean-binPROGRAMS clean-binPROGRAMS \
maintainer-clean-binPROGRAMS uninstall-binPROGRAMS install-binPROGRAMS \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile mostlyclean-libtool distclean-libtool \
clean-libtool maintainer-clean-libtool install-data-recursive \
uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check check-am installcheck-am installcheck install-exec-am \
install-exec install-data-am install-data install-am install \
uninstall-am uninstall all-redirect all-am all installdirs-am \
installdirs mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
