
/*---------------------------------------------------
 *    raschet diapazona ip adresov po IP  i maske
 *    function mask_ip(char ip[15],int msk);
 *    ip_m1...ip_m4 - oktety min ip
 *    ip_x1...ip_x4 - oktety max IP
 *---------------------------------------------------
 */

int ip[8][4];
int maska[33];
int ip_min[8][4];
int ip_max[8][4];
int ip_m1, ip_m2, ip_m3, ip_m4;
int ip_x1, ip_x2, ip_x3, ip_x4;

int dec_to_bin(int i, int octet)
{
    int j = 7;
    while (i != 0)
    {
        ip[j][octet] = (i & 1);
        i = i >> 1;
        j--;
    }

    if (j != 0)
    {
        for (i = j; j >= 0; j--)
        {
            ip[j][octet] = 0;
        }
    }
}

int bin_to_dec_min()
{
    int i, j, result = 0;
    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 8; i++)
        {
            if (ip_min[i][j] == 1 && i == 0)
            {
                result = result + 128;
            }
            if (ip_min[i][j] == 1 && i == 1)
            {
                result = result + 64;
            }
            if (ip_min[i][j] == 1 && i == 2)
            {
                result = result + 32;
            }
            if (ip_min[i][j] == 1 && i == 3)
            {
                result = result + 16;
            }
            if (ip_min[i][j] == 1 && i == 4)
            {
                result = result + 8;
            }
            if (ip_min[i][j] == 1 && i == 5)
            {
                result = result + 4;
            }
            if (ip_min[i][j] == 1 && i == 6)
            {
                result = result + 2;
            }
            if (ip_min[i][j] == 1 && i == 7)
            {
                result = result + 1;
            }
        }
        switch (j)
        {
        case 0:
            ip_m1 = result;
        case 1:
            ip_m2 = result;
        case 2:
            ip_m3 = result;
        case 3:
            ip_m4 = result;
        }
        result = 0;
    }
}

int bin_to_dec_max()
{
    int i, j, result = 0;
    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 8; i++)
        {
            if (ip_max[i][j] == 1 && i == 0)
            {
                result = result + 128;
            }
            if (ip_max[i][j] == 1 && i == 1)
            {
                result = result + 64;
            }
            if (ip_max[i][j] == 1 && i == 2)
            {
                result = result + 32;
            }
            if (ip_max[i][j] == 1 && i == 3)
            {
                result = result + 16;
            }
            if (ip_max[i][j] == 1 && i == 4)
            {
                result = result + 8;
            }
            if (ip_max[i][j] == 1 && i == 5)
            {
                result = result + 4;
            }
            if (ip_max[i][j] == 1 && i == 6)
            {
                result = result + 2;
            }
            if (ip_max[i][j] == 1 && i == 7)
            {
                result = result + 1;
            }
        }
        switch (j)
        {
        case 0:
            ip_x1 = result;
        case 1:
            ip_x2 = result;
        case 2:
            ip_x3 = result;
        case 3:
            ip_x4 = result;
        }
        result = 0;
    }
}

int f_ip_max(int mask)
{
    int i, j, k = 0;
    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 8; i++)
        {
            k++;
            if (k <= mask)
            {
                ip_max[i][j] = ip[i][j];
            }
            else
            { // printf("\n %i",k);
                if (k == 32)
                {
                    ip_max[i][j] = 0;
                }
                else
                {
                    ip_max[i][j] = 1;
                }
            }
        }
    }
}

int f_ip_min(int mask)
{
    int i, j, k = 0;
    for (j = 0; j < 4; j++)
    {
        for (i = 0; i < 8; i++)
        {
            k++;
            if (k <= mask)
            {
                ip_min[i][j] = ip[i][j];
            }
            else
            {
                if (k == 32)
                {
                    ip_min[i][j] = 1;
                }
                else
                {
                    ip_min[i][j] = 0;
                }
            }
        }
    }
}

int mask_ip(char ips[15], int mask)
{
    int ip1, ip2, ip3, ip4, kq, sq;
    int i;
    sscanf(ips, "%i.%i.%i.%i", &ip1, &ip2, &ip3, &ip4);
    for (kq = 0; kq < 4; kq++)
        for (sq = 0; sq < 8; sq++)
            ip[sq][kq] = 0;
    dec_to_bin(ip1, 0);
    dec_to_bin(ip2, 1);
    dec_to_bin(ip3, 2);
    dec_to_bin(ip4, 3);
    for (i = 0; i < 32; i++)
    {
        if (i < mask)
        {
            maska[i] = 1;
        }
        else
        {
            maska[i] = 0;
        }
    }
    f_ip_min(mask);
    f_ip_max(mask);
    bin_to_dec_min();
    bin_to_dec_max();
}
/*
 */
