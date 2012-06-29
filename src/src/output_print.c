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

/*
 * adjust print opt line
 */
void adjust_print_opt_line(char *n_opt_line, char *opt_line, int hdr_len)
{
	char pad[LEN_128] = {0};
	int  pad_len;

	if (hdr_len > strlen(opt_line)) {
		pad_len = (hdr_len - strlen(opt_line))/2;

		memset(pad, '-', pad_len);
		strcat(n_opt_line, pad);
		strcat(n_opt_line, opt_line);
		memset(&pad, '-', hdr_len - pad_len - strlen(opt_line));
		strcat(n_opt_line, pad);
	} else
		strncat(n_opt_line, opt_line, hdr_len);
}


/*
 * print header and update mod->n_item
 */
void print_header()
{
	struct module *mod = NULL;
	char header[LEN_4096] = {0};
	char opt_line[LEN_1024] = {0};
	char hdr_line[LEN_1024] = {0};
	char opt[LEN_128] = {0};
	char n_opt[LEN_256] = {0};
	char mod_hdr[LEN_256] = {0};
	char *token, *s_token, *n_record;
	int i;

	sprintf(opt_line, "Time       %s", PRINT_SEC_SPLIT);
	sprintf(hdr_line, "Time       %s", PRINT_SEC_SPLIT);

	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->enable)	
			continue;

		memset(n_opt, 0, sizeof(n_opt));
		memset(mod_hdr, 0, sizeof(mod_hdr));
		get_mod_hdr(mod_hdr, mod);

		if (strstr(mod->record, ITEM_SPLIT) && MERGE_NOT == conf.print_merge) {
			n_record = strdup(mod->record);
			/* set print opt line */
			token = strtok(n_record, ITEM_SPLIT);
			while (token) {
				s_token = strstr(token, ITEM_SPSTART);
				if (s_token) {
					memset(opt, 0, sizeof(opt));
					memset(n_opt, 0, sizeof(n_opt));
					strncat(opt, token, s_token - token);
					adjust_print_opt_line(n_opt, opt, strlen(mod_hdr));
					strcat(opt_line, n_opt);
					strcat(opt_line, PRINT_SEC_SPLIT);
					strcat(hdr_line, mod_hdr);
					strcat(hdr_line, PRINT_SEC_SPLIT);
				}
				token = strtok(NULL, ITEM_SPLIT);
			}

			free(n_record);
			n_record = NULL;
		} else {
			memset(opt, 0, sizeof(opt));
			/* set print opt line */

			adjust_print_opt_line(opt, mod->opt_line, strlen(mod_hdr));

			/* set print hdr line */
			strcat(hdr_line, mod_hdr);
			strcat(opt_line, opt);
		}

		strcat(hdr_line, PRINT_SEC_SPLIT);
		strcat(opt_line, PRINT_SEC_SPLIT);
	}

	sprintf(header, "%s\n%s\n", opt_line, hdr_line);
	printf("%s", header);
}


void printf_result(double result)
{
	if ((1000 - result) > 0.1)
		printf("%6.1f", result);
	else if ( (1000 - result/1024) > 0.1) {
		printf("%5.1f%s", result/1024, "K");
	} else if ((1000 - result/1024/1024) > 0.1) {
		printf("%5.1f%s", result/1024/1024, "M");
	} else if ((1000 - result/1024/1024/1024) > 0.1) {
		printf("%5.1f%s", result/1024/1024/1024, "G");
	} else if ((1000 - result/1024/1024/1024/1024) > 0.1) {
		printf("%5.1f%s", result/1024/1024/1024/1024, "T");
	}
	printf("%s", PRINT_DATA_SPLIT);
}


void print_array_stat(struct module *mod, double *st_array)
{
	int i;
	struct mod_info *info = mod->info;

	for (i=0; i < mod->n_col; i++) {
		/* print null */	
		if (!st_array || !mod->st_flag) {
			/* print record */
			if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[i].summary_bit)) 
					|| ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[i].summary_bit)))
				printf("------%s", PRINT_DATA_SPLIT);
		} else {
			/* print record */
			if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[i].summary_bit)) 
					|| ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[i].summary_bit)))
				printf_result(st_array[i]);
		}
	}
}


/* print current time */
void print_current_time()
{
	char cur_time[LEN_32] = {0};
	time_t timep;
	struct tm *t;

	time(&timep);
	t = localtime(&timep);
	strftime(cur_time, sizeof(cur_time), "%d/%m-%R", t);
	printf("%s%s", cur_time, PRINT_SEC_SPLIT);
}


void print_record()
{
	struct module *mod = NULL;
	int  i, j;
	double *st_array;

	/* print summary data */
	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->enable)				
			continue;

		if (!mod->n_item) {
			print_array_stat(mod, NULL);
			printf("%s", PRINT_SEC_SPLIT);

		} else {
			for (j = 0; j < mod->n_item; j++) {
				st_array = &mod->st_array[j * mod->n_col];
				print_array_stat(mod, st_array);
				printf("%s", PRINT_SEC_SPLIT);
			}
			if (mod->n_item > 1)
				printf("%s", PRINT_SEC_SPLIT);
		}
	}
	printf("\n");
}


