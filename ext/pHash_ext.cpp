#include "pHash_ext.h"
#include <future>

static int m_angles = 180;
static double m_sigma = 1.0;
static double m_gamma = 1.0;
static double m_alpha = 2.0;
static double m_level = 1.0;

static int dct_imagehash_thread(const CImg<uint8_t> &img, ulong64 &hash)
{
    return _ph_dct_imagehash(img, hash);
}


double compare_dct_imagehash_ex_multithread(const CImg<uint8_t> &img1, const CImg<uint8_t> &img2)
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

int compare_radial_imagehash_ex_multithread(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB)
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

cleanup:

    free(digestA.coeffs);
    free(digestB.coeffs);
    return result;
}
