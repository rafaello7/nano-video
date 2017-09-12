UI =  nanovideowin.ui menubar.ui
IMG = nano-video.svg 

bin_PROGRAMS = nano-video

nano_video_SOURCES = nanovideo.c nanovideoapp.c nanovideowin.c \
					nanovideoapp.h nanovideowin.h \
					nanovideo.gresource.xml \
					$(UI) $(IMG)

nodist_nano_video_SOURCES = resources.c

nano_video_CFLAGS  = -Wall $(LIBGTK_CFLAGS) $(LIBGST_CFLAGS) \
					 $(LIBGSTVIDEO_CFLAGS)
nano_video_LDADD   = $(LIBGTK_LIBS) $(LIBGST_LIBS) $(LIBGSTVIDEO_LIBS)
nano_video_LDFLAGS = -rdynamic

EXTRA_DIST = nanovideo.gresource.xml

resources.c: nanovideo.gresource.xml $(UI) $(IMG)
	glib-compile-resources --sourcedir=$(srcdir) $< --generate-source \
		--target=$@

iconsdir = $(datadir)/icons/hicolor/scalable/apps

dist_icons_DATA = nano-video.svg

CLEANFILES = resources.c

