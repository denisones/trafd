#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <errno.h>
#include "flowdata.h"
#include "fdtools.h"
#include <time.h>
#include <unistd.h>
#include "libpq-fe.h"
#include <sys/shm.h>
#include "tarif.h"
#include "mem.c"
#include "command.h"

/* signaly upravleniya */
#define NOT_SIGNAL 	0
#define KILL 		1
#define CONNECT 	2
#define DELETE 		3
#define CONFIG 		4
#define TARIFS 		5

int PORT = 1666;
struct sockaddr_in sa;
struct sockaddr_in remote_addr;
char drops_us[128];
int sockfd;
int sql_save;
int mbs_ = 1024;
float min_money;

int socketBufSize = DEF_SOCKET_BUFFER_SIZE;
// struct sockaddr_in socadr;

NOT_AVARIYA *n_av;
int sock_cl;
struct sockaddr_in sine;

char *flow_ptr = 0;
char bin_name[PATH_MAX];
ushort port = SERV_UDP_PORT;

int avari;
int ip1[4], ip2[4], ip3[4], ip4[4], pars[4], classtraf[4];
int ipt1, ipt2, ipt3, ipt4;
char c[100], time_sr[9];
char buf[1024];
int trcl;

FILE *llogf, *temp;
FILE *otchet;
char *pghost,
	*pgport,
	*pgoptions,
	*pgtty;
char *dbName;
PGconn *conn;
PGresult *res;
void exit_nicely(PGconn *conn);
void cleanUp(int i);
void sigHandler(int sig);
void GetFlow(int soc, struct sockaddr_in *socadr);
char t_s[22];

char *real_time()
{
	struct tm *time_k;
	time_t long_time;
	time(&long_time);
	time_k = localtime(&long_time);
	sprintf(t_s, "[%#02d/%#02d/%#02d %#02d:%#02d:%#02d]", time_k->tm_mday, time_k->tm_mon + 1, time_k->tm_year + 1900, time_k->tm_hour, time_k->tm_min, time_k->tm_sec);
	return t_s;
}

int load_config()
{
	char cf[128];
	FILE *conf = fopen("/usr/local/etc/billtrafd.conf", "r");
	if (conf != NULL)
	{
		while (fgets(cf, sizeof(cf), conf) != NULL)
		{
			sscanf(cf, "port CISCO = \"%i\"", &port);
			sscanf(cf, "port client = \"%i\"", &PORT);
			sscanf(cf, "NetFlow save = \"%i\"", &sql_save);
			sscanf(cf, "1 Mb = \"%i\"", &mbs_);
			sscanf(cf, "min money = \"%f\"", &min_money);
			if (strstr(cf, "stop script") != NULL)
			{
				strcpy(cf, 1 + strstr(cf, "\""));
				strncpy(drops_us, cf, strlen(cf) - 2);
				drops_us[strlen(cf) - 2] = '\000';
			}
		}
		fclose(conf);
		return 1;
	}
	else
	{
		return 0;
	}
}

int reload(int port, int i) // i==0 delete port, j=0 struct upravleniya, j=1 struct tarif
{
	if (i)
	{
		sa.sin_port = htons(port);
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			fprintf(llogf, "%s: socket() client comm failed, %s\n", bin_name, strerror(errno));
			cleanUp(1);
		}
		if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) < 0)
		{
			fprintf(llogf, "%s: bind() client comm failed, %s\n", bin_name, strerror(errno));
			cleanUp(1);
		}
	}
	else
	{
		close(sockfd);
	}
}

void exit_nicely(PGconn *conn)
{
	PQfinish(conn);
	exit(1);
}

drop_user(char ip_drop_user[16])
{
	char path[128];
	strcpy(path, drops_us);
	strcat(path, " ");
	strcat(path, ip_drop_user);
	system(path);
	return 0;
}

void cleanUp(i) int i;
{
	PQfinish(conn);
	if (i)
	{
		fprintf(llogf, "	Exiting\n");
	}
	free(flow_ptr);
	free_all_tarif();
	exit(i);
}

void sigHandler(sig) int sig;
{
	fprintf(llogf, "Received signal %i\n", sig);
	fflush(llogf);
	if (sig == 11)
	{
		fprintf(otchet, "%s ATTENTION!!! PROGRAM CORE!!! FOR RESTART INSERT COMMAND ./trafd restart\n", real_time());
		fflush(otchet);
	}
	// avar_("not",0);
	cleanUp(sig);
}

