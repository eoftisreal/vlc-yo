AUTOMAKE_OPTIONS = subdir-objects
pkgconfigdir = $(libdir)/pkgconfig
AM_CFLAGS = $(CFLAGS_libapoi)

SUFFIXES = .pc.in .pc .rc.in .rc

BUILT_SOURCES = $(nodist_pkginclude_HEADERS)
CLEANFILES = $(BUILT_SOURCES) $(pkgconfig_DATA)

pkginclude_HEADERS = \
	../include/apoi/libapoi.h \
	../include/vlc/libapoi_dialog.h \
	../include/vlc/libapoi_events.h \
	../include/vlc/libapoi_media.h \
	../include/vlc/libapoi_media_discoverer.h \
	../include/vlc/libapoi_media_list.h \
	../include/vlc/libapoi_media_list_player.h \
	../include/vlc/libapoi_media_player.h \
	../include/vlc/libapoi_media_track.h \
	../include/vlc/libapoi_renderer_discoverer.h \
	../include/vlc/libapoi_picture.h \
	../include/vlc/libapoi_version.h \
	../include/vlc/libapoi_video.h \
	../include/vlc/vlc.h

lib_LTLIBRARIES = libapoi.la

libapoi_la_SOURCES = \
	libapoi_internal.h \
	media_internal.h \
	media_list_internal.h \
	media_player_internal.h \
	picture_internal.h \
	renderer_discoverer_internal.h \
	core.c \
	dialog.c \
	renderer_discoverer.c \
	error.c \
	log.c \
	video.c \
	audio.c \
	event.c \
	media.c \
	media_track.c \
	media_player.c \
	media_list.c \
	media_list_path.h \
	media_list_player.c \
	media_discoverer.c \
	picture.c \
	libapoi.pc.in \
	libapoi.sym \
	../src/revision.c
EXTRA_DIST =

if HAVE_WIN32
noinst_DATA = libapoi_win32_rc.rc
endif
EXTRA_DIST += libapoi_win32_rc.rc.in

libapoi_win32_rc.rc: libapoi_win32_rc.rc.in $(top_builddir)/config.status
	$(AM_V_GEN)cd "$(top_builddir)" && \
	$(SHELL) ./config.status --file="lib/$@"

libapoi_la_LIBADD = ../src/libapoicore.la ../compat/libcompat.la $(LIBM)
libapoi_la_LDFLAGS = \
	$(LDFLAGS_libapoi) \
	-no-undefined \
	-version-number @LIBAPOI_ABI_MAJOR@:@LIBAPOI_ABI_MINOR@:@LIBAPOI_ABI_MICRO@ \
	-export-symbols $(srcdir)/libapoi.sym
EXTRA_libapoi_la_DEPENDENCIES = libapoi.sym
libapoi_la_CPPFLAGS = $(AM_CPPFLAGS)
if HAVE_WIN32
EXTRA_libapoi_la_DEPENDENCIES += libapoi_win32_rc.$(OBJEXT)
libapoi_la_LDFLAGS += -Wl,libapoi_win32_rc.$(OBJEXT) -avoid-version -Wc,-static
if HAVE_DYNAMIC_PLUGINS
libapoi_la_CPPFLAGS += -DLIBAPOI_DLL_EXPORT
endif
endif
if HAVE_OS2
libapoi_la_LDFLAGS += -avoid-version
endif
if HAVE_DARWIN
libapoi_la_LDFLAGS += -Xlinker -install_name -Xlinker @rpath/libapoi.dylib
endif

# iOS and tvOS applications cannot install global shared libraries and
# dylibs must be in frameworks so there's no need for libtool versioning.
if HAVE_IOS
libapoi_la_LDFLAGS += -avoid-version
endif
if HAVE_TVOS
libapoi_la_LDFLAGS += -avoid-version
endif

pkgconfig_DATA = libapoi.pc

libapoi.pc: libapoi.pc.in $(top_builddir)/config.status
	$(AM_V_GEN)cd "$(top_builddir)" && \
	$(SHELL) ./config.status --file="lib/$@"

libapoi_win32_rc.$(OBJEXT): libapoi_win32_rc.rc $(top_srcdir)/extras/package/win32/libapoi.dll.manifest
	$(WINDRES) --include-dir $(top_srcdir)/share --include-dir $(top_srcdir)/extras/package/win32 -i $< -o $@

checkheader_verbose = $(checkheader_verbose_$(V))
checkheader_verbose_ = $(checkheader_verbose_$(AM_DEFAULT_VERBOSITY))
checkheader_verbose_0 = @echo "  CHECK    $(@:checkheader_%_h=vlc/%.h)";
checkheader_verbose__0 = $(checkheader_verbose_0)
nodist_check_SCRIPTS = $(pkginclude_HEADERS:../include/vlc/%.h=checkheader_%_h)
MOSTLYCLEANFILES = \
	$(pkginclude_HEADERS:../include/vlc/%.h=checkheader_%_h.c) \
	$(pkginclude_HEADERS:../include/vlc/%.h=checkheader_%_h.lo)

checkheader_%_h:
	@echo "#include <stdbool.h>" > $@.c
	@echo "#include <apoi/libapoi.h>" >> $@.c
	@echo "#include <apoi/$*.h>" >> $@.c
	$(checkheader_verbose)$(LTCOMPILE) -c $@.c -o $@.lo

check-local:
	$(SHELL) $(top_srcdir)/src/check_headers $(pkginclude_HEADERS)