/* running in print live mode */
void running_print_live()
{
	int print_num = 1, re_p_hdr = 0;

	collect_record();

	/* print header */
	print_header();

	/* set struct module fields */	
	init_module_fields();

	/* skip first record */
	if(collect_record_stat() == 0)
		do_debug(LOG_INFO, "collect_record_stat warn\n");
	sleep(conf.print_interval);

	/* print live record */
	while (1) {
		collect_record();

		if (!((print_num) % DEFAULT_PRINT_NUM) || re_p_hdr) {
			/* get the header will print every DEFAULT_PRINT_NUM */
			print_header();
			re_p_hdr = 0;
			print_num = 1;
		}

		if (!collect_record_stat()) {
			re_p_hdr = 1;
			continue;
		}

		/* print current time */
		print_current_time();
		print_record();

		print_num++;
		/* sleep every interval */
		sleep(conf.print_interval);
	}
}



/* find where start printting */
void find_offset_from_start(FILE *fp,int number)
{
	char	line[LEN_4096] = {0};
	long	fset, fend, file_len, off_start, off_end, offset, line_len;
	char	*p_sec_token;
	time_t	now, t_token, t_get;

	/* get file len */
	fseek(fp, 0, SEEK_END);
	fend = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fset = ftell(fp);
	file_len = fend - fset;

	memset(&line, 0, LEN_4096);		
	if(!fgets(line, LEN_4096, fp)){
		do_debug(LOG_FATAL, "get line error\n");
	}
	line_len = strlen(line);

	/* get time token */
	time(&now);
	/* set max days for print 6 months*/
	if(conf.print_ndays > 180)
		conf.print_ndays = 180;
	now = now - now % (60 * conf.print_nline_interval);
	t_token = now - conf.print_ndays * (24 * 60 * 60);

	offset = off_start = 0;
	off_end = file_len;
	while (1) {
		offset = (off_start + off_end)/2;
		memset(&line, 0, LEN_4096);		
		fseek(fp, offset, SEEK_SET);
		if(!fgets(line, LEN_4096, fp)){
			do_debug(LOG_FATAL, "get line error\n");
		}
		memset(&line, 0, LEN_4096);		
		if(!fgets(line, LEN_4096, fp)){
			do_debug(LOG_FATAL, "get line error\n");
		}
		if (0 != line[0] && offset > line_len) {
			p_sec_token = strstr(line, SECTION_SPLIT);
			if (p_sec_token) {
				*p_sec_token = '\0';
				t_get = strtol(line,NULL,0);
				if (abs(t_get - t_token) <= 60) {
					conf.print_file_number = number;
					break;
				}

				/* Binary Search */
				if (t_get > t_token) {
					off_end = offset;
				}
				else if (t_get < t_token) {
					off_start = offset;
				}	
			}
			else {
				break;
			}
		}
		else
			break;
		if (offset == (off_start + off_end)/2)
			break;
	}
}


/* 
 * set and print record time 
 */
long set_record_time(char *line)
{
	char *token, s_time[LEN_32] = {0};
	static long pre_time, c_time = 0;

	/* get record time */
	token = strstr(line, SECTION_SPLIT);
	memcpy(s_time, line, token - line);

	/* swap time */
	pre_time = c_time;
	c_time = strtol(s_time,NULL,0);

	c_time = c_time - c_time%60;
	pre_time = pre_time - pre_time%60;
	/* if skip record when two lines haveing same minute */
	if (!(conf.print_interval = c_time - pre_time))
		return 0;
	else
		return c_time;
}

/* 
 * check time if corret for pirnt from tsar.data 
 */
int check_time(char *line)
{
	char *token, s_time[LEN_32] = {0};
	long now_time = 0;

	/* get record time */
	token = strstr(line, SECTION_SPLIT);
	memcpy(s_time, line, token - line);
	now_time = strtol(s_time,NULL,0);

	/* if time is divide by conf.print_nline_interval*/ 
	now_time = now_time - now_time % 60;
	if (!(now_time % ( 60 * conf.print_nline_interval)))
		return 0;
	else
		return 1;
}

void print_record_time(long c_time)
{
	char s_time[LEN_32] = {0};
	struct tm *t;

	t = localtime(&c_time);
	strftime(s_time, sizeof(s_time), "%d/%m-%R", t);
	printf("%s%s", s_time, PRINT_SEC_SPLIT);
}


