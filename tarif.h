
/*-----------------------------------------------------------
 *	Struct loads tarif's v0.5 beta
 *	14/02/08
 *	structr ID TARIF-> class traffic -> porogi for byte
 *					 -> porogi for time
 *-----------------------------------------------------------
 */

#include "mask.c"

typedef struct load_tarif
{
	char name[5];
	int ar1;
	int ar2;
	int ar5;
	int ar6;
	char ar3[15];
	float ar4;
} LOAD_TARIF;

typedef struct porog_byte
{
	float cost;
	unsigned long long int do_porog;
	struct porog_byte *next;
} *POROG_BYTE;

typedef struct porog_time
{
	char ot_h[9];
	float cost;
	char dob_h[9];
	struct porog_time *next;
} *POROG_TIME;

typedef struct class_traf
{
	char ip_ch[16];
	int ip_min1;
	int ip_min2;
	int ip_min3;
	int ip_min4;

	int ip_max1;
	int ip_max2;
	int ip_max3;
	int ip_max4;

	int id;
	int mask;
	float cost;
	int flag;
	POROG_BYTE por_byte;
	POROG_TIME por_time;
	POROG_BYTE by_finish;
	POROG_TIME tm_finish;
	char comments[40];
	struct class_traf *next;
} *CLASS_TRAF;

typedef struct inets
{
	float cost;
	char comments[40];
	int flag;
	POROG_BYTE por_byte;
	POROG_TIME por_time;
	POROG_BYTE by_finish;
	POROG_TIME tm_finish;
} *INET;

typedef struct tarif
{
	int ID;
	float cost;
	int class_count;
	CLASS_TRAF trafs;
	INET inet;
	struct tarif *back;
} *TARIF;

LOAD_TARIF load_t;
TARIF tarif = NULL, t_old = NULL, tr_back;
CLASS_TRAF cl_old = NULL;
POROG_BYTE pb_old;
POROG_TIME pt_old;
INET inet;

int id_db, mask_db, flags, k_id; // <- this flag dlya ot dostignutogo
unsigned long long int mb_db;
char type_db[5], time1_db[9], time2_db[9], ip_db[16], comment[40];
float cost_db;

int i = 0;

int tarif_sort(TARIF sort, TARIF sr_back)
{
	if (sort == tarif)
		return 1;

	sr_back->back = sort->back;
	sort->back = tarif;
	tarif = sort;
	t_old = tarif;
	return 2;
}