int calculat(CLT client, unsigned long long int octets, float cost, POROG_BYTE pb, POROG_TIME pt, int k, int i, int flag_)
{
	TRAF trafs;
	POROG_BYTE pb_temp;
	int j = 0;
	float m_r = 0, fl = -1;
	/* OBRABITKA VHODYACHEGO TRAFICA BEGIN */
	if (k == 2) // vhodyachi trafic in
	{
		client->total_in_byte += octets; // v5_flow->records[0].dOctets;
		trafs = client->trafic;
		for (j = 0; j < i; j++)
		{
			if (trafs->next != NULL)
			{
				trafs = trafs->next;
			}
			else
			{
				break;
			}
		}
		trafs->byte_in += octets;
		trafs->current_month += octets;
		pb_temp = pb;
		while (pb_temp != NULL) // poisk poroga 0;
		{
			if ((trafs->current_month <= pb_temp->do_porog) && (cost == 0))
			{
				fl = 1;
			}
			pb_temp = pb_temp->next;
		}
		if (fl != -1)
		{
			m_r = octets * (cost / (1024.0 * 1024.0));
			client->money -= m_r;
			trafs->summa += m_r;
			return 7;
		}
		while (pt != NULL)
		{
			if ((strcmp(pt->ot_h, time_sr) <= 0) && (strcmp(pt->dob_h, time_sr) >= 0))
			{
				m_r = octets * (pt->cost / (1024.0 * 1024.0));
				trafs->summa += m_r;
				client->money -= m_r;
				return 0;
			}
			pt = pt->next;
		}
		/* POROGI BYTE --------------------------------------------------------------- */
		while (pb != NULL)
		{
			if (flag_ == 1)
			{
				if (trafs->last_month > trafs->current_month)
				{
					if (trafs->last_month >= pb->do_porog)
					{
						m_r = octets * (pb->cost / (1024.0 * 1024.0));
						client->money -= m_r;
						trafs->summa += m_r;
						return 2;
					}
				}
				else
				{
					if (trafs->current_month >= pb->do_porog)
					{
						m_r = octets * (pb->cost / (1024.0 * 1024.0));
						client->money -= m_r;
						trafs->summa += m_r;
						return 5;
					}
				}
			}
			else
			{
				if (trafs->current_month >= pb->do_porog)
				{
					m_r = octets * (pb->cost / (1024.0 * 1024.0));
					client->money -= m_r;
					trafs->summa += m_r;
					return 6;
				}
			}
			pb = pb->next;
		}
		/* POROGI BYTE --------------------------------------------------------------- */
		if (((pb == NULL) || (pt == NULL))) // esli net porogov ili net nujnyh porogov ti scitaem po obshemu
		{
			m_r = octets * (cost / (1024.0 * 1024.0));
			client->money -= m_r;
			trafs->summa += m_r;
			return 3;
		}
	}
	/* OBRABOTKA VHODYACHEGO TRAFICA END */

	/* OBRABOTKA ISHODYACHEGO TRAFICA BEGIN */

	if (k == 1) // ishodyachii trafic
	{
		client->total_out_byte += octets;
		trafs = client->trafic;
		for (j = 1; j <= i; j++)
		{
			trafs = trafs->next;
		}
		trafs->byte_out += octets;
		return 4;
	}
	/* OBRABOTKA ISHODYACHEGO TRAFICA END */

	return -1;
}

int calc(CLT ca, CLASS_TRAF as, unsigned long long int oct, int ei, int io)
{
	calculat(ca, oct, as->cost, as->por_byte, as->por_time, ei, io, as->flag);
}

int calc_i(CLT ca, INET as, unsigned long long int oct, int ei, int io)
{
	calculat(ca, oct, as->cost, as->por_byte, as->por_time, ei, io, as->flag);
}

int bd_error(char buffers[1024])
{
	res = PQexec(conn, buffers);
	if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		if (!strstr((char *)res, "Table does not exist."))
		{
			return 1;
		}
		else
		{
			fprintf(llogf, "INSERT  command failed\n");
			fflush(llogf);
			if (res)
				PQclear(res);
			exit_nicely(conn);
			return 0;
		}
	}
	return 1;
}

