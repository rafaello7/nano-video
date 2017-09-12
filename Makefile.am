SUBDIRS = src debian

desktopdir = $(datadir)/applications
dist_desktop_DATA = nano-video.desktop

mimedir = $(datadir)/mime/packages
dist_mime_DATA = nano-video.xml

deb: dist
	rm -rf dpkg-deb
	mkdir dpkg-deb
	cd dpkg-deb && tar xf ../$(distdir).tar.gz
	cd dpkg-deb/$(distdir) && dpkg-buildpackage -us -uc

