# A Perceptual Hashing C++ Library

The basic pHash principle can be seen in [this](https://www.phash.org/).
This project is mainly modified from the official project and starkdg-phash.

Features of this project:

1. More simplified, Multi-platform and generic (MinGW, NDK, MSVC, GCC...).
2. Add 4-channel color support, the original project will crash.
3. Fix some bugs in the official pHash project.
4. Support c++11 multithread.
6. Add fast Gaussian blur algorithm.
7. Code optimization for faster speeds.
8. Algorithm acceleration for 2 images of the same size. (on testing).

<br/>

The project is mainly referenced from:

1. [pHash](https://github.com/aetilius/pHash)
2. [releases pHash-0.9.6.tar.gz](https://www.phash.org/releases/pHash-0.9.6.tar.gz)
3. [starkdg-phash](https://github.com/starkdg/phash)
4. C++ implementation of a fast Gaussian blur algorithm by Ivan Kutskir, see [here](https://github.com/starkdg/phash).
5. [CIMG](https://cimg.eu/)
