/****************************************************
**			this file is a part of the progrem	   **
**		defined a class for showing grisys data	   **
**		updated 2001.3.17 by cjh				   **
**		updated 2001.12.16 by wyx				   **
****************************************************/
//#include "stdafx.h"
//#include "fb.h"
//#include "fbdoc.h"
#include "gridataadi.h"
//#include "string.h"
griData::griData()
{
    init();
    wsam = 1;
}

qint32 griData::open_read(QString file,qint32 gather,qint32 maxTrs)
{
    return open_read(file.toUtf8().data(),gather,maxTrs);
}

qint32 griData::open_read(const char *file,qint32 grp_id,qint32 max_grps)
{
    /* flag   1: open error;
    		  2: length = 0;
              3: read error;
              4: not grisys data;
              5: no index file;
    		  6: INDEX_OPEN_ERR 6
              7: INDEX_FILE_LENGTH_ERR 7
              8: INDEX_READ_ERR     8
              9: INDEX_TEMP_ALLOC_ERR 9
             10: INDEX_PTR_GROUP_ALLOC_ERR 10
             11: INDEX_GROUP_ID_NOT_IN_IFILE     11
    */
//    int iie;
//    init();
    group_id = grp_id;
    MAX_GROUPS = max_grps;

    set_msg_err();
    // init();

// open file
if(bDEBUG)
    qDebug() <<"file =" <<  file << grp_id << max_grps;

    dfile=new QFile(file);
//	printf("after----inti--\n");
    if (!dfile->open(QFile::ReadOnly))
    {
//		printf("----in if open----\n");
        flag = OPEN_ERR;
        return flag;
    }
// set File_Name;
    strcpy_s(FileName,file);
// get file length
    if (dfile->size() == 0L)
    {
        flag = LENGTH_ERR;
        return flag;
    }
    else
    {
//get data information
//printf("000--file length=%d\n",dfile->size());
        iie = 0;
        iie = get_data_info();
//printf("222--file length=%d\n",dfile->size());
        switch (iie)
        {
        case 0:
            break;
        case -1:
            flag = READ_ERR;
            return flag;
        case -2:
            flag = NOT_GRISYS;
            return flag;
        }


        trace = new float[sam+10];

//printf("444----inti--\n");
        if(swap_flag == FMT12)
            itrace = new qint16[sam+10];
        else
            ltrace = new quint32[sam+10];
// about ifile;
        bool b;
        b = is_ifile_exists(file);

        if(!b)
        {
            flag=NO_INDEX;
            return flag;
        }
//get ifile information
//printf("----get_index_info----\n");
        iie = get_index_info();
//printf("111----get_index_info--iie=%d\n",iie);

        switch(iie)
        {
        case -1:
            flag = INDEX_OPEN_ERR;
            return flag;
        case -2:
            flag = INDEX_FILE_LENGTH_ERR;
            return flag;
        case -3:
            flag = INDEX_READ_ERR;
            return flag;
        case -4:
            flag = INDEX_TEMP_ALLOC_ERR;
            return flag;
        case -5:
            flag =INDEX_PTR_GROUP_ALLOC_ERR;
            return flag;
        case -6:
            flag =INDEX_GROUP_ID_NOT_IN_IFILE;
            return flag;
        }
    }
//printf("-------------END--------\n");
// init same numbers;
    /*	trace = new float(sam*sizeof(float));

    	if(swap_flag == FMT12)
                    itrace = new quint32(bytes);
    	else
                    ltrace = new quint32(bytes);
    */

//printf("2222-------------END--------flag=%d\n",flag);
    return flag;
}

void griData::close_read()
{

    flag = 0;
    iie = 0;
// init numbers

    swap_flag = 0;
    shift_num = 0;

    max_cs = 0;
    min_cs = 30000;
    all_trs = 0;
    all_groups =0;
    sam =1;
    wsam = 1;
    current_group_idx =0;

//    set_msg_err();

// open file
    //ebug() << "dfile=" << dfile <<dfile->handle();
    dfile->close();
    //ebug() << "dfile=" << dfile <<dfile->handle();
#if 0 //  why error?
  //if(dfile != NULL) delete dfile;
#endif

    if(ifile != NULL)
        if(ifile->handle() !=-1)
        {
            ifile->close();
            if(ifile != NULL) delete ifile;
        }

    if(trace !=NULL) delete []trace;
    if(itrace !=NULL) delete []itrace;
    if(ltrace !=NULL) delete []ltrace;
    if(listGroup !=NULL) delete []listGroup;
//	if(head != NULL) delete []head;
//	if(listGroup !=NULL) delete []listGroup;
    return ;
}
void griData::SwapByte1( qint16 *data_2byte, qint32 trLen)
{
    qint32 i;
    char *c;
    char t;
    c = NULL;
    if(COMPUTER == SUN_C) return;
    for(i =0; i<trLen; i++)//=2)
    {
        c=(char *)&data_2byte[i];


        t=c[0];
        c[0] = c[1];
        c[1]= t;
    }
}
// swap
void griData::SwapByte( qint16 *data_2byte, qint32 trLen)
{
    qint32 i;
    char *c;
    char t;
    qint16 tmp0;
    if(COMPUTER == SUN_C) return;
    for(i =0; i<trLen; i+=2)
    {
        tmp0 = data_2byte[i];

        data_2byte[i]=data_2byte[i+1];
        data_2byte[i+1] = tmp0;

        c=(char *)&data_2byte[i];
        t=c[0];
        c[0] = c[1];
        c[1]= t;

        c=(char *)&data_2byte[i+1];
        t=c[0];
        c[0] = c[1];
        c[1]= t;
    }
}

