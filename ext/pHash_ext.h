#ifndef __PHASH_EXT_H__
#define __PHASH_EXT_H__

#include "pHash.h"

DLL_EXPORT double compare_dct_imagehash_ex_multithread(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);

DLL_EXPORT int compare_radial_imagehash_ex_multithread(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB);

DLL_EXPORT int compare_radial_imagehash_ex_samesize(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB);

#endif // __PHASH_EXT_H__
