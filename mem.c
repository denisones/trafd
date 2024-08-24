
typedef struct clients
{						  // spisok kientov
	struct clients *back; // ukazatel na sledushi
	char ip_client[16];	  // ip clienta
	char nik[20];		  // ip napravlenya
	char id_ks[32];
	float money;
	float balans;
	unsigned long long int total_in_byte;  // summa byte k pol'zovatelyu
	unsigned long long int total_out_byte; // summa byte ot
	int id_tarif;
	short int drop;
	struct traf *trafic; // ukaztel na trafik
} *CLT;					 // ukazatel na spisok

CLT clt = NULL, oldc = NULL, sort_back;
// *********podumat nuzhno li
int count_cl = 0, k_fl = 0;
// *********podumat

typedef struct traf
{ // trafik
	int id;
	float summa;
	unsigned long long int last_month;	  //  za proshlyi
	unsigned long long int current_month; // za tekushi
	unsigned long long int byte_in;		  // prinyato cientom
	unsigned long long int byte_out;	  // peredano clientom
	struct traf *next;					  // ukazatel' na next
} *TRAF;

unsigned long long int last, current;

typedef struct delenie
{
	unsigned long long quot;
	unsigned long long rem;
} DELENIE;

DELENIE divs;

FILE *lot;

int ID_tarif_ins(int ID_tarif) /* prosmotr tarifa i vozrachenie kolichestva podkluchenii*/
{							  
	TARIF t = tarif;

	while (t > 0)
	{
		if (t->ID == ID_tarif)
		{
			return (t->class_count);
		}
		t = t->back;
	}

	return (0);
}
//**************** udalenii

char *user_search(char user[16])
{
	CLT us_cl = clt;
	while (us_cl != NULL)
	{

		if (strcmp(us_cl->ip_client, user) == 0)
			return us_cl->nik;

		us_cl = us_cl->back;
	}
	return "NULL";
}

