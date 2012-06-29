#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "tsar.h"

/*
 * Structure for TS information
 */
struct stats_ts_cache {
        int hit;
        int ram_hit;
        int band;
};
//return value type
const static short int TS_REC_INT = 0;
const static short int TS_REC_COUNTER = 0;
const static short int TS_REC_FLOAT = 2;
const static short int TS_REC_STRING = 3;
//command type
const static short int TS_RECORD_GET = 3;
//records
const static int LINE_1024 = 1024;
const static int LINE_4096 = 4096;
const static char *RECORDS_NAME[]= {
        "proxy.node.cache_hit_ratio_avg_10s",
        "proxy.node.cache_hit_mem_ratio_avg_10s",
        "proxy.node.bandwidth_hit_ratio_avg_10s"
};
//socket patch
//const static char *sock_path = "/var/run/trafficserver/mgmtapisocket";
//const static char *sock_path = "/zserver/trafficserver/var/trafficserver/mgmtapisocket";

static char *ts_cache_usage = "    --ts_cache          trafficserver cache statistics";
int len, fd; 
static struct mod_info ts_cache_info[] = {
  {"   hit", DETAIL_BIT, 0, STATS_NULL},
  {"ramhit", DETAIL_BIT, 0, STATS_NULL},
  {"  band", DETAIL_BIT, 0, STATS_NULL}
};

void read_ts_cache_stats(struct module *mod)
{
  //int fd = -1;
  struct sockaddr_un un;
  struct stats_ts_cache st_ts;
  int pos;
  char buf[LINE_4096];
  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    goto done;
  }
  bzero(&st_ts, sizeof(st_ts));
  bzero(&un, sizeof(un));
  un.sun_family = AF_UNIX;
  strcpy(un.sun_path, conf.ts_mng_socket);
  len = strlen(un.sun_path) + sizeof(un.sun_family) ; 
  //if (connect(fd, (struct sockaddr *)&un, sizeof(un)) < 0) { 
  if (connect(fd, (struct sockaddr *)&un, len) < 0) { 
    printf("aaaaa");
    goto done;
  }

  int record_len = sizeof(RECORDS_NAME)/sizeof(RECORDS_NAME[0]);
  int i;
  const char *info;
  for ( i = 0; i < record_len; ++i) {
    info = RECORDS_NAME[i];
    long int info_len = strlen(info);
    short int command = TS_RECORD_GET;
    char write_buf[LINE_1024];
    *((short int *)&write_buf[0]) = command;
    *((long int *)&write_buf[2]) = info_len;
    strcpy(write_buf+6, info);
    write(fd, write_buf, 2+4+strlen(info));

    short int ret_status;
    long int ret_info_len;
    short int ret_type;
    int read_len = read(fd, buf, LINE_1024);
    if (read_len != -1) {
      ret_status = *((short int *)&buf[0]);
      ret_info_len = *((long int *)&buf[2]);
      ret_type = *((short int *)&buf[6]);
    }
    if (0 == ret_status) {
	float ret_val_float = *((float *)&buf[8]);
        ((int *)&st_ts)[i] = (int)ret_val_float * 1000;
    }
  }
done:
  if (-1 != fd)
    close(fd);
  pos = sprintf(buf, "%d,%d,%d",
                st_ts.hit,
                st_ts.ram_hit,
                st_ts.band
                );
  buf[pos] = '\0';
  set_mod_record(mod, buf);
}


void set_ts_cache_record(struct module *mod, double st_array[],
                             U_64 pre_array[], U_64 cur_array[], int inter)
{
  int i = 0;
  for (i = 0; i < 3; ++i) {
    st_array[i] = cur_array[i]/10;
  }
}



void mod_register(struct module *mod)
{
  register_mod_fileds(mod, "--ts_cache", ts_cache_usage, ts_cache_info, 3, read_ts_cache_stats, set_ts_cache_record);

}