query_radacct(char user[21], char id_k[32])
{
	int n, j;
	last = current = 0;
	sprintf(buf, "select acctinputoctets,acctoutputoctets from view_get_radacct where acctuniqueid='%s';", id_k);
	if (i = bd_error(buf))
	{
		n = PQnfields(res);
		for (i = 0; i < PQntuples(res); i++)
		{
			for (j = 0; j < n; j++)
				if (PQgetisnull(res, i, j) != 1)
				{
					if (j == 0)
						last = atol(PQgetvalue(res, i, j));
					if (j == 1)
						current = atol(PQgetvalue(res, i, j));
				}
				else
				{
					last = current = 0;
				}
		}
		PQclear(res);
		return 0;
	}
	else
	{
		fprintf(llogf, " \nBD ERROR! 34\n");
	}
}
save_data_on_bd_radacct(char id_k[32], char usern[20], TRAF trafik, char ip_u[16], int mk, char comme[40])
{
	strcpy(buf, "");
	if (mk != -1)
	{
		sprintf(buf, "select fn_insert_raddact_plus('%s', '%s', %lld, %lld, '%s', %i);", id_k, usern, trafik->byte_out, trafik->byte_in, ip_u, mk);
	}
	else
	{
		sprintf(buf, "select fn_insert_raddact_plus('%s', '%s', %lld, %lld, NULL, NULL);", id_k, usern, trafik->byte_out, trafik->byte_in);
	}

	if (i = bd_error(buf))
	{
		PQclear(res);
	}
	else
	{
		fprintf(llogf, " \nBD ERROR!1 user: %s %i\n", usern, i);
		fflush(llogf);
	}
}

get_money_user_q(usern, in_b, summ_bs, comme) char usern[20];
float summ_bs;
unsigned long long int in_b;
char comme[40];
{
	fprintf(llogf, "money %lld ,come : %s\n", in_b, comme);
	fflush(llogf);
	if (in_b != 0)
	{
		fprintf(llogf, "mone snyat user %s\n", usern);
		fflush(llogf);
		sprintf(buf, "select fn_ballans_getmoney('%s', fn_ballans_round_up(%f),' �� ����� ����� ����� '|| fn_ballans_round_up(%f) ||', %s ('|| round((%lld/(1024.0*1024.0)), 3) || ' mb.)' ,'system',1,'%.2f');", usern, summ_bs, summ_bs, comme, in_b, (summ_bs / ((in_b) / 1024.0 / 1024.0)));
		if (bd_error(buf))
		{
			PQclear(res);
		}
		else
		{
			fprintf(llogf, " \nBD ERROR! 2\n");
		}
	}
}

int restart_()
{
	TRAF tr_fs;
	char bf[128];
	char ok[10];
	char niks[20], id_kl[32];
	char ut[6] = "user/", ip_[16];
	float mn, spt;
	unsigned long long int in_, out_, l_mn, c_mn;
	int id_;
	char st[26];
	FILE *cli_fil = fopen("clients.log", "r");
	FILE *userf;
	if (cli_fil != NULL)
	{
		while (fgets(bf, sizeof(bf), cli_fil) != NULL)
		{
			strcpy(niks, "-1");
			sscanf(bf, "%s", &ok);
			if (strcmp(ok, "OK") == 0)
				return -2;
			sscanf(bf, "nick: %s", &niks);
			if (strcmp(niks, "-1") == 0)
				return 1;
			strcpy(st, ut);
			strcat(st, niks);
			if ((userf = fopen(st, "r")) != NULL)
			{
				while (fgets(bf, sizeof(bf), userf) != NULL)
				{
					id_ = -1;
					sscanf(bf, "nik: %s ip: %s money: %f spent: %f in: %lld out: %lld idtarif: %i id: %s", &niks, &ip_, &mn, &spt, &in_, &out_, &id_, &id_kl);
					if (id_ != -1)
					{
						if ((connects(niks, ip_, id_, mn, min_money, id_kl)) == 2)
						{
							if (clt != NULL)
							{
								clt->balans = (spt + mn);
								strcpy(clt->id_ks, id_kl);
								clt->total_in_byte = in_;
								clt->total_out_byte = out_;
								tr_fs = clt->trafic;
							}
						}
						else
						{
							tr_fs = NULL;
						}
					}
					in_ = 123;
					out_ = 321;
					sscanf(bf, "in: %lld out: %lld money: %f last month: %lld, current month: %lld", &in_, &out_, &mn, &l_mn, &c_mn);
					if ((in_ != 123) && (out_ != 321))
					{
						if (tr_fs != NULL)
						{
							tr_fs->last_month = l_mn;
							tr_fs->current_month = c_mn;
							tr_fs->byte_in = in_;
							tr_fs->byte_out = out_;
							tr_fs->summa = mn;
							tr_fs = tr_fs->next;
						}
					}
				}
				client_print(clt);
				if (llogf != NULL)
				{
					fprintf(llogf, "Vosstanovlen: %s\n", clt->nik);
					fflush(llogf);
					pinta_c();
				}
				fclose(userf);
			}
		}
	}
	else
	{
		return -1;
	}
}

