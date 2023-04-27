#if _MSC_VER && !__INTEL_COMPILER
#include "win/pHash.h"
#else
#include "pHash.h"
#endif

typedef struct bmb_hash {
    uint8_t *hash;
    uint32_t bytelength;
} BMBHash;

void ph_bmb_free(BMBHash &bh);
int _ph_bmb_imagehash(const CImg<uint8_t> &src, BMBHash &ret_hash);
int ph_bmb_imagehash(const char *file, BMBHash &ret_hash);
double ph_bmb_distance(const BMBHash &bh1, const BMBHash &bh2);
