#if _MSC_VER && !__INTEL_COMPILER
#include "win/pHash.h"
#else
#include "pHash.h"
#endif

#include <iostream>
#include <chrono>

static int m_angles = 180;
static double m_sigma = 1.0;
static double m_gamma = 1.0;
static double m_alpha = 2.0;
static double m_level = 1.0;

double compare_dct_imagehash(const char *file1, const char *file2) {
    ulong64 hash1;
    if (ph_dct_imagehash(file1, hash1) < 0) return -1.0;

    ulong64 hash2;
    if (ph_dct_imagehash(file2, hash2) < 0) return -1.0;

    int d = ph_hamming_distance(hash1, hash2);
    return static_cast<double>(d);
}

static void free_digest(Digest &digest)
{
    free(digest.coeffs);
}

double compare_radial_imagehash(const char *file1, const char *file2) {
    Digest digest1;
    if (ph_image_digest(file1, m_sigma, m_gamma, digest1, m_angles) < 0)
        return -1;

    Digest digest2;
    if (ph_image_digest(file2, m_sigma, m_gamma, digest2, m_angles) < 0)
        return -1;

    double pcc;
    if (ph_crosscorr(digest1, digest2, pcc) < 0) return -1;

    free_digest(digest1);
    free_digest(digest2);

    return pcc;
}

double compare_mh_imagehash(const char *file1, const char *file2) {
    int n1, n2;
    uint8_t *mh1 = ph_mh_imagehash(file1, n1, m_alpha, m_level);
    if (mh1 == NULL) return -1;

    uint8_t *mh2 = ph_mh_imagehash(file2, n2, m_alpha, m_level);
    if (mh2 == NULL) return -1;

    double d = ph_hammingdistance2(mh1, n1, mh2, n2);

    free(mh1);
    free(mh2);

    return d;
}

double compare_bmb_imagehash(const char *file1, const char *file2) {
    BMBHash bh1, bh2;

    if (ph_bmb_imagehash(file1, bh1) < 0) return -1;

    if (ph_bmb_imagehash(file2, bh2) < 0) return -1;

    double d = ph_bmb_distance(bh1, bh2);

    ph_bmb_free(bh1);
    ph_bmb_free(bh2);

    return d;
}

int main(int argc, char *argv[])
{
    std::cout << "Hello pHash!" << std::endl;
    char file1[500] = "";
    char file2[500] = "";
    if (argc == 3)
    {
        strcpy(file1, argv[1]);
        strcpy(file2, argv[2]);
    }
    else
    {
        const char *o_file1 = "./resources/011.bmp";
        const char *o_file2 = "./resources/012.bmp";
        strcpy(file1, o_file1);
        strcpy(file2, o_file2);
    }

    using p_dur = std::chrono::duration<double, std::ratio<1, 1000>>;

    auto t_st = std::chrono::high_resolution_clock::now();
    double d1 = compare_dct_imagehash(file1, file2);
    auto t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "dct hash ret = " << d1 << ", similar res = " << (d1 < 26) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d2 = compare_radial_imagehash(file1, file2);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "radial hash ret = " << d2 << ", similar res = " << (d2 > 0.85) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d3 = compare_mh_imagehash(file1, file2);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "mh hash ret = " << d3 << ", similar res = " << (d3 < 0.4) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d4 = compare_bmb_imagehash(file1, file2);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "bmb hash ret = " << d4 << ", similar res = " << (d4 < 0.4) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    return 0;
}
