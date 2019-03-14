/*****************************************************
**		this file is a head file of progrem 		**
**			for showing grisys data  				**
**			updated 2001.3.17 by cjh				**
**			updated VC-->QT  2001.12.16 by wyx		**
*****************************************************/
#ifndef GRIDATA_ADI_H
#define GRIDATA_ADI_H
/*
// ver1.1: if no ifile close_read error
//         1: add if before close ifile
//         2: initialize listGroup to NULL
//         3: move the all_trs from get_indx_info to get_data_info
//
//
2002.10.12:
    1:
    CreatIdxFile,copy_file,add last parameter: int hd= which gather
    if shot we give 2, if cmp gather we give 4,line gather we give 68.

    2:about index file
    for read : ifile is not exist we give the idx filename:
        get_index_file_name
        get_index_filen
    for write: if the data file name length >=8 we create ifile
               if the data file name length <8 we create idx file
        CreateIdxFile
        open_write

*/
#include <QFile>
#include <QFileInfo>
#include <QDebug>

#define FROM_NOW 1
#define FROM_BEGIN 0
#define FROM_END 2
#define strcpy_s strcpy
#define strcat_s strcat

// for error msg

#define OPEN_ERR 1
#define LENGTH_ERR 2
#define READ_ERR 3
#define NOT_GRISYS 4
#define NO_INDEX 5
#define INDEX_OPEN_ERR 6
#define INDEX_FILE_LENGTH_ERR 7
#define INDEX_READ_ERR     8
#define INDEX_TEMP_ALLOC_ERR 9
#define INDEX_PTR_GROUP_ALLOC_ERR 10
#define INDEX_GROUP_ID_NOT_IN_IFILE     11
#define MAX_MSG_ERR 15

// for grisys data file

#define HEAD_BYTES 128*4
#define HEAD_WORDS 128
#define FMT4 0
#define FMT12 1
#define FMT3 2
#define FMT5 5
#define SUN_C 100
#define PC 101
#define DEC 101

//#define WORKSTATION_C
#ifdef SUN
#define COMPUTER  SUN_C
#else
#define COMPUTER PC
#endif

// for index file
#define IDX_ITEM_BYTES 32
#define IDX_ITEMS 8
// for header
#define FORMAT 55
#define SI 9
#define TRACE_LTR 10
#define CMP 4
#define LINE 68
#define SHOT 2
#define TR 17
#define STATION 18
#define OFFSET 20
#define CELL 123
#define LINEC 123
#define FILEN_LEN 256

#define MAX_TRS_PER_GATHER 3000
#define bDEBUG 0
class griData
{
public:

    qint32 flag;

private:

    typedef struct
    {
//		quint32 idx;
        qint32 idx;
        qint32 grp,num;

    }PTR_GROUP;
    qint32 MAX_GROUPS;
    qint32 group_id;
    char FileName[FILEN_LEN],IFileName[FILEN_LEN],HFileName[FILEN_LEN];
    char WFileName[FILEN_LEN],WIFileName[FILEN_LEN],WHFileName[FILEN_LEN];
    qint32 m_iICount;


    qint32 file_len,max_cs,min_cs;
    qint32 all_groups,all_trs,max_trs_per_group;
    qint32 si,format,ltr,bytes,iie,sam;
    qint32 wsam;

// listGroup.grp[i],listGroup.idx[i]
    qint32 current_group,current_group_idx;

    char msg_err[MAX_MSG_ERR][80];

    QFile *dfile,*ifile,*tfile;
    QFile *wdfile,*wifile,*whfile;
    qint16 *itrace;
    quint32 *ltrace;
    qint32 swap_flag,shift_num;
    quint32 Itr[IDX_ITEMS];
//==================================================
public:
// not open to ther

    float *trace;
    qint32 *offset;
    qint32 head[128];
    PTR_GROUP *listGroup;

// real public	 interface:
public:

    griData();
    ~griData();

    /** @return
          0: OK
          1: open error;\n
     *    2: length = 0;\n
     *    3: read error;\n
     *    4: not grisys data;\n
     *    5: no index file;\n
     *    6:INDEX_OPEN_ERR 6\n
     *   7: INDEX_FILE_LENGTH_ERR 7\n
     *   8: INDEX_READ_ERR\n
     *   9: INDEX_TEMP_ALLOC_ERR\n
     *   10: INDEX_PTR_GROUP_ALLOC_ERR\n
     *   11: INDEX_GROUP_ID_NOT_IN_IFILE  \n
    */
    qint32 open_read(const char *file, qint32 grp_id =SHOT, qint32 max_grps = MAX_TRS_PER_GATHER);
    qint32 open_read(QString file, qint32 grp_id = SHOT, qint32 max_grps =MAX_TRS_PER_GATHER);

