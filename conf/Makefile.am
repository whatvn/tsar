AUTOMAKE_OPTIONS = foreign

confdir=/etc/tsar
conf_DATA= nagios.conf tsar.conf
confddir=/etc/tsar/conf.d
confd_DATA=
rotatedir=/etc/logrotate.d

rotate_DATA=tsar.logrotate

EXTRA_DIST=$(conf_DATA) $(rotate_DATA) tsar.8

man_MANS=tsar.8