void print_tail(int tail_type)
{
	struct module *mod = NULL;
	int  i, j, k;
	double *m_tail;

	switch (tail_type) {
		case TAIL_MAX:
			printf("MAX        %s", PRINT_SEC_SPLIT);
			break;
		case TAIL_MEAN:
			printf("MEAN       %s", PRINT_SEC_SPLIT);
			break;
		case TAIL_MIN:
			printf("MIN        %s", PRINT_SEC_SPLIT);
			break;
		default:
			return;
	}

	/* print summary data */
	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->enable)
			continue;
		switch (tail_type) {
			case TAIL_MAX:
				m_tail = mod->max_array;
				break;
			case TAIL_MEAN:
				m_tail = mod->mean_array;
				break;
			case TAIL_MIN:
				m_tail = mod->min_array;
				break;
			default:
				return;
		}

		k = 0;
		for (j = 0; j < mod->n_item; j++) {
			int i;
			struct mod_info *info = mod->info;
			for (i=0; i < mod->n_col; i++) {
				/* print record */
				if (((DATA_SUMMARY == conf.print_mode) && (SUMMARY_BIT == info[i].summary_bit)) 
						|| ((DATA_DETAIL == conf.print_mode) && (HIDE_BIT != info[i].summary_bit))) {
					printf_result(m_tail[k]);
				}
				k++;
			}
			printf("%s", PRINT_SEC_SPLIT);
		}
		if (mod->n_item != 1){
			if(!m_tail){
				print_array_stat(mod, NULL);
			}
			printf("%s", PRINT_SEC_SPLIT);
		}
	}
	printf("\n");
}


/*
 * init_running_print, if sucess then return fp, else return NULL
 */
FILE *init_running_print()
{
	int	i=0;
	FILE	*fp,*fptmp;
	char	line[LEN_4096] = {0};
	char	filename[LEN_128] = {0};

	/* will print tail*/
	conf.print_tail = 1;

	fp = fopen(conf.output_file_path, "r");
	if (!fp){
		do_debug(LOG_FATAL, "unable to open the log file\n");
	}
	/*log number to use for print*/
	conf.print_file_number = -1;
	/* find start offset will print from tsar.data */
	find_offset_from_start(fp,i);
	if(conf.print_file_number != 0){
		/*find all possible record*/
		for(i=1;;i++){
			memset(filename,0,sizeof(filename));
			sprintf(filename,"%s.%d",conf.output_file_path,i);
			fptmp = fopen(filename, "r");
			if(!fptmp){
				conf.print_file_number = i - 1;
				break;
			}
			find_offset_from_start(fptmp,i);
			fclose(fp);
			fp=fptmp;
			/*find start record for last ndays*/
			if(conf.print_file_number == i){
				break;
			}
			else{
				conf.print_file_number = i;
				continue;
			}
		}
	}

	/* get record */
	if (!fgets(line, LEN_4096, fp)) {
		do_debug(LOG_FATAL, "can't get enough log info\n");
	}

	/* read one line to init module parameter */
	read_line_to_module_record(line);

	/* print header */
	print_header();

	/* set struct module fields */	
	init_module_fields();

	if(set_record_time(line) == 0)
		do_debug(LOG_WARN, "line have same time.%s\n",line);
	return fp;
}


/*
 * print mode, print data from tsar.data
 */
void running_print()
{
	char	line[LEN_4096] = {0};
	char	filename[LEN_128] = {0};
	int	print_num = 1, re_p_hdr = 0;
	long	n_record = 0, s_time;
	FILE 	*fp;

	fp = init_running_print();

	/* skip first record */
	if(collect_record_stat() ==0)
		do_debug(LOG_INFO, "collect_record_stat warn\n");
	while (1) {
		if(!fgets(line, LEN_4096, fp)){
			if(conf.print_file_number <= 0)
				break;
			else{
				conf.print_file_number = conf.print_file_number - 1;
				memset(filename,0,sizeof(filename));
				if(conf.print_file_number == 0)
					sprintf(filename,"%s",conf.output_file_path);
				else
					sprintf(filename,"%s.%d",conf.output_file_path,conf.print_file_number);
				fclose(fp);
				fp = fopen(filename,"r");
				if(!fp){
					do_debug(LOG_FATAL, "unable to open the log file %s.\n",filename);
				}
				continue;
			}
		}
		if(check_time(line)){
			continue;
		}

		/* collect data then set mod->summary */
		read_line_to_module_record(line);

		if (!(print_num % DEFAULT_PRINT_NUM) || re_p_hdr) {
			/* get the header will print every DEFAULT_PRINT_NUM */
			print_header();
			re_p_hdr = 0;
			print_num = 1;
		}

		/* exclued the two record have same time */
		if (!(s_time = set_record_time(line)))
			continue;

		/* reprint header because of n_item's modifing */
		if (!collect_record_stat()) {
			re_p_hdr = 1;
			continue;
		}

		print_record_time(s_time);
		print_record();
		n_record++;
		print_num++;
		memset(line, 0, sizeof(line));
	}
	
	if (n_record) {
		printf("\n");
		print_tail(TAIL_MAX);
		print_tail(TAIL_MEAN);
		print_tail(TAIL_MIN);
	}

	fclose(fp);
	fp = NULL;
}