    qint32 open_write(const char *pzFile);
    qint32 open_write(QString file);

    void close_read();
    void close_write();

//!read head ,set shift_num ,return read bytes;
    qint32 get_offsets(qint32, qint32 *);
    qint32 get_offsta(qint32 idx, qint32 *offset, qint32 *station);
    qint32 get_min_max_cs(void);

//!seek HEAD_BYTES + bytes  return ntrs from beginning
    qint32 fseek_trs(qint32 ntrs, qint32 position);


//! locate data file acording to indx(listGroup);
    /**	return 0: ok
               -1: end position;
    */
    qint32 point_next_group();
/**	return 0: ok
               -1: begnning position;
    */

    qint32 point_pre_group();

/**	return >0: index of the group
               <0: not found that group;
    */
    qint32 point_n_group(qint32 trs);
    qint32 point_idx_group(qint32 n);

/**	return >0: indexof the group
               -1: n is not in range 0-100;
    */
    qint32 indexOfGroup(qint32 n);
    qint32 point_percent_group(qint32 n);
    qint32 re_point_group();
/*! return 0: ok
              1: read head err;
              2: read trace err;
    */
    qint32 read_a_trace(qint32 *head, float *trace); //return 0 ok;
    //! read traces;  return: 0:ok, >0 traces read;
    int readTraces(qint32 *head, float *trace,int trs);
    qint32 write_a_trace(qint32 *head, float *trace); //return 0 ok;

    qint32 createIdxFile(QString file, int hd);
//! test gather type ,return HD?,2=shot,4=cmp
    int testGather(QString filen);
    QString getKey();//gather key:cmp,shot,line
    int getAllTraces();
    int getAllGroups();
    int getMaxTrsPerGroup();
    int getLTR();
    int getSI();
    int getSamples();

private:

// not open to  outside

    qint32 read_a_trace(); //return 0 ok,>0 error
                           // seek bytes ,return bytes from beginning
    qint32 fseek_bytes(qint32 nbytes, qint32 position);
    qint32 read_head(void); //return bytes
    qint32 read_trace(void); //return bytes
    qint32 read_head(qint32 *head);   //return bytes
                                      // read trace (bytes) (use shift_num)  return read bytes
    qint32 read_trace(float *trace);  //return 0 ok,>0 error

    qint32 write_datah(qint32 *head);  //return 0 ok,>0 error
                                       //write a head to dfile
    qint32 write_datat(float *trace); //write a trace to dfile
    qint32 write_a_data(qint32 *head, float *trace); //write to dfile
    qint32 write_datas(qint32 *head, float *trace, int trs); //write to dfile


// write
    qint32 write_head(qint32 *head); //return bytes,write to wdfile
    qint32 write_trace(float *trace); //return bytes,write to wdfile

    // internal use
    void put_head14(float *f, float f1);
    void SwapByte1(qint16 *in, qint32 len);
    void SwapByte(qint16 *in, qint32 len);
    void SwapByte(qint32 *in, qint32 len);
    void SwapByte(quint32 *in, qint32 len);

    void spi2iee(qint16 *in, float *out, qint32 sam);
    void bgn2iee(qint16 *in, float *out, qint32 sam);
    void ibm2iee(quint32 *in, float *out, qint32 sam);
    void shift_on_off(float *in, qint32 shift, qint32 on_off, qint32 sam);
    void set_msg_err();

    qint32 get_data_info();
    qint32 get_index_info();
    // return 0:  ok
    /*
//       -1: idx filen open error
         -2: index file get length err;
         -3: idx file read err;
         -4  idx tmp allocation err
         -5  idx listGroup allocation err
         -6  the group id is not in the ifile header;
    */
    float get_abs_max(float *f, qint32 len);
    int copy_file(QString filein, QString fileout, int hd);
    void init();
    bool is_ifile_exists(QString file);
    bool is_ifile_exists(const char *pszPathName);
    //get the IFilename; return 0; if Ifile no exists give idx file
    QString get_ifile_name(const char *pszPathName);
    QString get_ifile_name(QString file);

    //get ifilename;if Ifile no exists return idx file
};

#endif
