#include "pHash.h"
#include <iostream>
#include <chrono>
#include <future>

#ifndef RES_DIR_PATH
#error ResourcesDir path not define! Need Modifiy CMakeLists.txt
#endif

static int m_angles = 180;
static double m_sigma = 1.0;
static double m_gamma = 1.0;
static double m_alpha = 2.0;
static double m_level = 1.0;

enum IMGHash { DCTHASH = 1, RADIALHASH, MHASH, BMBHASH };
static bool _compare_images_ex(const CImg<uint8_t> &img1,
                               const CImg<uint8_t> &img2,
                               const IMGHash method);

int main(int argc, char *argv[])
{
    std::cout << "Hello pHash!" << std::endl;
    std::cout << "cimg_OS = " << cimg_OS << ", cimg_use_openmp = " << cimg_use_openmp << std::endl;
    char file1[500] = "";
    char file2[500] = "";
    if (argc == 3)
    {
        strcpy(file1, argv[1]);
        strcpy(file2, argv[2]);
    }
    else
    {
        const char *path = RES_DIR_PATH;
        snprintf(file1, 500, "%s/%s", path, "011.bmp");
        snprintf(file2, 500, "%s/%s", path, "012.bmp");
    }

    CImg<uint8_t>img1(file1);
    CImg<uint8_t>img2(file2);

    using p_dur = std::chrono::duration<double, std::ratio<1, 1000>>;

    auto t_st = std::chrono::high_resolution_clock::now();
    double d1 = _compare_images_ex(img1, img2, DCTHASH);
    auto t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "dct hash ret = " << d1 << ", similar res = " << (d1 < 26) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d2 = _compare_images_ex(img1, img2, RADIALHASH);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "radial hash ret = " << d2 << ", similar res = " << (d2 > 0.85) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d3 = _compare_images_ex(img1, img2, MHASH);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "mh hash ret = " << d3 << ", similar res = " << (d3 < 0.4) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    t_st = std::chrono::high_resolution_clock::now();
    double d4 = _compare_images_ex(img1, img2, BMBHASH);
    t_ed = std::chrono::high_resolution_clock::now();
    std::cout << "bmb hash ret = " << d4 << ", similar res = " << (d4 < 0.4) << ", duration = " << p_dur(t_ed - t_st).count() << "ms" << std::endl;

    return 0;
}



static double compare_dct_imagehash_ex_multithread(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static int compare_radial_imagehash_ex_multithread(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static double compare_mh_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static double compare_bmb_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static bool _compare_images_ex(const CImg<uint8_t> &img1,
                               const CImg<uint8_t> &img2,
                               const IMGHash method) {
    bool ret = false;
    double d = 0;
    switch (method) {
        case DCTHASH:
            d = compare_dct_imagehash_ex_multithread(img1, img2);  // 官方小于 26 为相似
            printf("ImageMatching-test: DCTHASH ret = %f\n", d);
            if (d < 26) ret = true;
            break;
        case RADIALHASH:
            ret = compare_radial_imagehash_ex_multithread(img1, img2);
            break;
        case MHASH:
            d = compare_mh_imagehash_ex(img1, img2);  // 官方小于0.4为相似
            printf("ImageMatching-test: MHASH ret = %f\n", d);
            if (d < 0.4) ret = true;
            break;
        case BMBHASH:
            d = compare_bmb_imagehash_ex(img1, img2);  // 官方小于0.4为相似
            printf("ImageMatching-test: BMBHASH ret = %f\n", d);
            if (d < 0.4) ret = true;
            break;
        default:
            break;
    }

    return ret;
}

static int dct_imagehash_thread(const CImg<uint8_t> &img, ulong64 &hash)
{
    return _ph_dct_imagehash(img, hash);
}

static double compare_dct_imagehash_ex_multithread(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
{
    ulong64 hash1 = 0;
    auto future = std::async(std::launch::async, dct_imagehash_thread, std::ref(img1), std::ref(hash1));

    ulong64 hash2;
    int ret1 = _ph_dct_imagehash(img2, hash2);
    int ret2 = future.get();

    int d = -1;
    if (ret1 >= 0 && ret2 >= 0)
        d = ph_hamming_distance(hash1, hash2);

    return static_cast<double>(d);
}

static int radial_imagehash_thread(const CImg<uint8_t> &img, Digest &digest)
{
    return _ph_image_digest(img, m_sigma, m_gamma, digest, m_angles);
}

static int compare_radial_imagehash_ex_multithread(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB)
{
    Digest digestA;
    auto future = std::async(std::launch::async, radial_imagehash_thread, std::ref(imA), std::ref(digestA));

    // some data
    int result = 0;
    double pcc = 1.0;
    double threshold = 0.85;

    Digest digestB;
    int ret1 = _ph_image_digest(imB, m_sigma, m_gamma, digestB, m_angles);
    int ret2 = future.get();

    if (ret1 >= 0 && ret2 >= 0)
    {
        if (ph_crosscorr(digestA, digestB, pcc, threshold) > 0)
            result = 1;
    }
    printf("ImageMatching-test: RADIALHASH multithread, get pcc = %f\n", pcc);

cleanup:

    free(digestA.coeffs);
    free(digestB.coeffs);
    return result;
}


static double compare_mh_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
{
    double d = -1;
    int n1, n2;
    uint8_t *mh1 = nullptr;
    uint8_t *mh2 = nullptr;

    mh1 = _ph_mh_imagehash(img1, n1, m_alpha, m_level);
    if (mh1 == nullptr)
        goto cleanup;

    mh2 = _ph_mh_imagehash(img2, n2, m_alpha, m_level);
    if (mh2 == nullptr)
        goto cleanup;

    d = ph_hammingdistance2(mh1, n1, mh2, n2);

cleanup:

    free(mh1);
    free(mh2);
    return d;
}


static double compare_bmb_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
{
    double d = -1;
    BMBHash bh1, bh2;

    if (_ph_bmb_imagehash(img1, bh1) < 0)
        goto cleanup;

    if (_ph_bmb_imagehash(img2, bh2) < 0)
        goto cleanup;

    d = ph_bmb_distance(bh1, bh2);

cleanup:

    ph_bmb_free(bh1);
    ph_bmb_free(bh2);
    return d;
}