func_last_month(char uzer[21], char mazk[16])
{
	int i, j, n;
	if (strcmp(mazk, "null") == 0)
	{
		sprintf(buf, "select fn_get_user_last_month_traffic_den ('%s', %s);", uzer, mazk);
	}
	else
	{
		sprintf(buf, "select fn_get_user_last_month_traffic_den ('%s', '%s');", uzer, mazk);
	}
	if (bd_error(buf))
	{
		n = PQnfields(res);
		// next, print out the rows
		for (i = 0; i < PQntuples(res); i++)
		{
			for (j = 0; j < n; j++)
				if (PQgetisnull(res, i, j) != 1)
				{
					last = atol(PQgetvalue(res, i, j));
				}
				else
				{
					last = 0;
				}
		}
	}
	PQclear(res);
	return 0;
}

func_current_month(char uzer[21], char mazk[16])
{
	int i, j, n;
	if (strcmp(mazk, "null") == 0)
	{
		sprintf(buf, "select fn_get_user_current_month_traffic_den ('%s', %s);", uzer, mazk);
	}
	else
	{
		sprintf(buf, "select fn_get_user_current_month_traffic_den ('%s', '%s');", uzer, mazk);
	}
	if (bd_error(buf))
	{
		n = PQnfields(res);
		// next, print out the rows
		for (i = 0; i < PQntuples(res); i++)
		{
			for (j = 0; j < n; j++)
				if (PQgetisnull(res, i, j) != 1)
				{
					current = atol(PQgetvalue(res, i, j));
				}
				else
				{
					current = 0;
				}
		}
	}
	PQclear(res);
	return 0;
}

void GetFlow(soc, socadr) int soc;
struct sockaddr_in *socadr;
{
	int cnt, alen, flowcount = 0, flowrec = 0, size, addrlen, k = 0, i = 0, j, /* for use*/ n;
	int quer_id_tar;
	float quer_money;
	ushort header_count = 0;
	struct in_addr saddr, daddr, *scaddr;
	struct tm *time_r;
	time_t long_time;
	long tmplong;
	char strptr1[16], strptr2[16], strptr3[16], nikname[20], ur[6] = "user/", us[26], time_mass[23];
	char strusr[20];
	IPStat5Msg *v5_flow = (IPStat5Msg *)flow_ptr;
	COMMAND c;
	alen = sizeof(struct sockaddr_in);
	fd_set fds;
	TARIF tarifs;
	CLT client = NULL;
	CLASS_TRAF class_t;
	TRAF trafs;
	POROG_BYTE pb;
	POROG_TIME pt;
	INET inet;
	FILE *users;
	//********** avarinyi zapusk
	// n_av = malloc (sizeof(NOT_AVARIYA));
	// memset(&sine,0, sizeof(sine));
	// sine.sin_family=AF_INET;
	// sine.sin_port = htons(1777);
	// inet_pton(AF_INET,"127.0.0.1",&sine.sin_addr);
	char cf[100];
	//********** avarinyi zapusk
	// base disconnected;
	conn = PQsetdb(pghost, pgport, pgoptions, pgtty, dbName);

	if (PQstatus(conn) == CONNECTION_BAD)
	{
		fprintf(llogf, "Connection to database '%s' failed.\n", dbName);
		fprintf(llogf, "%s", PQerrorMessage(conn));
		exit_nicely(conn);
	}
	addrlen = sizeof(remote_addr);

	//   FILE *otchet=fopen("report.log","a");
	/*----------------------------------------
	 * WAIT LOAD TARIF's
	 *----------------------------------------
	 */
	fprintf(llogf, " Wait load TARIF's\n", PORT);
	fflush(llogf);
	sprintf(buf, "SELECT * FROM view_get_all_tarifs where cost is not null;");
	if (bd_error(buf))
	{
		n = PQnfields(res);
		for (i = 0; i < PQntuples(res); i++)
		{
			for (j = 0; j < n; j++)
				if (PQgetisnull(res, i, j) != 1)
					add_tarif(PQgetvalue(res, i, j), j, PQfname(res, j));
				else
					add_tarif("-1", j, PQfname(res, j));
		}
		PQclear(res);
	}
	else
	{
		fprintf(llogf, "Ne mogu zagruzit TARIFS\n");
		fflush(llogf);
		cleanUp(1);
	}
	sort_tarif();
	pint_all();

