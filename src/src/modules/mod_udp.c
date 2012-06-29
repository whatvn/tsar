#include "tsar.h"

#define UDP_DETAIL_HDR(d)			\
	"  idgm"d"  odgm"d"noport"d"idmerr"	

char *udp_usage =
	"    --udp               UDP traffic     (v4)";

/* Structure for UDP statistics */
struct stats_udp {
        unsigned long InDatagrams;
        unsigned long OutDatagrams;
        unsigned long NoPorts;     
        unsigned long InErrors;
};

#define STATS_UDP_SIZE      (sizeof(struct stats_udp))

void read_udp_stats(struct module *mod)
{
	FILE *fp;
        char line[LEN_1024];
	char buf[LEN_1024];
	memset(buf, 0, LEN_1024);
        int sw = FALSE;
	struct stats_udp st_udp;
	memset(&st_udp, 0, sizeof(struct stats_udp));
        if ((fp = fopen(NET_SNMP, "r")) == NULL) {
                return;
	}

        while (fgets(line, LEN_1024, fp) != NULL) {

                if (!strncmp(line, "Udp:", 4)) {
                        if (sw) {
                                sscanf(line + 4, "%lu %lu %lu %lu",
                                       &st_udp.InDatagrams,
                                       &st_udp.NoPorts,
                                       &st_udp.InErrors,
                                       &st_udp.OutDatagrams);
                                break;
                        }
                        else {
                                sw = TRUE;
                        }
                }
        }

        fclose(fp);

	int pos = sprintf(buf, "%ld,%ld,%ld,%ld",
			  st_udp.InDatagrams,
			  st_udp.NoPorts,
			  st_udp.InErrors,
			  st_udp.OutDatagrams);
	buf[pos] = '\0';
	set_mod_record(mod, buf);
}

static struct mod_info udp_info[] = {
        {"  idgm", DETAIL_BIT,  0,  STATS_SUB_INTER},
        {"  odgm", DETAIL_BIT,  0,  STATS_SUB_INTER},
        {"noport", DETAIL_BIT,  0,  STATS_SUB_INTER},
        {"idmerr", DETAIL_BIT,  0,  STATS_SUB_INTER},
};

void mod_register(struct module *mod)
{
	register_mod_fileds(mod, "--udp", udp_usage, udp_info, 4, read_udp_stats, NULL);
}