void griData::SwapByte(qint32 * data_4byte, qint32 len)
{
    qint32 i;
    qint8 j;
    char *it;
    it = NULL;
    if(COMPUTER == SUN_C) return;
//sprintf(str,"shot=%lX,format=%lx",data_4byte[1],data_4byte[54]);
    for(i =0; i<len; i++)
    {
        it =(char *)&data_4byte[i];
        j=it[0];
        it[0] = it[3];
        it[3]=j;
        j=it[1];
        it[1] = it[2];
        it[2]=j;
    }

}
void griData::SwapByte(quint32 * data_4byte, qint32 len)
{
    qint32 i;
    qint8 j;
    char  *it;
    it = NULL;
//sprintf(str,"shot=%lX,format=%lx",data_4byte[1],data_4byte[54]);
    if(COMPUTER == SUN_C) return;
    for(i =0; i<len; i++)
    {
        it =(char *)&data_4byte[i];
        j=it[0];
        it[0] = it[3];
        it[3]=j;
        j=it[1];
        it[1] = it[2];
        it[2]=j;
    }

}
//-----------------------------------------------------
/* read  data file first header
    and set some numbers:ltr,si,sam,format
    finally point the file potinter to beginning*/
qint32  griData::get_data_info()
{
//return : 0: ok
//        -2: not grisys data
//      -1: read error
    qint32 i;
// read header
    if(!dfile->isOpen())
    {
        //printf("ERROR: The file is not opened.\n");
        return -1;
    }
//printf("in get_data_info--dfile at:=%d\n",dfile->at());
    i = dfile->read((char *)head,128*sizeof(qint32));
//printf("in get_data_info--i=%d,qint32=%d\n",i,128*sizeof(qint32));

    if (i != 128*sizeof(qint32))
        return -1;
//printf("head[55]=%d\n",head[FORMAT-1]);
    if(head[FORMAT-1] <1 || head[FORMAT-1] >5)
        SwapByte(head,128);
    if(head[FORMAT-1] <1 || head[FORMAT-1] >5)
        return -2;
// set info
    file_len = dfile->size();
    si = (qint32) head[SI-1];
    ltr =(qint32) head[TRACE_LTR-1];
    sam =(qint32)  ltr/si;

//printf("in get_data_info--ltr=%d,si=%d\n",ltr,si);
    format =(qint32)  head[FORMAT - 1];
    if(head[FORMAT-1] == 1 || head[FORMAT-1] == 2)
    {
        swap_flag =FMT12 ;
//nt4.0
        if( head[121] != 0 )
        {
            if( sam/2*2 != sam ) sam++;
        }
        else
        {
            if( sam/2*2 != sam ) sam--;
        }
//--------------------------------------------------
        bytes = sam *sizeof(qint16);
    }
    else  if(head[FORMAT-1] ==3 )
    {
        swap_flag = FMT3;
        bytes = sam * sizeof(qint32);
    }
    else if(head[FORMAT-1] ==4 )
    {
        bytes=sam*sizeof(qint32);
        swap_flag = FMT4;
    }
    else if(head[FORMAT-1] ==5 )
    {
        bytes=sam*sizeof(qint32);
        swap_flag = FMT5;
    }
//printf("%d\n",head[FORMAT-1]);
// if FMT12 and bytes can not integer divided by 4;
//nt4.0 qyh 99.1
    /*	if(bytes != bytes/4*4)
    	{
    		sam --;
    		bytes = bytes/4*4;
    	}                         */
//=======================================
    all_trs = file_len / (512+ bytes) ;
//printf("-2002---file_len=%d,bytes=%d,all_trs=%d\n",file_len,bytes,all_trs);
    dfile->seek(0);
    if(bDEBUG)
    {
        qDebug() << "data info ===============";
        qDebug() << "si=" << si;
        qDebug() << "ltr=" << ltr;
        qDebug() << "format=" << format;
        qDebug() << "sam=" << sam;
        qDebug() << "all_trs=" << all_trs;
        qDebug() << "file_len=" << file_len;
    }

    return 0;

}
//------------------------------------------
// get IFileName  according to FileName;
// return 0:  ok
//
bool griData::is_ifile_exists(QString file)
{
    return is_ifile_exists(file.toUtf8().data());
}
bool griData::is_ifile_exists(const char *pszPathName)
{
     QString str;
     str  = get_ifile_name(pszPathName);
     QFile file(str);
     return file.exists();

}
QString griData::get_ifile_name(QString file)
{
    return get_ifile_name(file.toUtf8().data());
}
QString griData::get_ifile_name(const char *pszPathName)
{
    //return QString
    // if I file is not exist return idx file
    char str[FILEN_LEN];
    QString ff;
    qint32 len;
    strcpy_s(str,pszPathName);
//-----------------------------

//qyh add
    len = strlen( str );
    *(str+len-8)='I';
//------------------------
    QString filen;
    filen = str;
    QFile file(filen);
    if( !file.exists())
    {
        strcpy_s(str,pszPathName);
        strcat_s(str,".idx");
    }
    else
    {
        //
    }

    ff = str;
    strcpy(IFileName,str);
    return ff;
}