	fprintf(llogf, " Load TARIF completed!\n");
	fflush(llogf);
	/*
	 * vosstanovlenie posle sboya
	 */
	int re_t;
	if (avari == 1)
	{
		switch (re_t = restart_())
		{
		case -1:
			fprintf(llogf, " No such file clients.log \n");
			break;
		case -2:
			fprintf(llogf, " Vse horoho \n");
			break;
		case 1:
			fprintf(llogf, " Clienty uspeshno vosstanovleny \n");
			break;
		default:;
		}
		pinta_c();
	}
	fflush(llogf);
	pinta_c();

	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		if (FD_ISSET(sockfd, &fds))
		{
			k = 1;
			time(&long_time);
			strcpy(time_mass, real_time());
			time_r = localtime(&long_time);
			sprintf(time_sr, "%#02d:%#02d:%#02d", time_r->tm_hour, time_r->tm_min, time_r->tm_sec);
			while (k > 0)
			{
				k = -1;
				if (recv(sockfd, &c, sizeof(COMMAND), MSG_DONTWAIT) != -1)
				{
					k = 1;
					/*----------------------------------------------------------------------
					 * 	BLOCK UPRAVLENIYA
					 *---------------------------------------------------------------------
					 */

					if (c.comm == 1) // command exit
					{
						client = clt;
						while (client != NULL)
						{
							if (dell_c(client->ip_client, time_mass))
								fprintf(otchet, "%s Disconnect user: %s\n", time_mass, client->nik);
							fflush(otchet);
							client = client->back;
						} // end while(client!=null)

						fflush(otchet);

						FILE *fr = fopen("clients.log", "w");
						if (fr != NULL)
						{
							fprintf(fr, "OK");
							fflush(fr);
							fclose(fr);
						}
						fprintf(otchet, "%s Command exit\n", time_mass);
						fflush(otchet);
						cleanUp(1);
					}
					if (c.comm == 2) // command connect
					{
						int err_cone;
						strcpy(nikname, c.nik);
						strcpy(strptr1, c.ip);
						{
							//-------------- zapros info about user in BD
							sprintf(buf, "select T.inet_id,B.money from userinfo1 as U left outer JOIN(select id,inet_id from ballans_tarifs)\
		     as T on T.id=U.tarif_id left outer JOIN(select money,username from ballans) as B on B.username = U.username where U.username = '%s';",
									nikname);
							if (bd_error(buf))
							{
								n = PQnfields(res);
								// next, print out the rows
								for (i = 0; i < PQntuples(res); i++)
								{
									for (j = 0; j < n; j++)
										if (PQgetisnull(res, i, j) != 1)
										{
											if (j == 0)
												quer_id_tar = atoi(PQgetvalue(res, i, j));
											if (j == 1)
												quer_money = atof(PQgetvalue(res, i, j));
										}
								}
								PQclear(res);
							}
							else
							{
								fprintf(otchet, "%s Receipt of the balance of user %s error\n", time_mass, nikname);
							}
							if ((err_cone = connects(nikname, strptr1, quer_id_tar, quer_money, min_money, c.id_key)) == 2)
							{
								fprintf(otchet, "%s Connect %s, %s, money: %f, ID tarif: %i\n", time_mass, nikname, strptr1, quer_money, quer_id_tar);
								fflush(otchet);
								pinta_c();
							}
							else
							{
								if (err_cone == -2)
								{
									fprintf(otchet, "%s Error allocating memory for the user! User %s can not be connected \n", time_mass, nikname);
									drop_user(strptr1);
								}
								if (err_cone == -1)
								{
									fprintf(otchet, "%s Tariff %i not found! User '%s' can not be connected \n", time_mass, quer_id_tar, nikname);
									drop_user(strptr1);
								}
								fflush(otchet);
							}
						}
					}
					if (c.comm == 3) // command delete
					{
						strcpy(strusr, user_search(c.ip));
						if (strcmp(strusr, "NULL") != 0)
						{
							if (dell_c(c.ip, time_mass /*,id_key*/) != -1)
							{
								fprintf(otchet, "%s Disconnect user: %s %s\n", time_mass, strusr, c.ip);
								fflush(otchet);
								pinta_c();
							}
							else
							{
								fprintf(otchet, "%s !!!!!! USER NOT DELETE! TARIF NOT FOUND: %s %s\n", time_mass, strusr, c.ip);
								fflush(otchet);
							}
						}
					}
					if (c.comm == 4) // command load config
					{
						char cf[100], cl[128];
						fprintf(llogf, " LOAD NEW PARAMETERS IN FILE CONFIG\n");
						fflush(llogf);
						if (load_config())
						{
							fprintf(otchet, "%s Configuration file load!\n", real_time());
							fflush(otchet);
						}
						else
						{
							port = 9991;
							PORT = 1667;
							sql_save = 0;
							min_money = 0.5;
							fprintf(llogf, "Configuration file billtrafd.conf not found!\n Option loaded by default\n \
		'Port NetFlow: %i\nPort client: %i\n NetFlow save= %i\nMin money=%f",
									port, PORT, sql_save, min_money);
							fflush(llogf);
						}
						fprintf(otchet, "%s Reload config\n", time_mass);
						fflush(otchet);
					}
					if (c.comm == 5) // command load tarif
					{
						fprintf(llogf, " LOAD NEW TARIF's\n");
						fflush(llogf);
						free_all_tarif();
						sprintf(buf, "SELECT * FROM view_get_all_tarifs where cost is not null;");
						if (bd_error(buf))
						{
							n = PQnfields(res);
							for (i = 0; i < PQntuples(res); i++)
							{
								for (j = 0; j < n; j++)
									if (PQgetisnull(res, i, j) != 1)
									{
										add_tarif(PQgetvalue(res, i, j), j, PQfname(res, j));
									}
									else
									{
										add_tarif("-1", j, PQfname(res, j));
									}
							}
							PQclear(res);
							sort_tarif();
							pint_all();
							fprintf(llogf, "        Load new traif completed!\n");
							fflush(llogf);
							fprintf(otchet, "%s Load new tarif\n", time_mass);
							fflush(otchet);
						}
						else
						{
							fprintf(otchet, "%s Error rates receipt from the database\n", time_mass);
						}
						fflush(llogf);
					}
					if (c.comm == 6)
					{
						// empty
					}

				} // end recv
			} // end while

		} // end if FD_SSET

