/*
 * (C) 2010-2011 Alibaba Group Holding Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "tsar.h"

struct statistic statis;
struct configure conf;
struct module	mods[MAX_MOD_NUM];

void usage()
{
	int i;
	struct module *mod;

	fprintf(stderr, 
			"Usage: tsar [options]\n"
			"Options:\n"
			"    --cron/-c		run in cron mode, output data to file\n"
			"    --interval/-i	specify intervals numbers, in minutes if with --live, it is in seconds\n"
			"    --list/-L		list enabled modules\n"
			"    --live/-l		running print live mode, which module will print\n"
			"    --ndays/-n		show the value for the past days (default: 1)\n"
			"    --merge/-m		merge multiply item to one\n"
			"    --help/-h		help\n");

	fprintf(stderr,
		"Modules Enabled:\n"
		);

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if(mod->usage) {
			fprintf(stderr, "%s", mod->usage);
			fprintf(stderr, "\n");
		}
	}

	exit(0);
}


struct option longopts[] = {
	{ "cron", no_argument, NULL, 'c' },
	{ "interval", required_argument, NULL, 'i' },
	{ "list", no_argument, NULL, 'L' },
	{ "live", no_argument, NULL, 'l' },
	{ "ndays", required_argument, NULL, 'n' },
	{ "merge", no_argument, NULL, 'm' },
	{ "help", no_argument, NULL, 'h' },
	{ 0, 0, 0, 0},
};


static void main_init(int argc, char **argv)
{
	int opt, oind = 0;
	while ((opt = getopt_long(argc, argv, ":ci:Lln:mh", longopts, NULL)) != -1) {
		oind++;
		switch (opt) {
			case 'c':
				conf.running_mode = RUN_CRON;
				break;
			case 'i':
				conf.print_interval = strtol(optarg,NULL,0);
				oind++;
				break;
			case 'L':
				conf.running_mode = RUN_LIST;	
				break;
			case 'l':
				conf.running_mode = RUN_PRINT_LIVE;
				break;
			case 'n':
				conf.print_ndays = strtol(optarg,NULL,0);
				oind++;
				break;
			case 'm':
				conf.print_merge  = MERGE_ITEM;
				break;
			case 'h':
				usage();
			case ':':
				printf("must have parameter\n");
				usage();
			case '?':
				if (argv[oind] && strstr(argv[oind], "--")) {
					strcat(conf.output_print_mod, argv[oind]);
					strcat(conf.output_print_mod, DATA_SPLIT);
				} else
					usage();
			default:
				break;
		}
	}

	/* set default parameter */
	if (!conf.print_ndays)
		conf.print_ndays = 1;

	if (!conf.print_interval)
		conf.print_interval = DEFAULT_PRINT_INTERVAL;

	if (RUN_NULL == conf.running_mode)
		conf.running_mode = RUN_PRINT;

	if (!strlen(conf.output_print_mod))
		conf.print_mode = DATA_SUMMARY;
	else
		conf.print_mode = DATA_DETAIL;

	strcpy(conf.config_file, DEFAULT_CONF_FILE_PATH);
	if (access(conf.config_file, F_OK)) {
		do_debug(LOG_FATAL, "main_init: can't find tsar.conf");
	}
}


void shut_down()
{
	free_modules();

	memset(&conf, 0, sizeof(struct configure));
	memset(&mods, 0, sizeof(struct module) * MAX_MOD_NUM);
	memset(&statis, 0, sizeof(struct statistic));
}


void running_list()
{
	int i;
	struct module *mod;

	printf("tsar enable follow modules:\n");

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		printf("	%s\n", mod->name + 4);
	}
}


void running_cron()
{
	/* output interface */
	//store to tsar.data by default
	/* output data */
	collect_record();
	output_file();

	if (strstr(conf.output_interface, "db")) {
		output_db();
	}
	if (strstr(conf.output_interface, "nagios")) {
		output_nagios();
	}
}


int main(int argc, char **argv)
{
	parse_config_file(DEFAULT_CONF_FILE_PATH);

	load_modules();
	
	statis.cur_time = time(NULL);
	
	main_init(argc, argv);

	/* 
	 * enter running
	 */
	switch (conf.running_mode) {
		case RUN_LIST:
			running_list();
			break;

		case RUN_CRON:
			conf.print_mode = DATA_DETAIL;
			running_cron();
			break;
		case RUN_PRINT:
			/* reload module by output_stdio_mod and output_print_mod*/
			reload_modules(conf.output_stdio_mod);
			reload_modules(conf.output_print_mod);
			/* disable module when n_col is zero */
			disable_col_zero();

			/* set conf.print_nline_interval */
			conf.print_nline_interval = conf.print_interval;

			running_print();
			break;

		case RUN_PRINT_LIVE:
			/* reload module by output_stdio_mod and output_print_mod*/
			reload_modules(conf.output_stdio_mod);
			reload_modules(conf.output_print_mod);
			
			/* disable module when n_col is zero */
			disable_col_zero();

			running_print_live();
			break;
		
		default:
			break;
	}

	shut_down();
	return 0;
}