/*//------------------------------------------
// get information
// IFileName  acording to FileName;
// return 0:  ok
//       -1: idx filen open error
		 -2: index file get length err;
		 -3: idx file read err;
		 -4  idx tmp allocation err
		 -5  idx listGroup allocation err
		 -6  the group id is not in the ifile header;
*/
qint32 griData::get_index_info()
{
    qint32 i,ict,c_g,ic,igp;
    qint32 itmpidx,lsize;
// open index file
    ifile=new QFile(IFileName);
    if (!ifile->open(QFile::ReadOnly))
        return -1;

// get the number of traces of this data
// HSQ add
    //all_trs = ifile->size() / IDX_ITEM_BYTES ;
    if(all_trs <= 0) return -2;


    switch(group_id)
    {
    case SHOT:
        c_g = 1;
        break;
    case CMP:
        c_g = 2;
        break;
    case STATION:
        c_g = 4;
        break;
    case OFFSET:
        c_g = 5;
        break;
    case LINE:
        c_g = 6;
        break;
    case CELL:
        c_g = 7;
        break;
    default:
        return -6;
    }

    PTR_GROUP *prt_gp_tmp;
    lsize = MAX_GROUPS;
    prt_gp_tmp = new   PTR_GROUP[lsize];
    if(prt_gp_tmp == NULL)
    {
        return -4;
    }

    ict = 0;
    ic = 0;
    max_trs_per_group = 0;

// get group information
    for(i = 0; i < all_trs ; i++)
    {
        if(ifile->read((char *)Itr,IDX_ITEM_BYTES) != IDX_ITEM_BYTES)
            return -3;
        SwapByte(Itr, IDX_ITEMS);
        if(i == 0)
        {
            itmpidx = 0;
            igp = (qint32) Itr[c_g];
            current_group = igp;
        }
        if(igp != (qint32) Itr[c_g])
        {
            prt_gp_tmp[ict].idx = itmpidx;
            prt_gp_tmp[ict].grp = igp;
            prt_gp_tmp[ict].num = ic;

            itmpidx = Itr[0]-1;
            igp = (qint32) Itr[c_g];
            ict ++;
            if(ict > MAX_GROUPS) return -5;
            if(ic > max_trs_per_group) max_trs_per_group = ic;
            ic = 0;
        }
        ic++;
    }

    if(ic > max_trs_per_group) max_trs_per_group = ic;
    ict ++;
    if(ict > MAX_GROUPS) return -5;
    prt_gp_tmp[ict-1].idx = itmpidx;
    prt_gp_tmp[ict-1].grp = igp;
    prt_gp_tmp[ict-1].num = ic;
    all_groups = ict ;

// fill struct listGroup

    listGroup = new PTR_GROUP[all_groups+10];
    if(listGroup == NULL ) return -5;
    for(i =0; i<all_groups; i++)
    {
        listGroup[i].idx = prt_gp_tmp[i].idx;
        listGroup[i].grp = prt_gp_tmp[i].grp;
        listGroup[i].num = prt_gp_tmp[i].num;
    }
    delete prt_gp_tmp;

    ifile->seek(0);
    return 0;
}
//-----------------------------------------------------------
// read a header with out parameter
//  return : bytes read
qint32 griData::read_head()
{
    qint32 i;
    i = dfile->read((char *)head,HEAD_BYTES) ;
    if(i != HEAD_BYTES)
        return i;
    if(swap_flag !=0)
        SwapByte(head,HEAD_WORDS);
    shift_num =(qint32) head[12];
    return i;
}
//------------------------------------------------------------------
// read a header whit parmeter
//  return :bytes read
qint32 griData ::read_head(qint32 *head)
{
    qint32 i;
    i = dfile->read((char *)head,HEAD_BYTES);
    if(i != HEAD_BYTES) return i;
    if(swap_flag != 0)
        SwapByte(head,HEAD_WORDS);
    shift_num =(qint32) head[12];
    return i;
}
//----------------------------------------------------
//read a trace without parameter;
// infact wirte to internal number trace buffer;
// return :bytes read
qint32 griData ::read_trace(void)
{
    //QFile dfile;
    qint32 i;
    switch(swap_flag)
    {
    case FMT12:
        i = dfile->read((char *)itrace,bytes);
        if(i != bytes) return i;
//--------------------------------------------------
//nt4.0 qyh add
        if ( head[121] != 0)
            SwapByte1( itrace,sam);
        else
            SwapByte( itrace,sam);
//nt4.0 note
//				SwapByte(itrace,sam);
//--------------------------------------------------
        if(format == 1) spi2iee(itrace,trace,sam);
        if(format ==2 ) bgn2iee(itrace,trace,sam);
        break;
    case FMT3:
        i = dfile->read((char *)ltrace,bytes);
        if(i != bytes) return i;
        SwapByte(ltrace,sam);
        ibm2iee(ltrace,trace,sam);
        break;
    case FMT4:
        i = dfile->read((char *)trace,bytes);
        if(i != bytes) return i;
        break;
    case FMT5:
        i = dfile->read((char *)trace,bytes);
        if(i != bytes) return i;
        SwapByte((qint32 *)trace,sam);
        //ibm2iee(ltrace,trace,sam);
        break;
    }
    if(shift_num !=0)
        shift_on_off(trace, shift_num, 0, sam);
    shift_num = 0;
    return i;
}
//-------------------------------------------------------------------------
// read a trace with parameter
// return :bytes read;
qint32 griData ::read_trace(float *trace)
{
    qint32 i;
    switch(swap_flag)
    {
    case FMT12:
        i = dfile->read((char *)itrace,bytes);
        if(i != bytes) return i;
//--------------------------------------------------
//nt4.0 qyh add
        if ( head[121] != 0)
            SwapByte1( itrace,sam);
        else
            SwapByte( itrace,sam);
//nt4.0 note
//			SwapByte(itrace,sam);
//--------------------------------------------------
        if(format == 1) spi2iee(itrace,trace,sam);
        if(format ==2 ) bgn2iee(itrace,trace,sam);
        break;
    case FMT3:
        i = dfile->read((char *)ltrace,bytes);
        if(i != bytes) return i;
        SwapByte(ltrace,sam);
        ibm2iee(ltrace,trace,sam);
        break;
    case FMT4:
        i = dfile->read((char *)trace,bytes);
        if(i != bytes) return i;
        break;
    case FMT5:
        i = dfile->read((char *)trace,bytes);
        if(i != bytes) return i;
        SwapByte((qint32*)trace,sam);
        //ibm2iee(ltrace,trace,sam);
        break;
    }
    if(shift_num !=0)
        shift_on_off(trace, shift_num, 0, sam);
    shift_num = 0;
    return i;
}
//----------------------------------------------------------------
// f2----->ieee
//  len : samples
void griData::bgn2iee( qint16 *in, float *out, qint32 len)
{
    qint32 i, k, l, m;
    qint32 ff;
    for(i=0; i<len; i++)
    {
        k=(0x8000 & in[i]) >>15;  /* get signed of data */
        l=in[i] & 0x000F;  /*  get gain value    */
        in[i] = in[i]>>4;      /* right shift 4 bits */
        if( k == 1)
        {
            in[i]=0xf000 |in[i];
            in[i]=~in[i]+1;
        }

        l = 15 -l ;
        ff = 0;
        ff=ff &0x00000000;
        ff= (ff | in[i]);

        if( l>0)                       /* gain >0 left shift */
        {
            for ( m=0; m<l; m++)
                ff= ff<<1;
        }
        if(l<0)
        {
            for( m=0 ; m>l; m--)      /* gain <0 right shift */
                ff= ff>>1;
        }
        if(k==1)   ff =~ff+1 ;
        out[i] = (float)ff;
    }                                /* BGN => IEEEE */

}
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*   apply shift number or recove shift number.
	in : float input array.
	shift: shift number.
	on_off_switch: is a switch. =0 apply shift number.
				                =1 recove shift number.
	len          :  samples.  */