		if (recvfrom(soc, flow_ptr, MAX_FLOW_PAK_SIZE, 0, (struct sockaddr *)socadr, &alen) < 0)
		{
			fprintf(llogf, "%s: recvfrom() failed, %s\n", bin_name, strerror(errno));
			cleanUp(1);
		}
		if (tarif != NULL)
			if ((client = clt) != NULL)
			{

				if ((ntohs(*(ushort *)flow_ptr)) == FLOW_VERSION_5)
				{

					header_count = ntohs(v5_flow->header.count);
					if (header_count > V5FLOWS_PER_PAK)
					{
						fprintf(llogf, "%s: Invalid flow count\n", bin_name);
					}
					else
					{
						flowrec += header_count;
						tmplong = ntohl(v5_flow->header.unix_secs);
						time_r = localtime(&tmplong);
						cnt = v5_flow->header.engine_type;
						if ((cnt != V5_RSP_EXPORT) || (cnt != V5_RSP_EXPORT))
						{
							fprintf(llogf, "Received a version 5 flow from %s, "
										   "with unknown engine_type %u\n",
									inet_ntoa(socadr->sin_addr), v5_flow->header.engine_type);
						}
						// FOR COUNTER

						for (cnt = 0; cnt < header_count; cnt++)
						{
							saddr.s_addr = v5_flow->records[cnt].srcaddr;
							daddr.s_addr = v5_flow->records[cnt].dstaddr;
							strcpy(strptr1, inet_ntoa(saddr));
							strcpy(strptr2, inet_ntoa(daddr));
							client = sort_back = clt;
							tarifs = tr_back = tarif;
							// class_t=NULL;
							k = -1;
							sscanf(strptr1, "%d.%d.%d.%d", &ip1[1], &ip2[1], &ip3[1], &ip4[1]);
							sscanf(strptr2, "%d.%d.%d.%d", &ip1[2], &ip2[2], &ip3[2], &ip4[2]);

							while (client > 0) // search CLIENTa in memory
							{

								sscanf(client->ip_client, "%d.%d.%d.%d", &ip1[3], &ip2[3], &ip3[3], &ip4[3]);
								if ((ip1[1] == ip1[3]) && (ip2[1] == ip2[3]) && (ip3[1] == ip3[3]) && (ip4[1] == ip4[3]))
								{
									k = 1;
									break; // out
								}
								if ((ip1[2] == ip1[3]) && (ip2[2] == ip2[3]) && (ip3[2] == ip3[3]) && (ip4[2] == ip4[3]))
								{
									k = 2;
									break; // in
								}
								sort_back = client;
								client = client->back;
							}
							sort_back = NULL;
							if (k > 0)
							{
								while (tarifs != NULL) // search TARIF clients po ID
								{
									if (client->id_tarif == tarifs->ID)
									{
										class_t = tarifs->trafs;
										tarif_sort(tarifs, tr_back);
										break;
									}
									tr_back = tarifs;
									tarifs = tarifs->back;
								}

								if (tarifs == NULL)
								{
									drop_user(client->ip_client);
									fprintf(otchet, "%s User %s:%s dropped. Reason: Tarif could not be found\n", time_mass, client->nik, client->ip_client);
									fflush(otchet);
									dell_c(client->ip_client, time_mass);
									class_t = NULL;
								}

								tr_back = NULL;
								if (k == 2)
								{
									sscanf(strptr1, "%d.%d.%d.%d", &ipt1, &ipt2, &ipt3, &ipt4);
								}
								if (k == 1)
								{
									sscanf(strptr2, "%d.%d.%d.%d", &ipt1, &ipt2, &ipt3, &ipt4);
								}
								if (class_t != NULL)
								{ // i=0;
									j = -4;
									for (i = 0; i < tarifs->class_count; i++)
									{
										if ((class_t->ip_min1 <= ipt1) && (class_t->ip_min2 <= ipt2) && (class_t->ip_min3 <= ipt3) && (class_t->ip_min4 <= ipt4) &&
											(class_t->ip_max1 >= ipt1) && (class_t->ip_max2 >= ipt2) && (class_t->ip_max3 >= ipt3) && (class_t->ip_max4 >= ipt4))
										{
											j = -3;
											calculat(client, ntohl(v5_flow->records[cnt].dOctets), class_t->cost, class_t->por_byte, class_t->por_time, k, i, class_t->flag);
											break;
										}
										class_t = class_t->next;
									}
								}
								else
								{
									j = -4;
								}
								if (tarifs->inet != NULL)
									if (j == -4)
									{
										inet = tarifs->inet;
										calculat(client, ntohl(v5_flow->records[cnt].dOctets), inet->cost, inet->por_byte, inet->por_time, k, tarifs->class_count, inet->flag);
									}
								client_print(client);
								tarifs = NULL;
								class_t = NULL;
								inet = NULL;

								if (client->money <= min_money)
								{
									if (client->drop == 1)
									{
										drop_user(client->ip_client);
										fprintf(otchet, "%s Drop User: %s %f \n", time_mass, client->nik, client->money);
										fflush(otchet);
										dell_c(client->ip_client, time_mass);
										pinta_c();
									}
								}
							} // end if user not searching
							// INSERT COMMAND
							client = NULL;
							if (sql_save == 1)
							{
								sprintf(buf, "INSERT INTO traffic (Day,Hour,\"Mon\",Year2,Min,Sec,SrcIP,DstIP,SPort,DPort,\
		    InInt,OutInt,Prot,Pkts,Octets,SrcAS,DstAS,trafclass) VALUES \
               	 ('%d','%d','%d','%d','%d','%d','%s','%s', %u, %u,'%u','%u','%u', %u, %u, %u, %u, %u)",
										time_r->tm_mday,
										time_r->tm_hour,
										time_r->tm_mon + 1,
										time_r->tm_year + 1900,
										time_r->tm_min,
										time_r->tm_sec,
										strptr1,
										strptr2,
										ntohs(v5_flow->records[cnt].srcport),
										ntohs(v5_flow->records[cnt].dstport),
										ntohs(v5_flow->records[cnt].input),
										ntohs(v5_flow->records[cnt].output),
										v5_flow->records[cnt].prot,
										ntohl(v5_flow->records[cnt].dPkts),
										ntohl(v5_flow->records[cnt].dOctets),
										ntohs(v5_flow->records[cnt].src_as),
										ntohs(v5_flow->records[cnt].dst_as),
										trcl);
								res = PQexec(conn, buf);
								if (res == NULL || PQresultStatus(res) != PGRES_COMMAND_OK)
								{
									if (!strstr((char *)res, "Table does not exist."))
									{
										//	goto insert;
									}
									else
									{
										fprintf(llogf, "INSERT  command failed\n");
										if (res)
											PQclear(res);
										exit_nicely(conn);
									}
								}
								PQclear(res);
								// INSERT COMMAND END
							}
						}
					}
				}
				else
				{
					// fprintf(logf, "%s: Received datagram with unknown or invalid "
					//                      "version number = %u\n",
					//          bin_name, ntohs(*((ushort *)flow_ptr)));
					//  cleanUp(1);
				}
			}
	} // end while(1)
}