int add_tarif(char *value, int circle, char nametb[255])
{
	FILE *logf;
	logf = fopen("/var/log/trafd.log", "a");
	if (circle >= 0)
	{
		if (/*circle==0*/ !strcmp(nametb, "id"))
		{
			id_db = atoi(value);
		}
		if (/*circle==1*/ !strcmp(nametb, "type"))
		{
			strcpy(type_db, value);
		}
		if (/*circle==2*/ !strcmp(nametb, "cost"))
		{
			cost_db = atof(value);
		}
		if (/*circle==3*/ !strcmp(nametb, "ot_mb"))
		{
			mb_db = atol(value);
		}
		if (/*circle==4*/ !strcmp(nametb, "ot_time"))
		{
			strcpy(time1_db, value);
		}
		if (/*circle==5*/ !strcmp(nametb, "do_time"))
		{
			strcpy(time2_db, value);
		}
		if (/*circle==6*/ !strcmp(nametb, "ip"))
		{
			strcpy(ip_db, value);
		}
		if (/*circle==7*/ !strcmp(nametb, "mask"))
		{
			mask_db = atoi(value);
		}
		if (/*circle==8*/ !strcmp(nametb, "comm"))
		{
			strcpy(comment, value);
		}
		if (/*circle==8*/ !strcmp(nametb, "klass_id"))
		{
			k_id = atoi(value);
		}
		if (/*circle==9*/ !strcmp(nametb, "flag"))
		{
			if (!(strcmp(value, "f")) || !(strcmp(value, "-1")))
				flags = 0;
			if (strcmp(value, "t") == 0)
				flags = 1;
		}
		if (circle < 10)
		{
			return circle;
		}
	}

	fprintf(logf, "id: %i   ", id_db);
	fflush(logf);
	fprintf(logf, "type: %s     ", type_db);
	fflush(logf);
	fprintf(logf, "cost: %.2f     ", cost_db);
	fflush(logf);
	fprintf(logf, "mb: %i     ", mb_db);
	fflush(logf);
	fprintf(logf, "time ot: %s     ", time1_db);
	fflush(logf);
	fprintf(logf, "time do: %s     ", time2_db);
	fflush(logf);
	fprintf(logf, "ip: %s     ", ip_db);
	fflush(logf);
	fprintf(logf, "mask: %i     ", mask_db);
	fflush(logf);
	fprintf(logf, "id: %i     \n", k_id);
	fflush(logf);

	CLASS_TRAF cl_t;
	POROG_BYTE pb;
	POROG_TIME pt;
	TARIF tarif_o;

	if (tarif != NULL)
	{
		tarif_o = tarif;
		while (tarif_o != NULL)
		{
			if (tarif_o->ID == id_db)
			{
				//-----------------------------------------------
				if (strcmp(type_db, "local") == 0)
				{
					if (tarif_o->trafs != NULL)
					{
						cl_t = tarif_o->trafs;
						while (cl_t != NULL)
						{
							if (strcmp(cl_t->ip_ch, ip_db) == 0) // elsi est' class to sozdaem porogi
							{
								if (mb_db != -1)
								{
									//--------create MB---------------------------------------------
									if ((pb = malloc(sizeof(struct porog_byte))) == NULL)
									{
										return -3;
									}
									if (cl_t->por_byte != NULL)
									{ // prisvoit k predyduschemu
										pb_old = cl_t->by_finish;
										pb_old->next = pb;
									}
									else
									{
										cl_t->por_byte = pb;
									}
									pb->do_porog = mb_db * 1024;
									pb->do_porog *= 1024;
									pb->cost = cost_db;
									pb->next = NULL;
									cl_t->by_finish = pb;
									return 0;
								}
								if (strcmp(time1_db, "-1") != 0)
								{
									//-------- create time ------------------------------------
									if ((pt = malloc(sizeof(struct porog_time))) == NULL)
									{
										return -4;
									}
									if (cl_t->por_time != NULL)
									{
										cl_t->tm_finish->next = pt;
									}
									else
									{
										cl_t->por_time = pt;
									}
									strcpy(pt->ot_h, time1_db);
									strcpy(pt->dob_h, time2_db);
									pt->cost = cost_db;
									pt->next = NULL;
									cl_t->tm_finish = pt;
									//-------- end create time ------------------------------------
									return 0;
								}
								cl_t->cost = cost_db;
								return 0;
							}
							if (cl_t->next == NULL)
								cl_old = cl_t;
							cl_t = cl_t->next;
						}
						//----------------------creat classs!!!
						if ((cl_t = malloc(sizeof(struct class_traf))) == NULL)
						{
							return -2;
						}
						cl_t->next = NULL;
						cl_t->por_byte = NULL;
						cl_t->por_time = NULL;
						cl_t->by_finish = NULL;
						cl_t->tm_finish = NULL;
						cl_t->flag = flags;
						if (tarif_o->trafs != NULL)
						{
							cl_old->next = cl_t;
						}
						else
						{
							tarif_o->trafs = cl_t;
							tarif_o->class_count = 0;
						}
						tarif_o->class_count += 1;
						if (strcmp(ip_db, "-1") != 0)
						{
							cl_t->mask = mask_db;
							ip_m1 = ip_m2 = ip_m3 = ip_m4 = ip_x1 = ip_x2 = ip_x3 = ip_x4 = 0;
							strcpy(cl_t->comments, comment);
							strcpy(cl_t->ip_ch, ip_db);
							mask_ip(cl_t->ip_ch, cl_t->mask);
							cl_t->cost = cost_db;
							cl_t->id = k_id;
							cl_t->ip_min1 = ip_m1;
							cl_t->ip_min2 = ip_m2;
							cl_t->ip_min3 = ip_m3;
							cl_t->ip_min4 = ip_m4;
							cl_t->ip_max1 = ip_x1;
							cl_t->ip_max2 = ip_x2;
							cl_t->ip_max3 = ip_x3;
							cl_t->ip_max4 = ip_x4;
						}
						cl_old = cl_t;
						add_tarif("", -1, "");
						return 0;
					}
					else // create class new-----------------------------------------------------------
					{
						if ((cl_t = malloc(sizeof(struct class_traf))) == NULL)
						{
							return -2;
						}
						cl_t->next = NULL;
						cl_t->por_byte = NULL;
						cl_t->por_time = NULL;
						cl_t->by_finish = NULL;
						cl_t->tm_finish = NULL;
						cl_t->flag = flags;
						if (tarif_o->trafs != NULL)
						{
							cl_old->next = cl_t;
						}
						else
						{
							tarif_o->trafs = cl_t;
							tarif_o->class_count = 0;
						}
						tarif_o->class_count += 1;
						if (strcmp(ip_db, "-1") != 0)
						{
							cl_t->mask = mask_db;
							strcpy(cl_t->comments, comment);
							strcpy(cl_t->ip_ch, ip_db);
							mask_ip(cl_t->ip_ch, cl_t->mask);
							cl_t->id = k_id;
							cl_t->cost = cost_db;
							cl_t->ip_min1 = ip_m1;
							cl_t->ip_min2 = ip_m2;
							cl_t->ip_min3 = ip_m3;
							cl_t->ip_min4 = ip_m4;
							cl_t->ip_max1 = ip_x1;
							cl_t->ip_max2 = ip_x2;
							cl_t->ip_max3 = ip_x3;
							cl_t->ip_max4 = ip_x4;
						}
						cl_old = cl_t;
						add_tarif("", -1, "");
						return 0;
					} // end creat new tarif----------------------------------------------------------
				}
				//-----------------------------------------------
				if (strcmp(type_db, "inet") == 0)
				{
					inet = tarif_o->inet;
					//-------creat inet-------------------
					if (tarif_o->inet != NULL)
					{
						//************************************************* INET POROG
						if (mb_db != -1)
						{
							//--------create MB---------------------------------------------
							if ((pb = malloc(sizeof(struct porog_byte))) == NULL)
							{
								return -3;
							}
							if (inet->por_byte != NULL)
							{ // prisvoit k predyduschemu
								pb_old = inet->by_finish;
								pb_old->next = pb;
							}
							else
							{
								inet->por_byte = pb;
							}
							pb->do_porog = mb_db * 1024;
							pb->do_porog *= 1024;
							;
							pb->cost = cost_db;
							pb->next = NULL;
							inet->by_finish = pb;
							inet->flag = flags;
							// fprintf(logf," inet cretate mb hwd %i\n",pb);fflush(logf);
							return 0;
							//-------end create MB-------------------------------------------------
						}
						if (strcmp(time1_db, "-1") != 0)
						{
							//-------- create time ------------------------------------
							if ((pt = malloc(sizeof(struct porog_time))) == NULL)
							{
								return -4;
							}
							if (inet->por_time != NULL)
							{
								inet->tm_finish->next = pt;
							}
							else
							{
								inet->por_time = pt;
							}
							strcpy(pt->ot_h, time1_db);
							strcpy(pt->dob_h, time2_db);
							pt->cost = cost_db;
							pt->next = NULL;
							inet->tm_finish = pt;
							//-------- end create time ------------------------------------
							return 0;
						}
						inet->cost = cost_db;
						return 0;
						// 88888888888888888888 END INET POROG
					}
					else
					{
						if ((inet = malloc(sizeof(struct inets))) == NULL)
						{
							return -5;
						}
						tarif_o->inet = inet;
						inet->flag = flags;
						if (mb_db != -1 && strcmp(time1_db, "-1") != 0)
						{
							add_tarif("", -1, "");
							return 0;
						}
						strcpy(inet->comments, comment);
						inet->por_byte = NULL;
						inet->por_time = NULL;
						inet->cost = cost_db;
						return 0;
					}

					//-------end creat inet-------------------
				}
				//-----------------------------------------------
			}
			tarif_o = tarif_o->back;
			circle = -2;
		}
		//----------------------------------------------------
		{
			if ((tarif = malloc(sizeof(struct tarif))) == NULL)
			{
				return -1;
			}
			if (t_old != NULL)
			{
				tarif->back = t_old;
			}
			else
			{
				tarif->back = NULL;
			}

			tarif->ID = id_db;
			tarif->class_count = 0;
			tarif->trafs = NULL;
			tarif->inet = NULL;
			t_old = tarif_o = tarif;
			add_tarif("", -1, "");
			return 0;
		}
	}
	else
	{
		/* CREAT NEW TARIF */
		if ((tarif = malloc(sizeof(struct tarif))) == NULL)
		{
			return -1;
		}
		if (t_old != NULL)
		{
			tarif->back = t_old;
		}
		else
		{
			tarif->back = NULL;
		}
		tarif->ID = id_db;
		tarif->class_count = 0;
		tarif->trafs = NULL;
		tarif->inet = NULL;
		t_old = tarif_o = tarif;
		add_tarif("", -1, "");
		return 0;
	}
}

