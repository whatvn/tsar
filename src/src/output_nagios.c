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

#include <fcntl.h>
#include "tsar.h"


#define PRE_RECORD_FILE		"/tmp/.tsar.tmp"

/*
 */
int get_st_array_from_file()
{
	struct	module *mod;
	int  i, ret;
	char pre_line[LEN_4096] = {0};
	char line[LEN_4096] = {0};
	char detail[LEN_1024] = {0};
	char pre_time[32] = {0};
	char *s_token;
	FILE *fp;

	sprintf(line, "%ld", statis.cur_time);
	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (mod->enable && strlen(mod->record)) {
			memset(&detail, 0, sizeof(detail));
			/* save collect data to output_file */
			sprintf(detail, "%s%s%s%s", SECTION_SPLIT, mod->opt_line, STRING_SPLIT, mod->record);
			strcat(line, detail);
		}
	}
	strcat(line,"\n");
	
	
	/* if fopen PRE_RECORD_FILE sucess then store data to pre_record */
	if ((fp = fopen(PRE_RECORD_FILE, "r"))) {
		if (!fgets(pre_line, LEN_4096, fp)) {
			fclose(fp);
			ret = -1;
			goto out;	
		}
	} else {
		ret = -1;
		goto out;	
	}
	/* set print_interval */
	s_token = strstr(pre_line, SECTION_SPLIT);
	if (!s_token) {
		ret = -1;
		goto out;
	}
	memcpy(pre_time, pre_line, s_token - pre_line);	
	if (!(conf.print_interval = statis.cur_time - strtol(pre_time,NULL,0)))
		goto out;

	/* read pre_line to mod->record and store to pre_array */
	read_line_to_module_record(pre_line);
	init_module_fields();
	if(collect_record_stat() == 0)
		do_debug(LOG_INFO, "collect_record_stat warn\n");
	/* read cur_line and stats operation */
	read_line_to_module_record(line);
	if(collect_record_stat() == 0)
		do_debug(LOG_INFO, "collect_record_stat warn\n");
	ret = 0;

out:
	/* store current record to PRE_RECORD_FILE */
	if ((fp = fopen(PRE_RECORD_FILE, "w"))) {
		strcat(line,"\n");
		if(fputs(line, fp) < 0)
			do_debug(LOG_WARN, "write line error\n");
		fclose(fp);
		if(chmod(PRE_RECORD_FILE, 0666) < 0)
			do_debug(LOG_WARN, "chmod file %s error\n",PRE_RECORD_FILE);
	}
	
	return ret;
}


/*
 */
void send_sql_txt(int fd)
{
	struct	module *mod;
	char	sqls[LEN_4096] = {0};
	char	s_time[LEN_64] = {0};
	char	host_name[LEN_64] = {0};
	int	i = 0, j;

	/* get hostname */	
	if (0 != gethostname(host_name, sizeof(host_name))) {
		do_debug(LOG_FATAL, "send_sql_txt: gethostname err, errno=%d\n", errno);
	}
	while (host_name[i]) {
                if (!isprint(host_name[i++])) {
                        host_name[i-1] = '\0';
                        break;
                }
        }
	
	/* update module parameter */
	conf.print_merge = MERGE_ITEM;

	/* get st_array */
	if (get_st_array_from_file())
		return;
	
	/* only output from output_db_mod */
	reload_modules(conf.output_db_mod);

	sprintf(s_time, "%ld", time(NULL));

	/* print summary data */
	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->enable)
			continue;
		else if (!mod->st_flag) {
			char	sql_hdr[LEN_256] = {0};
			/* set sql header */
			memset(sql_hdr, '\0', sizeof(sql_hdr));
			sprintf(sql_hdr, "insert into `%s` (host_name, time) VALUES ('%s', '%s');", 
					mod->opt_line+2, host_name, s_time);
			strcat(sqls, sql_hdr);
		} else {
			char	str[LEN_32] = {0};
			char	sql_hdr[LEN_256] = {0};
			struct  mod_info *info = mod->info;

			/* set sql header */
			memset(sql_hdr, '\0', sizeof(sql_hdr));
			sprintf(sql_hdr, "insert into `%s` (host_name, time", mod->opt_line+2);
	
			/* get value */
			for (j = 0; j < mod->n_col; j++) {
				strcat(sql_hdr, ", ");
				strcat(sql_hdr, info[j].hdr);
			}
			strcat(sql_hdr, ") VALUES ('");
			strcat(sql_hdr, host_name);
			strcat(sql_hdr, "', '");
			strcat(sql_hdr, s_time);
			strcat(sql_hdr, "'");
			strcat(sqls, sql_hdr);

			/* get value */
			for (j = 0; j < mod->n_col; j++) {
				memset(str, 0, sizeof(str));
				sprintf(str, ", '%.1f'", mod->st_array[j]);
				strcat(sqls, str);	
			}
			strcat(sqls, ");");
		}
	}
	do_debug(LOG_DEBUG,"send to db sql:%s\n",sqls);
	if(write(fd, sqls, strlen(sqls)) < strlen(sqls)){
		do_debug(LOG_WARN, "send to db sql:'%s' error\n", sqls);
	}
}