void griData::shift_on_off(float *in,qint32 shift, qint32 on_off_switch, qint32 len)
{
    /* on_off_switch 0: apply shift number.
        			 1: recover shift number.*/
    qint32 i,k;
    float f0, f1;

    if( on_off_switch == 0)
    {
        f0=2.0;
        f1=1.0/2.0;
    }
    else
    {
        f0=1.0/2.0;
        f1=2.0;
    }

    for( i=0; i<len; i++)
    {
        if(shift>0)
        {
            for(k=0; k<shift; k++)
                in[i] =in[i]*f0;
        }
        if(shift<0)
        {
            for(k=0; k<-shift; k++)
                in[i] =in[i]*f1;
        }
    }
}
/*------------------------------------------------------------------------*/
/* SPI TO IEEE FORMAT CONVERT APPLICATION PROGRAM.  FMT1
         void spi2iee ( qint32 *in, float *out, qint32 len)
	 f1---->ieee
	 len samples              */
void griData::spi2iee( qint16 *in, float *out, qint32 len)
{
    qint32 i;
    for (i=0; i<len; i++)
        out[i] = in[i];
}
/*-------------------------------------------------------------------------*/
/* IBM TO IEEE FORMAT CONVERT APPLICATION PROGRAM.
         void ibm2iee ( qint32 *in, float *out, qint32 len)
	 len: samples              */