int sort_tarif()
{
	TARIF t = tarif;
	CLASS_TRAF ca = NULL;
	INET ins = NULL;
	POROG_BYTE pb2 = NULL, pb3_n = NULL;
	int i, j, k;
	// unsigned long meg;
	unsigned long long int mbb;
	float mon;
	FILE *l = fopen("/var/log/trafd.log", "a");
	while (t != NULL)
	{
		ca = t->trafs;
		while (ca != NULL)
		{
			i = 0;
			if ((pb2 = ca->por_byte) != NULL)
				while (pb2 > 0)
				{
					i++;
					pb2 = pb2->next;
				}
			if (i > 1)
			{
				for (j = 0; j < i; j++)
				{
					pb2 = ca->por_byte;
					pb3_n = pb2->next;
					for (k = 0; k < i; k++)
					{
						if ((pb2 == NULL) || (pb3_n == NULL))
							break;
						if (pb2->do_porog < pb3_n->do_porog)
						{
							mbb = pb2->do_porog;
							mon = pb2->cost;
							pb2->do_porog = pb3_n->do_porog;
							pb2->cost = pb3_n->cost;
							pb3_n->do_porog = mbb;
							pb3_n->cost = mon;
						}
						pb2 = pb2->next;
						if (pb2 == NULL)
							break;
						pb3_n = pb3_n->next;
					}
				}
			}
			ca = ca->next;
		}
		if ((ins = t->inet) != NULL)
		{
			i = 0;
			if ((pb2 = ins->por_byte) != NULL)
			{
				while (pb2 != NULL)
				{
					i++;
					pb2 = pb2->next;
				}
				if (i > 1)
				{
					for (j = 0; j <= i; j++)
					{
						pb2 = ins->por_byte;
						pb3_n = pb2->next;
						for (k = 0; k <= i; k++)
						{
							if ((pb2 == NULL) || (pb3_n == NULL))
								break;
							if (pb2->do_porog < pb3_n->do_porog)
							{
								mbb = pb2->do_porog;
								mon = pb2->cost;
								pb2->do_porog = pb3_n->do_porog;
								pb2->cost = pb3_n->cost;
								pb3_n->do_porog = mbb;
								pb3_n->cost = mon;
							}
							pb2 = pb2->next;
							pb3_n = pb3_n->next;
							if (pb3_n == NULL)
								break;
						}
					}
				}
			}
		}

		t = t->back;
	}
}