struct sockaddr_in *str2sa(char *str)
{
	static	struct sockaddr_in sa;
	char	*c;
	int	port;

	memset(&sa, 0, sizeof(sa));
	str = strdup(str);
	if (str == NULL)
		goto out_nofree;

	if ((c = strrchr(str,':')) != NULL) {
		*c++ = '\0';
		port = strtol(c,NULL,0);
	}        
	else     
		port = 0;

	if (*str == '*' || *str == '\0') { /* INADDR_ANY */
		sa.sin_addr.s_addr = INADDR_ANY;
	}
	else if (!inet_pton(AF_INET, str, &sa.sin_addr)) {
		struct hostent *he;

		if ((he = gethostbyname(str)) == NULL) {
			do_debug(LOG_FATAL, "str2sa: Invalid server name, '%s'\n", str);
		}
		else
			sa.sin_addr = *(struct in_addr *) *(he->h_addr_list);
	}
	sa.sin_port   = htons(port);
	sa.sin_family = AF_INET;

	free(str);
out_nofree:
	return &sa;
}


void output_db()
{
	struct	sockaddr_in db_addr;
	int	fd, flags, res;
    	fd_set	fdr, fdw;
	struct	timeval timeout;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		do_debug(LOG_FATAL, "can't get socket\n");	
	}

	/* set socket fd noblock */
	if((flags = fcntl(fd, F_GETFL, 0)) < 0) {
		close(fd);
		return;
	}

	if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		close(fd);
		return;
	}

	/* get db server address */
	db_addr = *str2sa(conf.output_db_addr);

	if (connect(fd, (struct sockaddr*)&db_addr, sizeof(db_addr)) != 0) {
		if (errno != EINPROGRESS) { // EINPROGRESS
			close(fd);
			return;
		}
		else
			goto select;
	}
	else
		goto send;

select:
	FD_ZERO(&fdr);
	FD_ZERO(&fdw);
	FD_SET(fd, &fdr);
	FD_SET(fd, &fdw);

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	res = select(fd + 1, &fdr, &fdw, NULL, &timeout);
	if(res < 0) {
		close(fd);
		return;
	}
	if(res == 0) {
		close(fd);
		return;
	}

send:
	send_sql_txt(fd);
	close(fd);
}

