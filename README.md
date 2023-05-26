# A Perceptual Hashing C++ Library

The basic pHash principle can be seen in [this](https://www.phash.org/).
This project is mainly modified from the official project and starkdg-phash.

Features of this project:
- More simplified, Multi-platform and generic (MinGW, NDK, MSVC, GCC...).
- Add 4-channel color support, the original project will crash.
- Fix some bugs in the official pHash project.
- Use sqlite to replace mmap, easy to modify and unify.
<br/>

Mainly on image hash:
- Support c++11 multithread.
- Code optimization for faster speeds.
- Add fast Gaussian blur algorithm.(on testing)
- Algorithm acceleration for 2 images of the same size. (on testing)

<br/>

Prerequisites:
- CMake 3.12 or newer (previous versions unknown)
- A C++11 compiler

<br/>

The project is mainly referenced from:

1. [pHash](https://github.com/aetilius/pHash)
2. [releases pHash-0.9.6.tar.gz](https://www.phash.org/releases/pHash-0.9.6.tar.gz)
3. [starkdg-phash](https://github.com/starkdg/phash)
4. C++ implementation of a fast Gaussian blur algorithm by Ivan Kutskir, see [here](https://github.com/starkdg/phash).
5. [CIMG](https://cimg.eu/)
6. [dirent.h in msvc](https://github.com/tronkko/dirent)