void griData::ibm2iee( quint32 *in, float  *out, qint32 len)
{
    qint32 in1,in2,in3,iqr, iqr1;
    qint32 i,j;
    float  *fff;
    for (i=0; i<len; i++)
    {
        in1 = in[i] & 0x80000000;
        in2 = in[i] & 0x7f000000;
        in3 = in[i] & 0x00ffffff;
        if ( in3 == 0)
        {
            out[i]=0.0;
            continue;
        }     /* A= f*16**(jm-64)  ==>f*2**( (jm-64)*4 )    */
        in2 = ( ( (in2 >>24) -64 ) << 2 )  +126;
        iqr = in3;
        j =0;
        while ( j==0 )
        {
            iqr1 = iqr & 0x00800000;
            if( iqr1 != 0)
            {
                iqr = iqr &0x007fffff;
                j = 7;
            }
            else
            {
                iqr = iqr <<1;
                in2 = in2 -1;
            }
        }
        in1 = in2 <<23 | in1 | iqr;
        fff=(float  *)&in1;
        out[i] = fff[0];
    }
}
void griData:: set_msg_err()
{
    qint32 i;
    i = 0;
    strcpy_s(msg_err[i],"OK");
    i++;
    strcpy_s(msg_err[i],"OPEN_DATA_ERR");
    i++;
    strcpy_s(msg_err[i],"DATA_LENGTH_ERR");
    i++;
    strcpy_s(msg_err[i],"DATA_EAD_ERR");
    i++;
    strcpy_s(msg_err[i],"NOT_GRISYS_DATA");
    i++;
    strcpy_s(msg_err[i],"NO_INDEX_FILE");
    i++;
    strcpy_s(msg_err[i],"INDEX_FILE_OPEN_ERR");
    i++;
    strcpy_s(msg_err[i],"INDEX_FILE_LENGTH_ERR");
    i++;
    strcpy_s(msg_err[i],"INDEX_READ_ERR");
    i++;
    strcpy_s(msg_err[i],"INDEX_TEMP_ALLOC_ERR");
    i++;
    strcpy_s(msg_err[i],"INDEX_PTR_GROUP_ALLOC_ERR");
}
qint32  griData::fseek_trs(qint32 ptr,qint32 pos)
{
    //return  dfile->at(ptr*(HEAD_BYTES+bytes)+pos)/(HEAD_BYTES+bytes);
//	return  dfile->at(ptr*(HEAD_BYTES+bytes))/(HEAD_BYTES+bytes);

    switch(pos)
    {
    case 0:
        dfile->seek(ptr*(HEAD_BYTES+bytes));
        return  dfile->pos()/(HEAD_BYTES+bytes);
    case 1:
        dfile->seek(dfile->pos() + ptr*(HEAD_BYTES+bytes));
        return  dfile->pos()/(HEAD_BYTES+bytes);
    case 2:
        dfile->seek(dfile->size() - ptr*(HEAD_BYTES+bytes));
        return  dfile->pos()/(HEAD_BYTES+bytes);

    }
    return 0;
}
qint32 griData::fseek_bytes(qint32 nbytes,qint32 pos)
{
    switch(pos)
    {
    case 0:
        dfile->seek(nbytes);
        return dfile->pos();
    case 1:
        dfile->seek(dfile->pos() + nbytes);
        return dfile->pos();
    case 2:
        dfile->seek(dfile->size() - nbytes);
        return dfile->pos();

    }
    return 0;

//	return dfile->at(nbytes+pos);
}
/*------------------------------------
return:
	1: end position;
*/
qint32 griData::point_next_group()
{
    if((current_group_idx ++) >= all_groups-1)
    {
        current_group_idx = all_groups-1;
        current_group = listGroup[current_group_idx].grp;
        return -1;
    };

    current_group = listGroup[current_group_idx].grp;
    re_point_group();
    //fseek_trs(listGroup[current_group_idx-1].num,FROM_NOW);
    return 0;
}
/*------------------------------------
return:
	1: begin position;
*/
qint32 griData::point_pre_group()
{
    if((current_group_idx -- ) <= 0)
    {
        current_group_idx = 0;
        current_group = listGroup[current_group_idx].grp;
        return -1;
    }

    current_group = listGroup[current_group_idx].grp;
    re_point_group();

    //fseek_trs((-1)*listGroup[current_group_idx].num,FROM_NOW);
    return 0;
}
qint32 griData::indexOfGroup(qint32 n)
{
    qint32 i,iflag;
    iflag = -1;
    for(i=0; i<all_groups; i++)
    {
        if(listGroup[i].grp == n)
        {
            iflag = i;
            break;
        }
    }
    return iflag;
}
/*--------------------------------------------
return :<0 not in range;
*/
qint32 griData::point_n_group(qint32 n)
{
    qint32 iflag;
    iflag =  indexOfGroup(n);
    if(iflag == -1) return -1;
    current_group = n;
    current_group_idx = iflag;
    fseek_trs(listGroup[current_group_idx].idx,FROM_BEGIN);

    return iflag;//index

}
qint32 griData::point_idx_group(qint32 n)
{
//	qint32 iflag;

    //if(iflag == 0 ) return 1;

    current_group_idx = n;
    current_group = listGroup[n].grp;
    fseek_trs(listGroup[current_group_idx].idx,FROM_BEGIN);

    return n;//index

}
/*--------------------------------------------
return :1: not in range;
*/
qint32 griData::point_percent_group(qint32 n)
{
    qint32 ip;
    if(n <0 || n >100 ) return 1;
    ip = (qint32)all_groups * n /100;
    if(ip == all_groups) ip = all_groups-1;
    current_group_idx = (qint32)ip;
    current_group = listGroup[ip].grp;

    fseek_trs(listGroup[current_group_idx].idx,FROM_BEGIN);
    return 	current_group_idx ;
}
/*--------------------------------------------
return :1
*/
qint32 griData::re_point_group()
{
    qint32 i ;

    i = listGroup[current_group_idx].idx;
    fseek_trs(i,FROM_BEGIN);

    return current_group_idx;
}

