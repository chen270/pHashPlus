# A Perceptual Hashing C++ Library

The audiophash, cimgffmpeg, ph_fft and win folders are composed by the pHash git.<br>
CIMG.h is from the CIMG official website (see https://cimg.eu/).<br>
The pHash.h and pHash.cpp files are from pHash-0.9.6.tar.gz, modified and optimized according to starkdg-phash project.<br>
Bmbhash is modified from starkdg-phash.<br>

<br/>

Features of this project:

1. Add mingw compilation, added the relevant mingw adaptation interface.
2. Add 4-channel color support, the original project will crash.
3. Fix some bugs in the official pHash project.
4. support c++11 multithread.
5. add fast Gaussian blur algorithm.
5. Algorithm acceleration for 2 images of the same size. (on testing)

<br/>

The project is mainly referenced from:

1. [pHash](https://github.com/aetilius/pHash)
2. [releases pHash-0.9.6.tar.gz](https://www.phash.org/releases/pHash-0.9.6.tar.gz)
3. [starkdg-phash](https://github.com/starkdg/phash)
4. C++ implementation of a fast Gaussian blur algorithm by Ivan Kutskir, see [here](https://github.com/starkdg/phash).
