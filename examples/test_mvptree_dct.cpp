#ifdef HAVE_MEMEORY_CHECK
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <cstring>
#include <iostream>
#include <vector>

#include "pHash.h"
#include "pHash_mvp.h"

#ifndef RES_DIR_PATH
#error "ResourcesDir path not define! Need Modifiy CMakeLists.txt"
#endif

float distancefunc(DP* pa, DP* pb) {
    float d = 10 * hammingdistance(pa, pb) / 64;
    float res = exp(d) - 1;
    return res;
}

int main() {
    const char* path = RES_DIR_PATH;  // 资源目录
    char* dir_name = strdup(path);
    char* filename = strdup("default_db");

    MVPFile mvpfile;
    mvpfile.filename = filename;
    mvpfile.hashdist = distancefunc;
    mvpfile.hash_type = IMAGE;
    mvpfile.hash_data_type = UINT64ARRAY;

    int nbfiles = 0;
    std::cout << "dir name: " << dir_name << std::endl;
    char** files = ph_readfilenames(dir_name, nbfiles);
    if (!files) {
        std::cout << "mem alloc error" << std::endl;
        return -2;
    }
    std::cout << "nbfiles = " << nbfiles << std::endl;
    DP** hashlist = (DP**)malloc(nbfiles * sizeof(DP*));
    if (!hashlist) {
        std::cout << "mem alloc error" << std::endl;
        return -3;
    }
    ulong64 tmphash;
    int count = 0;
    for (int i = 0; i < nbfiles; i++) {
        printf("file[%d]: %s\n", i, files[i]);
        hashlist[count] = ph_malloc_datapoint(IMAGE, mvpfile.hash_data_type);
        if (hashlist[count] == NULL) {
            std::cout << "mem alloc error" << std::endl;
            return -4;
        }
        hashlist[count]->hash = malloc(sizeof(ulong64));
        if (hashlist[count]->hash == NULL) {
            std::cout << "mem alloc error" << std::endl;
            ph_free_datapoint(hashlist[count]);
            return -5;
        }
        if (ph_dct_imagehash(files[i], tmphash) < 0) {
            std::cout << "unable to get hash" << std::endl;
            free(hashlist[count]->hash);
            ph_free_datapoint(hashlist[count]);
            continue;
        }
        hashlist[count]->id = strdup(files[i]);
        *((ulong64*)hashlist[count]->hash) = tmphash;
        hashlist[count]->hash_length = 1;
        count++;
    }

    // create
    ph_create_mvptree(&mvpfile, hashlist, count);

    // query a point
    DP retrievedData;
    if (ph_query_mvptree(&mvpfile, hashlist[0]->id, &retrievedData) == PH_SUCCESS) {
        std::cout << "Data retrieved successfully." << std::endl;
        std::cout << "ID: " << retrievedData.id << std::endl;
        std::cout << "Path: " << retrievedData.path << std::endl;
        std::cout << "Hash Length: " << retrievedData.hash_length << std::endl;
        std::cout << "Hash Data Type: " << retrievedData.hash_datatype << std::endl;
        std::cout << "Hash Values: ";
        for (uint32_t i = 0; i < retrievedData.hash_length; ++i) {
            std::cout << static_cast<uint32_t*>(retrievedData.hash)[i] << " ";
        }
        std::cout << std::endl;

        free(retrievedData.id);
        free(retrievedData.hash);
        free(retrievedData.path);
    } else {
        std::cout << "Failed to retrieve data." << std::endl;
    }

    ph_free_datapoints(hashlist, nbfiles);
    free(dir_name);
    free(filename);

    for (int i = 0; i < nbfiles; ++i) {
        free(files[i]);
    }
    free(files);

#ifdef HAVE_MEMEORY_CHECK
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