/*---------------------------------------------------------------
*/
qint32 griData:: read_a_trace(void)
{
    qint32 i;
    i = read_head();
    if(i != HEAD_BYTES) return 1;
    i = read_trace();
    if( i != bytes ) return 2;
    return 0;
}
qint32 griData:: read_a_trace(qint32 *head,float *trace)
{
    qint32 i;
    i = read_head(head);
    if(bDEBUG)
        qDebug() << "read_a_trace  read_head ret = " << i;
    if(i != HEAD_BYTES) return 1;
    i = read_trace(trace);

    if(bDEBUG)
        qDebug() << "read_a_trace  read_trace ret = " << i;
    if( i != bytes ) return 2;
    return 0;
}
int griData::readTraces(qint32 *head, float *trace,int trs)
{
    int i,id;
    for(i = 0; i < trs; i++)
    {
        id = read_a_trace(head +i*HEAD_WORDS,trace + i*sam );
        if(id !=0) break;
    }
    if(id !=0)
        return i;
    return id;
}
qint32 griData:: get_offsets(qint32 idx,qint32 *offset)
{
    qint32 tt[IDX_ITEM_BYTES];
    qint32 i;
    ifile->seek(listGroup[idx].idx*IDX_ITEM_BYTES+FROM_BEGIN);
    for(i = 0; i<listGroup[idx].num; i++)
    {
        if(ifile->read((char *)tt,IDX_ITEM_BYTES) != IDX_ITEM_BYTES)
            return -1;
        SwapByte(tt, IDX_ITEMS);
        offset[i] = tt[5];
//		cs = tt[4];
//		if(cs >max_cs ) max_cs = cs;
//		if(cs <min_cs ) min_cs = cs;
    }
    return 0;

}
qint32 griData:: get_min_max_cs()
{
    qint32 tt[IDX_ITEM_BYTES],cs;
    qint32 i,idx;
    for(idx=0; idx<all_groups; idx++)
    {
        ifile->seek(listGroup[idx].idx*IDX_ITEM_BYTES+FROM_BEGIN);
        for(i = 0; i<listGroup[idx].num; i++)
        {
            if(ifile->read((char *)tt,IDX_ITEM_BYTES) != IDX_ITEM_BYTES) return -1;
            SwapByte(tt, IDX_ITEMS);
            cs = tt[4];
            if(cs >max_cs ) max_cs = cs;
            if(cs <min_cs ) min_cs = cs;
        }
    }
    return 0;
}
qint32 griData:: get_offsta(qint32 idx,qint32 *offset,qint32 *station)
{
    qint32 tt[IDX_ITEM_BYTES];
    qint32 i;
    ifile->seek(listGroup[idx].idx*IDX_ITEM_BYTES+FROM_BEGIN);
    for(i = 0; i<listGroup[idx].num; i++)
    {
        if(ifile->read((char *)tt,IDX_ITEM_BYTES) != IDX_ITEM_BYTES) return -1;

        SwapByte(tt, IDX_ITEMS);
        offset[i] = tt[5];
        station[i] = tt[4];
    }
    return 0;

}
qint32 griData::open_write(QString file)
{
    return open_write(file.toUtf8().data());
}
qint32 griData::open_write(const char *file)
{
    /* flag   1: open error;

    */
//    int iie;
    //int len,i;
    qint32 flag;
    QString ddfile,ifile;
    ddfile = file;
    QFileInfo fi(ddfile);

    if(fi.fileName().length()  > 8)
        ifile = ddfile.left(ddfile.length()-8) +"I"+ddfile.right(7);
    else
        ifile = ddfile +".idx";

    strcpy_s(WFileName,file);
    m_iICount = 0;
// open file
    wdfile= new QFile(WFileName);
    if (!wdfile->open(QFile::ReadWrite))
    {
        flag = OPEN_ERR;
        return flag;
    }
    wifile=new QFile(ifile);
    if (!wifile->open(QFile::ReadWrite))
    {
        flag = OPEN_ERR;
        return flag;
    }

    return 0;
}
void griData::close_write()
{
    if(wdfile->handle()!=-1)
        wdfile->close();
    if(wifile->handle()!=-1)
        wifile->close();
    wsam = 1;
    if(trace !=NULL) delete []trace;
    if(itrace !=NULL) delete []itrace;
    if(ltrace !=NULL) delete []ltrace;
    if(listGroup !=NULL) delete []listGroup;
//    if(head!=NULL) delete []head;

    return ;
}
qint32 griData ::write_head(qint32 *head)
{
    int i;
    // Write the file header
    i = wdfile->write((char *)head,HEAD_BYTES);
    if(i != HEAD_BYTES)
        return -1;
    // write Ifile trace

    m_iICount++;
    Itr[0] = m_iICount;
    Itr[1] = head[SHOT-1];
    Itr[2] = head[CMP-1];
    Itr[3] = head[TR-1];
    Itr[4] = head[STATION -1];
    Itr[5] = head[OFFSET -1];
    Itr[6] = head[LINE -1];
    Itr[7] = head[CELL - 1];
    SwapByte(Itr,8);
    //qint32 j=0;
    wifile->write((char *)Itr,8*4);

    return HEAD_BYTES;


}
qint32 griData ::write_datah(qint32 *head)
{
    if(format !=4) return -10;
    // Write the file header
    int i;
    i = dfile->write((char *)head,HEAD_BYTES);
    if(i != HEAD_BYTES)
        return -1;
    return HEAD_BYTES;
}
qint32 griData ::write_datat(float *trace)
{
    qint32 bytes;
    if(format !=4) return -10;
// write FMT4 only
    bytes = sam*4;
//	try
//	{
    dfile->write((char *)trace,bytes);
    /*	}
        catch (QXmlParseException* e)
    	(wdfile->handle())
    	{
    	  TRACE0("write trace err\n");
          return -1;
    	}*/

    return bytes;
}
qint32 griData ::write_trace(float *trace)
{
    qint32 bytes;
// write FMT4 only
    bytes = wsam*4;
//	try
//	{
    wdfile->write((char *)trace,bytes);
    /*	}
        catch (QXmlParseException* e)
    	(wdfile->handle())
    	{
    	  TRACE0("write trace err\n");
          return -1;
    	}*/

    return bytes;
}
qint32 griData:: write_a_trace(qint32 *head,float *trace)
{
    qint32 i;
//
    qint32 bytes;
    float ff;
    if(head[55-1] !=4) head[55-1] = 4; //format
    if(head[SI-1]==0 || head[TRACE_LTR-1]==0 ) return -1; //SI,LTR =0
    if(wsam == 1)  wsam = head[TRACE_LTR-1]/head[SI-1];   //first write set wsam
    if(wsam !=qint32( head[TRACE_LTR-1]/head[SI-1])) return -2;  //wsam changed
    head[13-1] = 0;// header 13==0

    ff= get_abs_max(trace,wsam); //header14=max
//	    TRACE1("ff=%g\n",ff);
    if(ff == 0.0) head[11-1] = 0;
    put_head14((float *)head +14-1,ff);   //head14
//		TRACE3("head14=%x,%ld,%ld\n",head[14-1],head[11-1],head[10-1]);
// header
    i = write_head(head);
    if(i != HEAD_BYTES) return 1;
// trace
    bytes = wsam*4;
    i = write_trace(trace);
    if( i != bytes ) return 2;
    return 0;
}
qint32 griData::write_datas(qint32 *head,float *trace,int trs)
{
    int sams,i;
    if(format !=4) return -10;
    sams = sam ;
    for(i = 0; i <trs ; i++)
    {
        write_a_data(head + i* HEAD_WORDS ,trace +i*sams);
    }
//	bytes = bytes;
    return 0;
}
qint32 griData:: write_a_data(qint32 *head,float *trace)
{
    qint32 i;
    if(format !=4) return -10;
//
    qint32 bytes;
    float ff;
    if(head[55-1] !=4) head[55-1] = 4; //format
    if(head[SI-1]==0 || head[TRACE_LTR-1]==0 ) return -1; //SI,LTR =0
    if(sam == 1)  sam = head[TRACE_LTR-1]/head[SI-1];   //first write set wsam
    if(sam !=qint32( head[TRACE_LTR-1]/head[SI-1])) return -2;  //wsam changed
    head[13-1] = 0;// header 13==0

    ff= get_abs_max(trace,sam); //header14=max
//	    TRACE1("ff=%g\n",ff);
    if(ff == 0.0) head[11-1] = 0;
    put_head14((float *)head + 14-1,ff);   //head14
//		TRACE3("head14=%x,%ld,%ld\n",head[14-1],head[11-1],head[10-1]);
// header
    i = write_datah(head);
    if(i != HEAD_BYTES) return 1;
// trace
    bytes = sam*4;
    i = write_datat(trace);
    if( i != bytes ) return 2;
    return 0;
}
float griData:: get_abs_max(float *f,qint32 len)
{
    qint32 i;
    float max,f1;
    max = 0;
    for(i = 0; i<len ; i++)
    {
        f1 = f[i];
        if (f1 <0) f1 =-1*f[i];
        if(f1 > max ) max = f1;

        if(f[i] ==0. ) f[i] =(float)1.1;
    }
    return max;
}
void  griData:: put_head14(float *f1,float f2 )
{
    *f1 = f2;
}

