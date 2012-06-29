AUTOMAKE_OPTIONS = foreign
sbindir=${libdir}
sbin_PROGRAMS=mod_swap.so mod_partition.so mod_cpu.so mod_mem.so mod_traffic.so\
	mod_load.so mod_tcp.so mod_udp.so mod_tcpx.so mod_pcsw.so mod_io.so\
	mod_ts_client.so mod_ts_os.so mod_ts_cache.so mod_ts_err.so mod_ts_storage.so

mod_swap_so_SOURCES=mod_swap.c
mod_swap.so:
	gcc -I../../include -Wall -fPIC --shared -g  $(mod_swap_so_SOURCES) -o mod_swap.so
mod_partition_so_SOURCES=mod_partition.c
mod_partition.so:
	gcc -I../../include -Wall -fPIC --shared -g  $(mod_partition_so_SOURCES) -o mod_partition.so
mod_cpu.so:mod_cpu.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_cpu.c -o mod_cpu.so
mod_mem.so:mod_mem.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_mem.c -o mod_mem.so
mod_traffic.so:mod_traffic.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_traffic.c -o mod_traffic.so
mod_load.so:mod_load.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_load.c -o mod_load.so
mod_tcp.so:mod_tcp.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_tcp.c -o mod_tcp.so
mod_udp.so:mod_udp.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_udp.c -o mod_udp.so
mod_tcpx.so:mod_tcpx.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_tcpx.c -o mod_tcpx.so
mod_pcsw.so:mod_pcsw.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_pcsw.c -o mod_pcsw.so
mod_io.so:mod_io.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_io.c -o mod_io.so
mod_ts_client.so:mod_ts_client.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_ts_client.c -o mod_ts_client.so
mod_ts_os.so:mod_ts_os.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_ts_os.c -o mod_ts_os.so
mod_ts_err.so:mod_ts_err.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_ts_err.c -o mod_ts_err.so
mod_ts_cache.so:mod_ts_cache.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_ts_cache.c -o mod_ts_cache.so
mod_ts_storage.so:mod_ts_storage.c
	gcc -I ../../include -Wall -fPIC --shared -g mod_ts_storage.c -o mod_ts_storage.so
