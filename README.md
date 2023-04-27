# A Perceptual Hashing C++ Library

The project is mainly referenced from:

1. https://github.com/aetilius/pHash
2. https://www.phash.org/releases/pHash-0.9.6.tar.gz
3. https://github.com/starkdg/phash

The audiophash, cimgffmpeg, ph_fft and win folders are composed by the pHash git.
CIMG.h is from the CIMG official website (see https://cimg.eu/).
The pHash.h and pHash.cpp files are from pHash-0.9.6.tar.gz because I need pthread multi-thread support.
Bmbhash is modified from starkdg-phash.

<br/>

Features of this project:

1. Adapted mingw compilation, added the relevant mingw adaptation interface.
2. Add 4-channel color support, the original project will crash.
3. Fix some bugs in the official pHash project.
