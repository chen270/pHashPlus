#ifndef _PHASH_MVP_H
#define _PHASH_MVP_H

#include "pHash.h"

typedef enum ph_mvp_retcode {
    PH_SUCCESS = 0, /* success */
    PH_ERRFILEOPEN, /* file open error */
    PH_ERRFILEEXIST,
    PH_ERRTABLECREATE,
    PH_ERRPREPARE,
    PH_ERRINSERT,
    PH_ERRFILECLOSE,
    PH_ERRSUBMIT,
    PH_ERRHASHTYPE,
    PH_ERRDPFOUND,
    PH_ERRFILESETPOINTER,
    PH_ERRFILESETEOF,
    PH_ERRARG, /* general arg error */
    PH_ERRNULLARG,
    PH_INSUFFPOINTS, /* not enough data points in list */
    PH_ERRMEM,
    PH_ERRMEMALLOC, /* mem alloc error - not enough available memory */
    PH_ERRNTYPE,    /* unrecognized node type */
    PH_ERRCAP,      /* more results found than can be supported in ret array */
    PH_ERRFILETYPE, /*unrecognized file type  */
    PH_ERRDISTFUNC,
} MVPRetCode;

/* call back function for mvp tree functions - to performa distance calc.'s*/
typedef float (*hash_compareCB)(DP *pointA, DP *pointB);

typedef struct ph_mvp_file {
    char *filename; /* name of db to use */
    HashType hash_type;
    HashDataType hash_data_type;

    /*callback function to use to calculate the distance between 2 datapoints */
    hash_compareCB hashdist;
} MVPFile;

/** /brief get size of mvp tree
 *  /param m - MVPFile struct
 *  /param result - mvp tree size of result
 *  /return int number of bytes the datapoint will use in the file.
 **/
DLL_EXPORT MVPRetCode ph_sizeof_mvptree(MVPFile *m, int64_t &result);

/**
 * callback function for dct image hash use in mvptree structure.
 */
DLL_EXPORT float hammingdistance(DP *pntA, DP *pntB);

/** /brief query a datapoint by the id of the datapoint
 *  /param m - MVPFile state information
 *  /param id - id of datapoint to query
 *  /param result - the datapoint of result
 *  /return MVPRetCode
 **/
DLL_EXPORT MVPRetCode ph_query_mvptree(MVPFile* m, const char* id, DP* result);

/** /brief query similar datapoints in mvp file
 *  /param m - MVPFile state information
 *  /param query - DP of datapoint to query
 *  /param results - DP array of points of result
 *  /param count - int* number of results found (out)
 *  /return MVPRetCode
 **/
DLL_EXPORT MVPRetCode ph_query_mvptree(MVPFile *m, DP *query, float threshold, DP **results, int *count);

/**
 * /brief create a database file
 */
DLL_EXPORT MVPRetCode ph_create_mvptree(MVPFile *m);

/** /brief creat a database and save points to mvp file
 *  /param m - MVPFile state info of file
 *  /param points - DP** list of points to add
 *  /param nbpoints - int number of points
 *  /return MVPRetCode - ret code
 **/
DLL_EXPORT MVPRetCode ph_create_mvptree(MVPFile *m, DP **points, int nbpoints);

/**  /brief add a point to mvp file
 *   /param m - MVPFile state information of file
 *   /param new_dp - datapoint to add
 *   /return MVPRetCode
 **/
DLL_EXPORT MVPRetCode ph_add_mvptree(MVPFile *m, DP *new_dp);

/** /brief add a list of points to mvp file
 *  /param m - MVPFile state information of file.
 *  /param points - DP** list of points to add
 *  /param nbpoints - int number of points
 *  /param nbsaved - number of points added
 *  /return MVPRetCode
**/
DLL_EXPORT MVPRetCode ph_add_mvptree(MVPFile *m, DP **points, int nbpoints,
                                     int &nbsaved);

#endif  // _PHASH_MVP_H