void delsp(char *ch)
{
	char *src = ch;
	char *dst = ch;
	char c, lastc = '*';
	while ((c = *src) != 0)
	{
		if ((c != ' ') || (lastc != ' '))
		{
			*dst = c;
			dst++;
			lastc = c;
		}
		src++;
	}
	*dst = 0;
}
int div_l(unsigned long long int x, unsigned long long int y, unsigned long long int raz)
{
	double zl;
	char buf_d[128];
	zl = (double)x / (double)y * (double)raz;
	sprintf(buf_d, "%f", zl);
	sscanf(buf_d, "%lld.%lld", &divs.quot, &divs.rem);
	divs.quot += 1;
	return 0;
}

int main(int argc, char **argv)
{
	int soc, pid;
	FILE *flow;
	char s1[3], s2[3], s3[16], name_[10];
	llogf = fopen("/var/log/trafd.log", "a");
	lot = fopen("/var/log/trafd.log", "a");
	otchet = fopen("report.log", "a");
	fprintf(stdout, "See report.log\n");
	fprintf(otchet, "%s Start trafd...\n", real_time());
	fflush(otchet);
	struct sockaddr_in socadr;
	if (argc > 1)
		if (strcmp(argv[1], "restart") == 0)
		{
			avari = 1;
		}
	if (fork() < 0){
		fprintf(otchet, "%s fork fail", real_time());
		return 0;
	}
	if (setsid() < 0)
	{
		fprintf(stderr, "trafd: ");
		perror("cannot disassociate from controlling TTY");
		exit(1);
	}

	if ((flow = popen("ps ax | grep trafd | grep -v grep", "r")) != NULL)
	{
		while (fgets(buf, sizeof(buf), flow) != NULL)
		{
			delsp(buf);
			sscanf(buf, "%i %s %s %s %s", &pid, &s1, &s2, &s3, &name_);
			fprintf(llogf, "%s \n", buf);
			fflush(llogf);
			if (strcmp(name_, "./trafd") == 0)
			{
				if (getpid() != pid)
				{
					sprintf(buf, "kill -9 %i", pid);
					system(buf);
					fprintf(otchet, "%s Kill old copy program!\n", real_time());
					fflush(otchet);
					fprintf(llogf, "Kill old copy programm\n");
					fflush(llogf);
					avari = 1;
					break;
				}
			}
		} // else
	}
	pclose(flow);
	if (load_config())
	{
		printf("BillingTrafClass started! \n");
		fprintf(otchet, "%s Program started!\n", real_time());
		fflush(otchet);
	}
	else
	{
		port = 9991;
		PORT = 1667;
		sql_save = 0;
		min_money = 0.5;
		fprintf(llogf, "Configuration file billtrafd.conf not found!\n Option loaded by default\n \
		'Port NetFlow: %i\nPort client: %i\n NetFlow save= %i\nMin money=%f",
				port, PORT, sql_save, min_money);
		fflush(llogf);
	}
	pghost = NULL;
	pgport = NULL;
	pgoptions = NULL;
	pgtty = NULL;
	dbName = "radius";
	signal(SIGTERM, sigHandler);
	signal(SIGSEGV, sigHandler);

#ifdef __SVR4
	memset((char *)&socadr, 0, sizeof(struct sockaddr_in));
	memset(&sa, 0, sizeof(sa));
	memset(&remote_addr, 0, sizeof(remote_addr));
#else
	bzero((char *)&socadr, sizeof(struct sockaddr_in));
#endif
	socadr.sin_family = AF_INET;
	socadr.sin_addr.s_addr = htonl(INADDR_ANY);
	//--------------------
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	//-------------------
	socadr.sin_port = htons(port);

	flow_ptr = (char *)malloc(MAX_FLOW_PAK_SIZE);
	if ((soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		fprintf(llogf, "%s: socket() cisco failed, %s\n", bin_name, strerror(errno));
		cleanUp(1);
	}
	if (bind(soc, (struct sockaddr *)&socadr, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(llogf, "%s: bind() cisco failed, %s\n", bin_name, strerror(errno));
		cleanUp(1);
	}
	if (setsockopt(soc, SOL_SOCKET, SO_RCVBUF, (char *)&socketBufSize,
				   sizeof(socketBufSize)) < 0)
	{
		fprintf(llogf, "%s: setsockopt() failed, %s\n", bin_name,
				strerror(errno));
		cleanUp(1);
	}
	//--------------------------------------
	reload(PORT, 1);

	fprintf(llogf, " -------------------\n Listening on %u\n", port);
	fflush(llogf);
	GetFlow(soc, &socadr);
	return 0;
}
