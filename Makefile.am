AM_CPPFLAGS = -I $(top_srcdir)/include
export AM_CPPFLAGS
SUBDIRS = src devel crontrib conf
	
EXTRA_DIST=rpm/tsar-build.sh  rpm/tsar.spec.in  rpm/tsar-VER.txt\
	include/common.h  include/config.h  include/debug.h include/define.h\
	include/framework.h  include/output_file.h  include/output_nagios.h \
	include/output_print.h  include/public.h  include/tsar.h

release_dir:
	@$(am__remove_distdir)
	test -d .git && git clone . $(distdir) || svn export . $(distdir)
	cd $(distdir) && autoreconf -i
	rm -rf $(distdir)/autom4te.cache $(distdir)/.git

release: release_dir
	tardir=$(distdir) && $(am__tar) | bzip2 -9 -c >$(distdir).tar.bz2
	@$(am__remove_distdir)

release-sign: release
	md5sum -b $(distdir).tar.bz2 >$(distdir).tar.bz2.md5
	sha1sum -b $(distdir).tar.bz2 >$(distdir).tar.bz2.sha1
	gpg --armor --output $(distdir).tar.bz2.asc  --detach-sig $(distdir).tar.bz2