int dell_c(char IP_c[16], char time_ms[21])
{
	CLT clts = clt, c_olds;
	TARIF rar = tarif;
	CLASS_TRAF ass_t, ass_2;
	INET inet_t;
	TRAF trafs1;
	TRAF trafs2, trafs3;
	char str1[16], us2[26], ur2[6] = "user/";
	FILE *user2;
	int ie = 0, i_ = 0;
	int io = 0;
	unsigned long long int total_unlim = 0;
	while (clts != NULL)
	{
		if (strcmp(IP_c, clts->ip_client) == 0)
		{

			while (rar != NULL) // poisk tarifa
			{
				if (clts->id_tarif == rar->ID)
					break;
				rar = rar->back;
			}
			if (rar == NULL)
			{
				return -1;
			}
			ass_t = rar->trafs;
			inet_t = rar->inet;
			divs.quot = 0;
			query_radacct(clts->nik, clts->id_ks);
			trafs1 = clts->trafic;
			int ch_clas_traf = 0;
			while (trafs1 != NULL)
			{
				ch_clas_traf++;
				trafs1 = trafs1->next;
			}
			trafs1 = clts->trafic;
			if (current != 0)
			{
				if (current != clts->total_in_byte)
				{
					if (current > clts->total_in_byte) // esli traffic bolshe
					{
						unsigned long long int raznica = current - clts->total_in_byte;
						unsigned long long int razn = last - clts->total_out_byte, total_in_b = clts->total_in_byte;
						while (trafs1 != NULL)
						{ 
							if (trafs1->byte_in != 0)
							{
								div_l(trafs1->byte_in, total_in_b, raznica);
							}
							else
							{
								divs.quot = 0;
							}
							fprintf(lot, "RAZNICA2: current %lld trafs: %lld total %lld razni: %lld div: %lld\n", current, trafs1->byte_in, clts->total_in_byte, raznica, divs.quot);
							fflush(lot);
							if ((ch_clas_traf - 1) == io)
							{
								ass_t = NULL;
							}
							if (ass_t != NULL)
							{
								calc(clts, ass_t, divs.quot, 2, io);
								i_ = -1;
								ass_t = ass_t->next;
							}
							else
							{
								if (inet_t != NULL)
								{
									if (rar->class_count != 0)
									{
										calc_i(clts, inet_t, divs.quot, 2, rar->class_count);
										i_ = -1;
									}
									else
									{
										calc_i(clts, inet_t, raznica, 2, rar->class_count);
										calc_i(clts, inet_t, razn, 1, rar->class_count);
										i_ = -1;
									}
								}
							}
							io++;
							trafs1 = trafs1->next;
						}

					}
					//***********************************************************
					inet_t = rar->inet;
					if ((current < clts->total_in_byte) && (i_ != -1) && (inet_t->cost == 0))
					{
						int count_f = 0;
						ass_t = rar->trafs;
						inet_t = rar->inet;
						io = 0;
						trafs1 = clts->trafic;
						i_ = 0;
						while (ass_t != NULL)
						{
							count_f += 1;
							ass_t = ass_t->next;
						}
						if (inet_t != NULL)
							count_f += 1;
						if (count_f > 0)
						{
							unsigned long long int total_unlim = clts->total_in_byte;
							unsigned long long int raznica = total_unlim - current;
							ass_t = rar->trafs;
							inet_t = rar->inet;
							fprintf(lot, "RAZNICA3: current %lld trafs: %lld total %lld razni: %lld\n", current, trafs1->byte_in, clts->total_in_byte, raznica);
							fflush(lot);
							io = 0;
							while (trafs1 != NULL)
							{
								if ((ch_clas_traf - 1) == io)
								{
									ass_t = NULL;
								}
								if (ass_t != NULL)
								{
									{
										if (trafs1->byte_in != 0)
										{
											div_l(trafs1->byte_in, total_unlim, raznica);
										}
										else
										{
											divs.quot = 0;
										}
										fprintf(lot, "RAZNICA unlima: current %lld trafs: %lld total %lld razni: %lld div: %lld\n", current, trafs1->byte_in, total_unlim, raznica, divs.quot);
										fflush(lot);
										trafs1->byte_in -= divs.quot;
										clts->total_in_byte -= divs.quot;
									}
									ass_t = ass_t->next;
								}
								else
								{
									if (inet_t != NULL)
									{
										if (inet_t->cost == 0)
										{
											if (rar->class_count != 0)
											{
												if (trafs1->byte_in != 0)
												{
													div_l(trafs1->byte_in, total_unlim, raznica);
													divs.quot += 1;
												}
												else
												{
													divs.quot = 0;
												}
												fprintf(lot, "RAZNICA unlim 2: current %lld trafs: %lld total %lld razni: %lld div: %lld\n", current, trafs1->byte_in, total_unlim, raznica, divs.quot);
												fflush(lot);
												trafs1->byte_in -= divs.quot;
												clts->total_in_byte -= divs.quot;
											}
											else
											{

												fprintf(lot, "RAZNICA unlim 3: current %lld trafs: %lld total %lld razni: %lld div: %lld\n", current, trafs1->byte_in, clts->total_in_byte, raznica, divs.quot);
												fflush(lot);
												trafs1->byte_in -= raznica;
												clts->total_in_byte -= raznica;
											}
										}
									}
								}
								io++;
								trafs1 = trafs1->next;
							}
						}
						else // if(count_f>0)
						{
							// empty
						}

					} // end if(current<clts->total_out_byte)

				} // if(current!=clts->total_in_byte)
			} // if(current!=0)
			fprintf(lot, "proshwl\n");
			fflush(lot);
			trafs1 = clts->trafic;
			strcpy(us2, ur2);
			strcat(us2, clts->nik);
			user2 = fopen(us2, "w");
			ass_t = ass_2 = rar->trafs;
			inet_t = rar->inet;
			unsigned long long int in_b = 0;
			float summa_b = 0;
			int circle_by, clas_number;
			char comments[40];

			while (trafs1 != NULL)
			{ 
				clas_number = trafs1->id;
				ass_t = rar->trafs;
				in_b = summa_b = 0;
				trafs3 = clts->trafic;
				in_b = summa_b = 0;
				strcpy(comments, "");
				if ((clas_number != -2) && (clas_number != 0))
				{
					fprintf(lot, "proshwl7\n");
					fflush(lot);
					while (ass_t != NULL)
					{ 
						if ((ass_t->id != -2) && (clas_number == ass_t->id))
						{
							trafs3->id = -2;
							in_b += trafs3->byte_in;
							summa_b += trafs3->summa;
							strcpy(comments, ass_t->comments);
						}
						ass_t = ass_t->next;
						trafs3 = trafs3->next;
						if (trafs3->next == NULL)
						{ /*circle_by++;*/
							break;
						}
					}
					get_money_user_q(clts->nik, in_b, summa_b, comments);
				}
				if (trafs1->next != NULL)
				{ 
					trafs1 = trafs1->next;
				}
				else
				{
					break;
				}
				if (trafs1->next == NULL)
				{
					break;
				}
			}
			if (inet_t != NULL)
			{
				get_money_user_q(clts->nik, trafs1->byte_in, trafs1->summa, inet_t->comments);
			}
			trafs1 = clts->trafic;
			ass_t = rar->trafs;
			inet_t = rar->inet;

			if (user2 != NULL)
			{
				fprintf(user2, "nik: %s ip: %s money: %f spent: %f in: %ld out: %ld idtarif: %i\n",
						clts->nik, clts->ip_client, clts->money, (clts->balans - clts->money), clts->total_out_byte, clts->total_in_byte, clts->id_tarif);
				fflush(user2);
			}
			io = 0;
			while (trafs1 != NULL)
			{ 
				if ((ch_clas_traf - 1) == io)
				{
					ass_t = NULL;
				}
				if (ass_t != NULL)
				{
					save_data_on_bd_radacct(clts->id_ks, clts->nik, trafs1, ass_t->ip_ch, ass_t->mask, ass_t->comments);
					ass_t = ass_t->next;
				}
				else
				{
					save_data_on_bd_radacct(clts->id_ks, clts->nik, trafs1, "", -1, inet_t->comments);
				}

				if (user2 != NULL)
				{
					fprintf(user2, "in: %lld out: %lld money: %f\n", trafs1->byte_in, trafs1->byte_out, trafs1->summa);
					fflush(user2);
				}

				io++;
				trafs2 = trafs1->next;
				free(trafs1);
				trafs1 = trafs2;
			}
			trafs1 = trafs2 = NULL;
			if (user2 != NULL)
			{
				fclose(user2);
				remove(us2); // ---------------------------------------------------- vostanovit
			}

			if (ie != 0)
			{
				c_olds->back = clts->back;
			}
			else
			{
				clt = clts->back;
				oldc = clt;
			}
			free(clts);
			clts = NULL;
			count_cl -= 1;
			fprintf(lot, "7\n");
			fflush(lot);
			if (count_cl == 0)
			{
				clt = oldc = NULL;
			}
			inet_t = NULL;
			trafs1 = NULL;
			trafs2 = NULL;
			ass_t = NULL;
			rar = NULL;
			clts = NULL;
			c_olds = NULL;
			fprintf(lot, "dell_c 1\n");
			fflush(lot);
			return 1;
		}
		c_olds = clts;
		ie++;
		clts = clts->back;
	}
	inet_t = NULL;
	trafs1 = NULL;
	trafs2 = NULL;
	ass_t = NULL;
	rar = NULL;
	clts = NULL;
	c_olds = NULL;
	fprintf(lot, "dell_c 2\n");
	fflush(lot);
	return 0;
}