int pint_all()
{
	TARIF t = tarif;
	CLASS_TRAF c1; //=tarif->trafs;
	POROG_TIME pt_;
	POROG_BYTE pb_;
	INET inet;
	FILE *l = fopen("/var/log/trafd.log", "a");

	while (t != NULL)
	{
		c1 = t->trafs;
		if (l != NULL)
		{
			fprintf(l, "TARIF ID: %i Count Class: %i \n", t->ID, t->class_count);
			fflush(l);
		}
		// do
		while (c1 != NULL)
		{
			if (l != NULL)
			{
				fprintf(l, "	class ip: %s mask: %i cost: %.10f 'ot dostignutogo'%i  ID=%i\n", c1->ip_ch, c1->mask, c1->cost, c1->flag, c1->id);
				fprintf(l, "	ip min: %i.%i.%i.%i ip max: %i.%i.%i.%i \n", c1->ip_min1, c1->ip_min2, c1->ip_min3, c1->ip_min4, c1->ip_max1, c1->ip_max2, c1->ip_max3, c1->ip_max4);
				fflush(l);
			}
			pt_ = c1->por_time;
			pb_ = c1->por_byte;
			while (pt_ != NULL)
			{
				fprintf(l, "	time ot '%s'-'%s' cost:%f\n", pt_->ot_h, pt_->dob_h, pt_->cost);
				fflush(l);
				pt_ = pt_->next;
			}
			while (pb_ != NULL)
			{
				fprintf(l, "	byte do %lld cost %f\n", pb_->do_porog, pb_->cost);
				fflush(l);
				pb_ = pb_->next;
			}

			c1 = c1->next;
		}
		if (t->inet != NULL)
		{
			inet = t->inet;
			pt_ = inet->por_time;
			pb_ = inet->por_byte;

			fprintf(l, "INET cost:%f Ot dostignutot: %i\n", inet->cost, inet->flag);

			while (pt_ != NULL)
			{
				fprintf(l, "	INET TIME '%s'-'%s' cost:%f\n", pt_->ot_h, pt_->dob_h, pt_->cost);
				fflush(l);
				pt_ = pt_->next;
			}
			while (pb_ != NULL)
			{
				fprintf(l, "	INET BYTE do %lld cost %f\n", pb_->do_porog, pb_->cost);
				fflush(l);
				pb_ = pb_->next;
			}
		}
		t = t->back;
	}
	t = NULL;
	c1 = NULL;
	pt_ = NULL;
	pb_ = NULL;
	fflush(l);
	return 0;
}

