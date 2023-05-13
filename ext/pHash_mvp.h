#ifndef _PHASH_MVP_H
#define _PHASH_MVP_H

#include "pHash.h"

typedef enum ph_mvp_retcode {
    PH_SUCCESS = 0,   /* success */
    PH_ERRFILEOPEN ,       /* file open error */
    PH_ERRFILECLOSE, 
    PH_ERRFILESETPOINTER,
    PH_ERRFILESETEOF, 
    PH_ERRMMAP,
    PH_ERRMMAPCREATE ,        /* error creating file mapping object */
    PH_ERRMMAPVIEW,            /* error creating view from file mapping object */
    PH_ERRMMAPUNVIEW, 
    PH_ERRMMAPFLUSH,
    PH_ERRARG,   /* general arg error */
    PH_ERRNULLARG,
    PH_INSUFFPOINTS, /* not enough data points in list */
    PH_ERRMEM,
    PH_ERRMEMALLOC,       /* mem alloc error - not enough available memory */
    PH_ERRNTYPE,      /* unrecognized node type */
    PH_ERRCAP,     /* more results found than can be supported in ret array */
    PH_ERRFILETYPE,  /*unrecognized file type  */
    PH_ERRPGSIZE,    /* general pg size error */
    PH_ERRPGSZINCOMP, /* incompatibility between host page size and requested page size */ 
    PH_SMPGSIZE,     /* page size is too small - over flow error */
    PH_ERRDISTFUNC,
}MVPRetCode;

typedef enum ph_hashtype {
    BYTEARRAY   = 1,          /* refers to bitwidth of the hash value */
    UINT16ARRAY = 2,
    UINT32ARRAY = 4,
    UINT64ARRAY = 8,
}HashType;

/* call back function for mvp tree functions - to performa distance calc.'s*/
typedef float (*hash_compareCB)(DP *pointA, DP *pointB);

typedef struct ph_mvp_file
{
  char *filename; /* name of db to use */
  char *buf;
  off_t file_pos;
  int fd;
  HANDLE fh;
  uint8_t filenumber;
  uint8_t nbdbfiles;
  uint8_t branchfactor; /*branch factor of tree, M(=2)*/

  /*length of path to store distances from vantage points in the struct data_point.
    used when querying or constructing the tree, P(=5) */
  uint8_t pathlength;

  uint8_t leafcapacity; /*maximum number of data points to a leaf, K(=25) */

  /*size of page size to store a leaf of the tree structure.
    Must be >= system page size.  Might need to increase above
    the system page size to fit all the data points in a leaf.
  */
  off_t pgsize;
  HashType hash_type;

  /*callback function to use to calculate the distance between 2 datapoints */
  hash_compareCB hashdist;

} MVPFile;

/* /brief alloc a single data point
 *  allocates path array, does nto set id or path
 */
DLL_EXPORT DP* ph_malloc_datapoint(HashType type);

/** /brief free a datapoint and its path
 */
DLL_EXPORT void ph_free_datapoint(DP *dp);

/** /brief read a datapoint (aux function)
 *   /param m current MVPFile struct containing state information
 *   /return DP* the read datapoint struct
 **/
DLL_EXPORT DP* ph_read_datapoint(MVPFile *m);

/** /brief get size of a datapoint in bytes (aux. function)
 *  /param m MVPFile struct 
 *  /param dp DP struct
 *  /return int number of bytes the datapoint will use in the file.
 **/
DLL_EXPORT int ph_sizeof_dp(DP *dp,MVPFile *m);

/**  /brief save datapoint to file (aux. function)
 *   /param new_dp - DP struct of dp to be saved.
 *   /param m - MVPFil
 *   /return off_t file offset of newly written dp.
 **/
DLL_EXPORT off_t ph_save_datapoint(DP *new_dp, MVPFile *m);

/** /brief mmap memory to filenumber/offset
 *  /param filenumber - uint8_t number of file to map
 *  /param offset - off_t offset into new file
 *  /param m - MVPFile
 *  /return MVPFile - ptr to new struct containing the mmap info
 **/
MVPRetCode _ph_map_mvpfile(uint8_t filenumber, off_t offset, MVPFile *m, MVPFile *m2, int use_existing=1);

/** /brief unmap/map from m2 to m
 *  /param filenumber - uint8_t filenumber of m2
 *  /param orig_pos   = off_t offset into original file in m.
 *  /return void
 **/
MVPRetCode _ph_unmap_mvpfile(uint8_t filenumber, off_t orig_pos, MVPFile *m, MVPFile *m2);

/**
 * callback function for dct image hash use in mvptree structure.
 */
DLL_EXPORT float hammingdistance(DP *pntA, DP *pntB);

/** /brief aux function to query
 *  /param m - MVPFile state information
 *  /param query - DP of datapoint to query
 *  /param knearest - int capacity of results array.
 *  /param radius - float value of radius of values to consider
 *  /param results - DP array of points of result
 *  /param count - int* number of results found (out)
 *  /param level - int value to track recursion depth.
 *  /return MVPRetCode
**/
DLL_EXPORT MVPRetCode ph_query_mvptree(MVPFile *m, DP *query, int knearest, float radius, float threshold,
			    DP **results, int *count, int level);

/**  /brief query mvptree function
 *   /param m - MVPFile file state info
 *   /param query - DP* item to query for
 *   /param knearest - int capacity of results array
 *   /param radius  - float radius to consider in query
 *   /param results - DP** list of pointers to results found
 *   /param count -  int number of results found (out)
 **/
DLL_EXPORT MVPRetCode ph_query_mvptree(MVPFile *m, DP *query, int knearest, float radius, float threshold, 
			    DP **results, int *count);

/** /brief save dp points to a file (aux func)
 *  /param m - MVPFile state information of file
 *  /param points - DP** list of points to add
 *  /param nbpoints - int length of points array
 *  /param saveall_flag - int  1 indicates used by save, 0 indicates used by add function
 *  /param level - int track recursion level
 *  /return FileIndex* - fileno and offset into file.
**/
DLL_EXPORT MVPRetCode ph_save_mvptree(MVPFile *m, DP **points, int nbpoints, int saveall_flag, int level, FileIndex *pOffset);

/** /brief save points to mvp file 
 *  /param m - MVPFile state info of file
 *  /param points - DP** list of points to add
 *  /param nbpoints - int number of points
 *  /return MVPRetCode - ret code 
**/
DLL_EXPORT MVPRetCode ph_save_mvptree(MVPFile *m, DP **points, int nbpoints);

/**  /brief add points to mvp file (aux function)
 *   /param m - MVPFile state information of file
 *   /param new_dp - datapoint to add
 *   /param level - int track recursion level
 *   /return MVPRetCode
 **/
DLL_EXPORT MVPRetCode ph_add_mvptree(MVPFile *m, DP *new_dp, int level);

/** /brief add a list of points to mvp file
    /param m - MVPFile state information of file.
    /param points - DP** list of points to add
    /param nbpoints - int number of points
    /return int - number of points added, neg for error
**/
DLL_EXPORT MVPRetCode ph_add_mvptree(MVPFile *m, DP **points, int nbpoints, int &nbsaved);

#endif // _PHASH_MVP_H
