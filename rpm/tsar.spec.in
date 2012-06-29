Name: tsar
Version: @VERSION@
Release: 0
Summary: Taobao System Activity Reporter
URL: http://code.taobao.org/svn/tsar/trunk
Group: Taobao/Common
License: Apache License, Version 2.0
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Source: tsar-%{version}.tar.gz
#Requires: t-nsca-send

%description
tsar is Taobao monitor tool for collect system activity status, and report it.
It have a plugin system that is easy for collect plugin development. and may
setup different output target such as local logfile and remote nagios host.

%package devel
Summary: Taobao Tsar Devel
Group: Taobao/Common
Requires:tsar
%description devel
devel package include tsar header files and module template for the development

%prep
%setup -q

%build
./configure --prefix=/usr --libdir=%{_libdir}/tsar;make

%install

rm -rf %{buildroot}
mkdir -p %{buildroot}
make install DESTDIR=%{buildroot}


%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}


%files
%defattr(-,root,root)
%config(noreplace) /etc/tsar/tsar.conf
%config(noreplace) /etc/tsar/nagios.conf
%{_libdir}/tsar/*.so
/etc/cron.d/tsar.cron
/etc/tsar/conf.d
/etc/logrotate.d/tsar.logrotate
/usr/share/man/man8/*

%attr(755,root,root) %dir /usr/bin/tsar

%files devel
%defattr(-,root,root)
/usr/share/doc/tsar/tsar.h
/usr/share/doc/tsar/Makefile.test
/usr/share/doc/tsar/mod_test.c
/usr/share/doc/tsar/mod_test.conf
%attr(755,root,root) %dir /usr/bin/tsardevel

%changelog
* Wed May 11 2011 Ke Li <kongjian@taobao.com>
- add devel for module develop
* Thu Mar 22 2011 Ke Li <kongjian@taobao.com>
- add nagios conf and include conf support
* Thu Dec  9 2010 Ke Li <kongjian@taobao.com>
- add logrotate for tsar.data
* Tue Apr 26 2010 Bin Chen <kuotai@taobao.com>
- first create tsar rpm package