int sort_client(CLT sort, CLT sr_back)
{ 
	if (sort == clt)
		return 1;

	sr_back->back = sort->back;
	sort->back = clt;
	clt = oldc = sort;
	sort = sr_back = NULL;
	return 2;
}

connects(char nick[20], char IP_c[16], int vID_tarif, float v_money, float mins_mon, char key_id[32]) // podkluchaem NEw client
{																									
	TRAF old_t = NULL, trf = NULL;
	TARIF tarif_all = tarif;
	CLASS_TRAF cl_af;
	INET internet;
	while (tarif_all != NULL)
	{
		if (tarif_all->ID == vID_tarif)
		{
			break;
		}
		tarif_all = tarif_all->back;
	}
	if (tarif_all == NULL)
		return (-1);
	if ((clt = malloc(sizeof(struct clients))) != NULL)
	{
		if (oldc != NULL)
		{
			clt->back = oldc;
		}
		else
		{
			clt->back = NULL;
		}
		cl_af = tarif_all->trafs;
		internet = tarif_all->inet;
		int k = tarif_all->class_count + 1, i; // poluchnie kol-va trafikov

		for (i = 0; i < k; i++)
		{
			old_t = trf; // vydelenie pamyti dlya trafika
			if ((trf = malloc(sizeof(struct traf))) != NULL)
			{
				if (old_t == NULL)
				{
					clt->trafic = trf;
				}
				else
				{
					old_t->next = trf;
				}

				if (cl_af != NULL)
				{ 
					trf->current_month = trf->last_month = last = 0;
					if (cl_af->flag == 1)
					{
						func_last_month(nick, cl_af->ip_ch);
						trf->last_month = last;
					}

					if (cl_af->por_byte != NULL)
					{
						func_current_month(nick, cl_af->ip_ch);
						trf->current_month = current;
					}
					trf->id = cl_af->id;
					cl_af = cl_af->next;
				}
				else
				{
					trf->last_month = trf->current_month = 0;
					if (internet != NULL)
					{
						if (internet->flag == 1)
						{
							func_last_month(nick, "null");
							trf->last_month = last;
						}

						if (internet->por_byte != NULL)
						{
							func_current_month(nick, "null");
							trf->current_month = current;
						}
					}
					trf->id = 0;
				}
				trf->byte_in = trf->byte_out = 0;
				trf->summa = 0;
				trf->next = NULL;
			}
		}
		old_t = trf = NULL;
		count_cl += 1;
		strcpy(clt->ip_client, IP_c);
		strcpy(clt->nik, nick);
		strcpy(clt->id_ks, key_id);
		clt->id_tarif = vID_tarif;
		clt->money = clt->balans = v_money;
		if (clt->money < mins_mon)
		{
			clt->drop = 0;
		}
		else
		{
			clt->drop = 1;
		}
		clt->total_in_byte = clt->total_out_byte = 0;
		client_print(clt);
		oldc = clt;
		return (2);
	}
	else
	{
		return (-2); // 2 error memory shared
	}
}

