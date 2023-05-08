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

// on testing
static int _ph_image_digest_samesize(const CImg<uint8_t> &imgA, const CImg<uint8_t> &imgB, Digest &digestA, Digest &digestB, double sigma, double gamma, int N, int need_blur);
int compare_radial_imagehash_ex_samesize(const CImg<uint8_t> &imA, const CImg<uint8_t> &imB)
{
    // if (imA.width() == imB.width() && imA.height() == imB.height())

    // int need_gauss_blur = 0;
    // if (imA.width() < 100 && imA.height() < 100)
    //     need_gauss_blur = 1;
    double pcc = 1.0;
    double threshold = 0.8;
    int result = 0;
    Digest digestA;
    Digest digestB;
    if (_ph_image_digest_samesize(imA, imB, digestA, digestB, m_sigma, m_gamma, m_angles, 1) < 0)
        goto cleanup;

    if (ph_crosscorr(digestA, digestB, pcc, threshold) < 0)
        goto cleanup;

    if (pcc > threshold)
        result = 1;

    printf("------pcc = %f\n", pcc);
cleanup:
    free(digestA.coeffs);
    free(digestB.coeffs);
    return result;
    return 0;
}

static int ph_radon_projections_samesize(const CImg<uint8_t> &imgA, Projections &projsA, const CImg<uint8_t> &imgB, Projections &projsB, const int N) {
    int width = imgA.width();
    int height = imgB.height();
    int D = (width > height) ? width : height;
    float x_center = (float)width / 2;
    float y_center = (float)height / 2;
    int x_off = std::round(x_center);
    int y_off = std::round(y_center);

    projsA.R = new CImg<uint8_t>(N, D, 1, 1, 0);
    projsB.R = new CImg<uint8_t>(N, D, 1, 1, 0);

    projsA.size = N;
    projsB.size = N;

    CImg<uint8_t> *ptr_radon_mapA = projsA.R;
    CImg<uint8_t> *ptr_radon_mapB = projsB.R;
    int *nb_per_line = projsA.nb_pix_perline;
    double factorPi = cimg::PI / 180.0;
    int N_d2 = (N >> 1);
    int N_d4 = N / 4;
    for (int k = 0; k < N_d4 + 1; ++k) {
        double theta = k * factorPi;
        double alpha = std::tan(theta);
        for (int x = 0; x < D; ++x) {
            double y = alpha * (x - x_off);
            int yd = std::round(y);
            if ((yd + y_off >= 0) && (yd + y_off < height) && (x < width)) {
                *ptr_radon_mapA->data(k, x) = imgA(x, yd + y_off);
                *ptr_radon_mapB->data(k, x) = imgB(x, yd + y_off);
                nb_per_line[k] += 1;
            }
            if ((yd + x_off >= 0) && (yd + x_off < width) && (k != N_d4) &&
                (x < height)) {
                *ptr_radon_mapA->data(N_d2 - k, x) = imgA(yd + x_off, x);
                *ptr_radon_mapB->data(N_d2 - k, x) = imgB(yd + x_off, x);
                nb_per_line[N_d2 - k] += 1;
            }
        }
    }

    int j = 0;
    int N3_d4 = 3 * N / 4;
    int y_off_m2 = 2 * y_off;
    for (int k = N3_d4; k < N; ++k) {
        double theta = k * factorPi;
        double alpha = std::tan(theta);
        for (int x = 0; x < D; ++x) {
            double y = alpha * (x - x_off);
            int yd = std::round(y);
            if ((yd + y_off >= 0) && (yd + y_off < height) && (x < width)) {
                *ptr_radon_mapA->data(k, x) = imgA(x, yd + y_off);
                nb_per_line[k] += 1;

                *ptr_radon_mapB->data(k, x) = imgB(x, yd + y_off);
            }
            if ((y_off - yd >= 0) && (y_off - yd < width) &&
                (y_off_m2 - x >= 0) && (y_off_m2 - x < height) &&
                (k != N3_d4)) {
                *ptr_radon_mapA->data(k - j, x) =
                    imgA(-yd + y_off, -(x - y_off) + y_off);
                nb_per_line[k - j] += 1;

                *ptr_radon_mapB->data(k - j, x) =
                    imgB(-yd + y_off, -(x - y_off) + y_off);
            }
        }
        j += 2;
    }

    return 0;
}

