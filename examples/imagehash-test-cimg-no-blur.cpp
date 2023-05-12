#include "pHash.h"
#include <iostream>
#include <chrono>

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



static double compare_dct_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static double compare_radial_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static double compare_mh_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static double compare_bmb_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2);
static bool _compare_images_ex(const CImg<uint8_t> &img1,
                               const CImg<uint8_t> &img2,
                               const IMGHash method) {
    bool ret = false;
    double d = 0;
    switch (method) {
        case DCTHASH:
            d = compare_dct_imagehash_ex(img1, img2);  // 官方小于 26 为相似
            printf("ImageMatching-test: DCTHASH ret = %f\n", d);
            if (d < 26) ret = true;
            break;
        case RADIALHASH:
            d = compare_radial_imagehash_ex(img1, img2);  // 官方大于0.85为相似
            printf("ImageMatching-test: RADIALHASH ret = %f\n", d);
            if (d > 0.85) ret = true;
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

static double compare_dct_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
{
    ulong64 hash1;
    if (_ph_dct_imagehash(img1, hash1) < 0)
        return -1.0;

    ulong64 hash2;
    if (_ph_dct_imagehash(img2, hash2) < 0)
        return -1.0;

    int d = ph_hamming_distance(hash1, hash2);
    return static_cast<double>(d);
}

int _ph_image_digest_ext(const CImg<uint8_t> &img, double sigma, double gamma,
                     Digest &digest, int N) {
    int result = -1;
    CImg<uint8_t> graysc;
    if (img.spectrum() > 3)
    {
        CImg<> rgb = img.get_shared_channels(0, 2);
        graysc = rgb.RGBtoYCbCr().channel(0);
    }
    else if (img.spectrum() == 3)
    {
        graysc = img.get_RGBtoYCbCr().channel(0);
    }
    else if (img.spectrum() == 1)
    {
        graysc = img;
    }
    else
    {
        return result;
    }

    // graysc.blur((float)sigma);

    Projections projs;
    if (ph_radon_projections(graysc, N, projs) < 0) goto cleanup;

    Features features;
    if (ph_feature_vector(projs, features) < 0) goto cleanup;

    if (ph_dct(features, digest) < 0) goto cleanup;

    result = 0;

cleanup:
    free(projs.nb_pix_perline);
    free(features.features);

    delete projs.R;
    return result;
}

static int _ph_compare_images_ext(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB,
                       double &pcc, double sigma, double gamma, int N,
                       double threshold) {
    int result = 0;
    Digest digestA;
    if (_ph_image_digest_ext(imA, sigma, gamma, digestA, N) < 0) goto cleanup;

    Digest digestB;
    if (_ph_image_digest_ext(imB, sigma, gamma, digestB, N) < 0) goto cleanup;

    if (ph_crosscorr(digestA, digestB, pcc, threshold) < 0) goto cleanup;

    if (pcc > threshold) result = 1;

cleanup:

    free(digestA.coeffs);
    free(digestB.coeffs);
    return result;
}

static double compare_radial_imagehash_ex(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
{
    double pcc = 1.0;
    double threshold = 0.85;
    int ret = _ph_compare_images_ext(img1, img2, pcc, m_sigma, m_gamma, m_angles, threshold);
    return pcc;
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