griData::~griData()
{
    if(dfile !=NULL)
        if(dfile->handle() !=-1)
        {
//	printf("--in xigou close_read--\n");
            close_read();
        }
    if(wdfile !=NULL)
        if(wdfile->handle()!=-1)
        {
//	printf("--in xigou close_write--");
            close_write();
        }


}
void griData::init()
{
    flag = 0;
    iie = 0;
// init numbers


    swap_flag = 0;
    shift_num = 0;

    max_cs = 0;
    min_cs = 30000;
    all_trs = 0;
    all_groups =0;
    sam =1;
    current_group_idx =0;
    trace =NULL;
    itrace = NULL;
    ltrace =NULL;
    listGroup = NULL;


    wdfile = NULL;
    wifile = NULL;
    whfile = NULL;
    dfile = NULL;
    ifile = NULL;
    tfile = NULL;


//add by Wang
//	head = new qint32(128);
//	offset = new qint32(20);
    //ifile.remove();

    strcpy_s(FileName,"");
    strcpy_s(IFileName,"");
    strcpy_s(HFileName,"");

    strcpy_s(WFileName,"");
    strcpy_s(WIFileName,"");
    strcpy_s(WHFileName,"");

}
// -1 open read error;
// -2 open write error;

qint32  griData:: createIdxFile(QString file,int hd)
{
    qint32 i,ic;
    QString ifile,ddfile;
    QFile hdf;
//data file name
    ddfile = file;
//get i file name
    QFileInfo fi(ddfile);

    ifile = get_ifile_name(file);
//open data file for read and i file for write
    open_read(ddfile ,hd,30000);
    if(!(flag !=0 || flag == NO_INDEX|| flag == INDEX_OPEN_ERR)) return -1;
    QFile hif(ifile);
    if(!hif.open(QFile::WriteOnly)) return -2;
//read and write
    ic = 0;
    for(i = 0; i<all_trs; i++)
    {
        read_a_trace();
        ic++;
        Itr[0] = ic;
        Itr[1] = head[SHOT-1];
        Itr[2] = head[CMP-1];
        Itr[3] = head[TR-1];
        Itr[4] = head[STATION -1];
        Itr[5] = head[OFFSET -1];
        Itr[6] = head[LINE -1];
        Itr[7] = head[CELL - 1];
        SwapByte(Itr,8);
        hif.write((char *)Itr,8*4);

    }
    close_read();
    hif.close();
    return 0;
}
qint32  griData:: copy_file(QString file1,QString file2,int hd)
{
    qint32 i;
    QString ifile;
    QFile inf(file1);
//open read file
    open_read(file1.toUtf8().data(),hd,30000);
    if(!(flag !=0 || flag == NO_INDEX|| flag == INDEX_OPEN_ERR)) return -1;
// write file
    open_write(file2.toUtf8().data());
    for(i = 0; i < all_trs; i++)
    {
        read_a_trace();
        write_a_trace(head,trace);
    }
    close_read();
    close_write();
    return 0;
}
int griData::testGather(QString filen)
{

    int i,num,j;
//  int SHOT,CMP,LINE,LINEC;
    int gather;
    num = 50;
    i = open_read((char *)filen.toUtf8().data(),SHOT,MAX_TRS_PER_GATHER);

    if(bDEBUG)
        qDebug() << "testGather open ret = " << i;
    if (i != 0) return -1;//read error

    gather = -1;
    //read 50 traces:

    int shot,cmp,line,linec;
    int shot1,cmp1,line1,linec1;

    for (i = 0; i < num; i++)
    {
        j = read_a_trace(head, trace);
        if(bDEBUG)
            qDebug() << "testGather read ret = " << j;
        if (j != 0) break; // read error

        shot =  head[ SHOT - 1];
        cmp =   head[CMP - 1];
        line =  head[LINE - 1];
        linec =  head[LINEC - 1];
        //qDebug() << "testdata =" << i << shot << cmp << line << linec;
        // next 2 trace header eaqual, we consider it is the gather header
        if (i >0)// not first;
        {
            if (shot == shot1 && shot >=0)
            {
                gather = SHOT;
                break;
            }
            if (cmp == cmp1 && cmp >=0)
            {
                gather = CMP;
                break;
            }
            if (line == line1 )
            {
                if (cmp == cmp1 + 1)
                {
                    gather = LINE;
                }
                break;
            }
            if (linec == linec1 && linec >=0)
            {
                gather = LINEC;
                break;
            }
        }
        shot1 = shot;
        cmp1 = cmp;
        line1 = line;
        linec1 = linec;

    }
    close_read();
    group_id = gather;
    return gather;
}
QString  griData::getKey()//gather key:cmp,shot,line
{
   QString str;
   str = "";
   if (group_id == SHOT) str = "shot";
   if (group_id == CMP) str = "cmp";
   if (group_id == LINE) str = "line";
   if (group_id == LINEC) str = "cross line";
   return str;

}
int griData::getAllTraces()
{
    return all_trs;
}
int griData::getAllGroups()
{
    return all_groups;
}
int griData::getMaxTrsPerGroup()
{
    return max_trs_per_group;
}
int griData::getLTR()
{
    return ltr;
}
int griData::getSI()
{
    return si;
}
int griData::getSamples()
{
    return sam;
}