int ph_feature_vector_samesize(const Projections &projsA, Features &fvA, const Projections &projsB, Features &fvB)
{
    const CImg<uint8_t> &projection_mapA = *(projsA.R);
    const CImg<uint8_t> &projection_mapB = *(projsB.R);
    const int *nb_perline = projsA.nb_pix_perline;
    const int N = projsA.size;
    const int D = projection_mapA.height();

    fvA.features = (double *)malloc(N * sizeof(double));
    fvA.size = N;

    fvB.features = (double *)malloc(N * sizeof(double));
    fvB.size = N;
    // if (!fv.features)
    //     return -1;

    double *feat_vA = fvA.features;
    double *feat_vB = fvB.features;
    double sumA = 0.0;
    double sum_sqdA = 0.0;

    double sumB = 0.0;
    double sum_sqdB = 0.0;
    for (int k = 0; k < N; ++k)
    {
        double line_sumA = 0.0;
        double line_sum_sqdA = 0.0;

        double line_sumB = 0.0;
        double line_sum_sqdB = 0.0;
        int nb_pixels = nb_perline[k];
        if (nb_pixels == 0)
        {
            feat_vA[k] = 0.0;
            feat_vB[k] = 0.0;
            continue;
        }
        for (int i = 0; i < D; ++i)
        {
            const double pixel_valueA = projection_mapA(k, i);
            line_sumA += pixel_valueA;
            line_sum_sqdA += pixel_valueA * pixel_valueA;

            const double pixel_valueB = projection_mapB(k, i);
            line_sumB += pixel_valueB;
            line_sum_sqdB += pixel_valueB * pixel_valueB;
        }
        const double nb_pixels_inv = 1.0 / nb_pixels;
        const double meanA = line_sumA * nb_pixels_inv;
        const double meanB = line_sumB * nb_pixels_inv;
        feat_vA[k] = line_sum_sqdA * nb_pixels_inv - meanA * meanA;
        feat_vB[k] = line_sum_sqdB * nb_pixels_inv - meanB * meanB;
        sumA += feat_vA[k];
        sum_sqdA += feat_vA[k] * feat_vA[k];

        sumB += feat_vB[k];
        sum_sqdB += feat_vB[k] * feat_vB[k];
    }
    const double meanA = sumA / N;
    const double varA = 1.0 / sqrt((sum_sqdA / N) - meanA * meanA);

    const double meanB = sumB / N;
    const double varB = 1.0 / sqrt((sum_sqdB / N) - meanB * meanB);
    for (int i = 0; i < N; ++i)
    {
        feat_vA[i] = (feat_vA[i] - meanA) * varA;
        feat_vB[i] = (feat_vB[i] - meanB) * varB;
    }

    return 0;
}

int ph_dct_samsize(const Features &fvA, Digest &digestA, const Features &fvB, Digest &digestB) {
    const int N = fvA.size;
    const int nb_coeffs = 40;

    digestA.coeffs = (uint8_t *)malloc(nb_coeffs * sizeof(uint8_t));
    digestB.coeffs = (uint8_t *)malloc(nb_coeffs * sizeof(uint8_t));

    digestA.size = nb_coeffs;
    digestB.size = nb_coeffs;

    double *RA = fvA.features;
    double *RB = fvB.features;

    uint8_t *DA = digestA.coeffs;
    uint8_t *DB = digestB.coeffs;

    double D_tempA[nb_coeffs];
    double D_tempB[nb_coeffs];
    double maxA = 0.0;
    double minA = 0.0;

    double maxB = 0.0;
    double minB = 0.0;
    const double sqrt_n = 1.0 / sqrt((double)N);
    const double PI_2N = cimg::PI / (2 * N);
    const double SQRT_TWO_OVER_SQRT_N = SQRT_TWO * sqrt_n;

    for (int k = 0; k < nb_coeffs; ++k) {
        double sumA = 0.0;
        double sumB = 0.0;
        for (int n = 0; n < N; ++n) {
            sumA += RA[n] * cos(PI_2N * (2 * n + 1) * k);
            sumB += RB[n] * cos(PI_2N * (2 * n + 1) * k);
        }
        D_tempA[k] = sumA * ((k == 0) ? sqrt_n : SQRT_TWO_OVER_SQRT_N);
        D_tempB[k] = sumB * ((k == 0) ? sqrt_n : SQRT_TWO_OVER_SQRT_N);
        maxA = std::max(maxA, D_tempA[k]);
        minA = std::min(minA, D_tempA[k]);

        maxB = std::max(maxB, D_tempB[k]);
        minB = std::min(minB, D_tempB[k]);
    }

    double denoA = 1.0 / (maxA - minA);
    double denoB = 1.0 / (maxB - minB);
    for (int i = 0; i < nb_coeffs; i++) {
        DA[i] = (uint8_t)(UCHAR_MAX * (D_tempA[i] - minA) * denoA);
        DB[i] = (uint8_t)(UCHAR_MAX * (D_tempB[i] - minB) * denoB);
    }

    return 0;
}


static int _ph_image_digest_samesize(const CImg<uint8_t> &imgA, const CImg<uint8_t> &imgB, Digest &digestA, Digest &digestB, double sigma, double gamma, int N, int need_blur)
{
    int result = -1;
    CImg<uint8_t> grayscA, grayscB;
    if (imgA.spectrum() > 3)
    {
        CImg<> rgbA = imgA.get_shared_channels(0, 2);
        grayscA = rgbA.RGBtoYCbCr().channel(0);

        CImg<> rgbB = imgB.get_shared_channels(0, 2);
        grayscB = rgbB.RGBtoYCbCr().channel(0);
    }
    else if (imgA.spectrum() == 3)
    {
        grayscA = imgA.get_RGBtoYCbCr().channel(0);
        grayscB = imgB.get_RGBtoYCbCr().channel(0);
    }
    else if (imgA.spectrum() == 1)
    {
        grayscA = imgA;
        grayscB = imgB;
    }
    else
    {
        return result;
    }

    if (need_blur > 0)
    {
        grayscA.blur((float)sigma); // graysc blur, Too time-consuming
        grayscB.blur((float)sigma);
    }
    Projections projsA;
    Projections projsB;
    int* nb_pix_arr = (int *)calloc(N, sizeof(int));
    projsA.nb_pix_perline = nb_pix_arr;
    projsB.nb_pix_perline = nb_pix_arr;
    if (ph_radon_projections_samesize(grayscA, projsA, grayscB, projsB, N) < 0)
        goto cleanup;

    Features featuresA;
    Features featuresB;
    if (ph_feature_vector_samesize(projsA, featuresA, projsB, featuresB) < 0)
        goto cleanup;

    if (ph_dct_samsize(featuresA, digestA, featuresB, digestB) < 0)
        goto cleanup;

    result = 0;
cleanup:
    free(nb_pix_arr);
    free(featuresA.features);
    free(featuresB.features);

    delete projsA.R;
    delete projsB.R;
    return result;
}