void output_nagios(){
	struct	module *mod;
	int	result = 0;
	char	output[LEN_4096] = {0};
	char	output_err[LEN_4096] = {0};
	char	s_time[LEN_64] = {0};
	char	host_name[LEN_64] = {0};
	int	i = 0, j = 0, k = 0, l = 0;
	/* if cycle time ok*/
	int	now_time;
	now_time = statis.cur_time - statis.cur_time%60;
	if (now_time%*(conf.cycle_time) != 0)
		return;

	/* get hostname */	
	if (0 != gethostname(host_name, sizeof(host_name))) {
		do_debug(LOG_FATAL, "send to nagios: gethostname err, errno=%d \n", errno);
	}
	while (host_name[i]) {
                if (!isprint(host_name[i++])) {
                        host_name[i-1] = '\0';
                        break;
                }
        }

	/* update module parameter */
	conf.print_merge = MERGE_NOT;

	/* get st_array */
	if (get_st_array_from_file())
		return;
	
	/* only output from output_nagios_mod */
	reload_modules(conf.output_nagios_mod);

	sprintf(s_time, "%ld", time(NULL));

	/* print summary data */
	for (i = 0; i < statis.total_mod_num; i++) {
		mod = &mods[i];
		if (!mod->enable)
			continue;
		else if (!mod->st_flag) {
			printf("name %s\n",mod->name);
			printf("do nothing\n");
		}else {
			char opt[LEN_32];
			char check[LEN_64];
			char *n_record = strdup(mod->record);
			char *token = strtok(n_record, ITEM_SPLIT);
			char *s_token;
			double  *st_array;
			struct mod_info *info = mod->info;
			j = 0;
			//get mod_name.(item_name).col_name value
			while (token) {
				memset(check, 0, sizeof(check));
				strcat(check,mod->name+4);
				strcat(check,".");
				s_token = strstr(token, ITEM_SPSTART);
				//multi item
				if(s_token){
					memset(opt, 0, sizeof(opt));
					strncat(opt, token, s_token - token);
					strcat(check,opt);
					strcat(check,".");
				}
				//get value
				st_array = &mod->st_array[j * mod->n_col];
				token = strtok(NULL, ITEM_SPLIT);
				j++;
				for (k = 0; k < mod->n_col; k++) {
					char check_item[LEN_64];
					char *p;
					memset(check_item,0,LEN_64);
					memcpy(check_item,check,LEN_64);
					p = info[k].hdr;
					while(*p == ' ')
						p++;
					strcat(check_item,p);
					for(l = 0; l < conf.mod_num; l++){
						/* cmp tsar item with naigos item*/
						if(!strcmp(conf.check_name[l],check_item))
						{
							char value[LEN_32];
							memset(value,0,sizeof(value));
							sprintf(value,"%0.2f",st_array[k]);
							strcat(output,check_item);
							strcat(output,"=");
							strcat(output,value);
							strcat(output," ");
							if( conf.cmin[l] != 0 && st_array[k] >= conf.cmin[l] ){
								if( conf.cmax[l] == 0 || (conf.cmax[l] != 0 && st_array[k] <= conf.cmax[l]) ){
									result = 2;
									strcat(output_err,check_item);
									strcat(output_err,"=");
									strcat(output_err,value);
									strcat(output_err," ");
									continue;
								}
							}
							if( conf.wmin[l] != 0 && st_array[k] >= conf.wmin[l] ){
								if( conf.wmax[l] == 0 || (conf.wmax[l] != 0 && st_array[k] <= conf.wmax[l]) ){
									if( result != 2)
										result = 1;
									strcat(output_err,check_item);
									strcat(output_err,"=");
									strcat(output_err,value);
									strcat(output_err," ");
								}
							}
						}
					}
				}
			}
		}
	}
	if(!strcmp(output_err,""))
		strcat(output_err,"OK");
	/* send to nagios server*/
	char nagios_cmd[LEN_1024];
	sprintf(nagios_cmd,"echo \"%s;tsar;%d;%s|%s\"|%s -H %s -p %d -to 10 -d \";\" -c %s",host_name,result,output_err,output,conf.send_nsca_cmd,conf.server_addr,*(conf.server_port),conf.send_nsca_conf);
	do_debug(LOG_DEBUG,"send to naigos:%s\n",nagios_cmd);
	if(system(nagios_cmd) != 0)
		do_debug(LOG_WARN,"nsca run error:%s\n",nagios_cmd);;
	printf("%s\n",nagios_cmd);
}