int client_print(CLT prin_cl)
{
	TRAF prin_tr;
	char ts2[26], tr2[6] = "user/";
	FILE *prin_f;

	strcpy(ts2, tr2);
	strcat(ts2, prin_cl->nik);
	prin_f = fopen(ts2, "w");
	if (prin_cl->trafic != NULL)
	{
		if (prin_f != NULL)
		{
			fprintf(prin_f, "nik: %s ip: %s money: %f spent: %f in: %lld out: %lld idtarif: %i id: %s\n",
					prin_cl->nik, prin_cl->ip_client, prin_cl->money, (prin_cl->balans - prin_cl->money), prin_cl->total_in_byte, prin_cl->total_out_byte, prin_cl->id_tarif, prin_cl->id_ks);
			fflush(prin_f);
		}
		prin_tr = prin_cl->trafic;
		while (prin_tr != NULL)
		{
			if (prin_f != NULL)
			{
				fprintf(prin_f, "in: %lld out: %lld money: %f last month: %lld, current month: %lld\n", prin_tr->byte_in, prin_tr->byte_out, prin_tr->summa, prin_tr->last_month, prin_tr->current_month);
				fflush(prin_f);
			}
			prin_tr = prin_tr->next;
		}
	}
	prin_cl = NULL;
	prin_tr = NULL;
	fclose(prin_f);
}

int pinta_c()
{ 
	CLT pclt = clt;
	FILE *f = fopen("clients.log", "w");
	while (pclt != NULL)
	{
		if (f != NULL)
		{
			fprintf(f, "nick: %s\n", pclt->nik);
			fflush(f);
		}
		pclt = pclt->back;
	}
	if (f != NULL)
	{
		fprintf(f, "Count: %i\n", count_cl);
		fflush(f);
		fflush(f);
		fclose(f);
	}
	pclt = NULL;
	return 0;
}
/*
main()
{ int i=0,t,err,k;
  char a,b[16];
  float f;
  FILE *temp;
  //key_t  key=1918;
//char *a="sdfsdf";
//  for(i=0;i<100;i++){// add();}
 //printf(" HANDLE %i\n",getpid("/trafd "));
 //if((chm= shmget( key, 16000, 0600)) < 0 ){perror( "shmget" );printf("no");}
 ///p=shmat(chm,NULL, SHM_RDONLY);
// strcpy(aa,p);
 //if(p==NULL){printf("ponty");}
 temp=fopen("/var/tmp/temp.trd","r");
 fscanf(temp,"%i",&k);
 printf(" vyvod %i \n",k);

 //shmdt(p);
 kill(k,2);
 while (a!='x'){
 //a=getchar();
 switch (a)
	{
	case 'a': { printf("add user. Enter from \"ip id_tarif money\": ");
		scanf("%s %i %f",&b,&t,&f);
	//	printf(" you entered %s %i %f \n",b,t,f);
		connects(b,t,f);
		printf("ADD user Completed!\n");
		break;
		}
	case 'd': { pinta_c();
		printf("Add user for deleted: ");
		scanf("%s",&b);
		dell_c(b);
		break;
		}
	case 'p':{ pinta_c();break;}
	default: {printf("Enter command: 'x' - exit 'a' - add IP user 'p' - print all 'd' - deleted user \n");break;}
	}
 a=getchar();
  }
/*
 dell_c("10.12.0.10");
 getchar();
 */
//}