int free_all_tarif()
{
	CLASS_TRAF c2; //,c_old;
	POROG_TIME pt; //,pt_old;
	POROG_BYTE pb; //,pb_old;
	while (tarif != NULL)
	{
		t_old = tarif->back;
		c2 = tarif->trafs;
		while (c2 != NULL)
		{
			cl_old = c2->next;
			pt = c2->por_time;
			while (pt != NULL)
			{
				pt_old = pt->next;
				free(pt);
				pt = pt_old;
			}
			pt_old = NULL;
			c2->por_time = NULL;
			pb = c2->por_byte;
			while (pb != NULL)
			{
				pb_old = pb->next;
				free(pb);
				pb = pb_old;
			}
			pb_old = NULL;
			c2->por_byte = NULL;
			free(c2);
			c2 = cl_old;
		}
		cl_old = NULL;
		tarif->trafs = NULL;
		if (tarif->inet != NULL)
		{
			inet = tarif->inet;
			pt = inet->por_time;
			pb = inet->por_byte;
			while (pt > 0)
			{
				pt_old = pt->next;
				free(pt);
				pt = pt_old;
			}
			pt_old = NULL;
			while (pb > 0)
			{
				pb_old = pb->next;
				free(pb);
				pb = pb_old;
			}
			pb_old = NULL;
			inet->by_finish = NULL;
			free(inet);
			inet = NULL;
		}
		free(tarif);

		tarif = t_old;
	}
	tarif = NULL;
	t_old = NULL;
	cl_old = NULL;
	pb_old = NULL;
	pt_old = NULL;
}
