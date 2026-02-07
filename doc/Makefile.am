NULL =

LIBAPOI_SAMPLES = \
	libapoi/gtk_player.c \
	libapoi/QtPlayer/LICENSE \
	libapoi/QtPlayer/main.cpp \
	libapoi/QtPlayer/player.cpp \
	libapoi/QtPlayer/player.h \
	libapoi/QtPlayer/QtVLC.pro \
	libapoi/QtGL/main.cpp \
	libapoi/QtGL/qtvlcwidget.cpp \
	libapoi/QtGL/qtvlcwidget.h \
	libapoi/QtGL/QtGl.pro \
	libapoi/thumbnailer.c \
	libapoi/wx_player.cpp \
	libapoi/d3d11_player.cpp \
	libapoi/d3d9_player.c \
	libapoi/win_player.c \
	libapoi/CMakeLists.txt \
	libapoi/appkit_player.m \
	$(NULL)

# Check that samples that depends only on LibAPOI can build
AUTOMAKE_OPTIONS = subdir-objects
AM_LDFLAGS = -no-install $(LDFLAGS_vlc) -L../src/ -lvlccore -L../lib -lvlc

samples_libapoi_example_SOURCES = libapoi/example.c
samples_libapoi_thumbnailer_SOURCES = libapoi/thumbnailer.c

check_PROGRAMS = \
	samples_libapoi_example \
	samples_libapoi_thumbnailer \
	$(NULL)

nobase_doc_DATA = $(LIBAPOI_SAMPLES)

doc_DATA = \
	fortunes.txt \
	$(NULL)

CHANGELOGS = \
	ChangeLog-1999 \
	ChangeLog-2000 \
	ChangeLog-2001 \
	ChangeLog-2002 \
	ChangeLog-2003 \
	ChangeLog-2004 \
	ChangeLog-2005 \
	ChangeLog-2006 \
	ChangeLog-2007 \
	ChangeLog-2008 \
	ChangeLog-2009 \
	ChangeLog-2010 \
	ChangeLog-2011 \
	ChangeLog-2012 \
	ChangeLog-2013 \
	ChangeLog-2014 \
	ChangeLog-2015 \
	ChangeLog-2016 \
	ChangeLog-2017 \
	ChangeLog-2018 \
	ChangeLog-2019 \
	ChangeLog-2020 \
	ChangeLog-2021 \
	ChangeLog-2022 \
	ChangeLog-2023 \
	ChangeLog-2024 \
	ChangeLog-2025 \
	ChangeLog-2026 \
	$(NULL)

man1_MANS = \
	   vlc.1 \
	   vlc-wrapper.1

EXTRA_DIST = \
	$(CHANGELOGS) \
	$(man1_MANS) \
	$(LIBAPOI_SAMPLES) \
	fortunes.txt \
	release-howto.txt \
	Doxyfile.in \
	lirc/example.lircrc \
	skins/skins-howto.txt \
	skins/events-howto.txt \
	skins/curve_maker/Bezier.bas \
	skins/curve_maker/Bezier.frm \
	skins/curve_maker/Bezier.frx \
	skins/curve_maker/bezier.vbp \
	skins/curve_maker/Bezier.vbw \
	strings.txt \
	translations.txt \
	demo.sh \
	doc_helper.sh \
	$(NULL)

DISTCLEANFILES = Doxyfile
MAINTAINERCLEANFILES = $(CHANGELOGS)

Doxyfile: Doxyfile.in $(top_builddir)/config.status
	cd $(top_builddir) && $(SHELL) ./config.status --file=$(subdir)/$@

doc: Doxyfile
	doxygen

$(CHANGELOGS): Makefile.am
	if test -e "$(top_srcdir)/.git"; then \
		y="$@"; y="$${y##ChangeLog-}" ; \
		git --git-dir="$(top_srcdir)/.git" log \
			--since="$$y-01-01" \
			--until="$$y-12-31 23:00:00 -0100" \
			> "$@" ; \
	fi

# This one needs to be rebuilt all the time :)
.PHONY: ChangeLog-2024 changelogs doc

changelogs: $(CHANGELOGS)

#distclean-hook:
#	test "$(srcdir)" = "$(builddir)" || rm -f -- $(CHANGELOGS)
