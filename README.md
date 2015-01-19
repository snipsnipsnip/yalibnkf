yalibnkf
========

[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/snipsnipsnip/yalibnkf?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Build Status](https://travis-ci.org/snipsnipsnip/yalibnkf.svg?branch=master)](https://travis-ci.org/snipsnipsnip/yalibnkf)

Yet Another wrapper LIBrary for [NKF]( http://sourceforge.jp/projects/nkf/ ): yet another kanji code converter which ''converts input kanji code to designated kanji code such as ISO-2022-JP, Shift_JIS, EUC-JP, UTF-8, UTF-16 or UTF-32."

C言語のプログラムに組み込むための薄い[NKF]( http://sourceforge.jp/projects/nkf/ )のラッパーです。nkfを使うのと同様のオプションを指定して使えます。
[clib]( https://github.com/clibs/clib/ )に対応しています。
Python2のバインディングをほぼそのまま利用しました。スレッドセーフではありません。

Example
=======

```c
#include <stdio.h>
#include "yalibnkf.h"

int main(int argc, char **argv) {
    char hoge[] = "=?ISO-2022-JP?B?GyRCJFskMhsoQg==?=";
    yalibnkf_str result = yalibnkf_convert("-w16", hoge, sizeof(hoge) - 1);
    
    if (result == NULL) {
        return 1;
    }
    
    printf("hoge is \"%.*s\" in UTF16\n", result.len, result.str);
    yalibnkf_free(result);
    
    return 0;
}
```

Build
=====

Build with [CMake]( http://cmake.org/ ) to get binary:

* \*nix

```console
$ cd yalibnkf/cmake
$ cmake -DCMAKE_INSTALL_PREFIX=binary -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles"
$ cmake --build . --target install
```

* windows

```console
> cd yalibnkf/cmake
> cmake -DCMAKE_INSTALL_PREFIX=binary -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles"
> cmake --build . --target install
```

or use `cmake-gui`.

Install
=======

You can just import source with [clib]( https://github.com/clibs/clib ):

```console
$ clib install snipsnipsnip/yalibnkf
```

Test
====

```console
$ cd yalibnkf/cmake
$ cmake --build .
$ ctest -V
```

Available Options
=======

Here's usage of nkf v2.1.3.

```
Usage:  nkf -[flags] [--] [in file] .. [out file for -O flag]
 j/s/e/w  Specify output encoding ISO-2022-JP, Shift_JIS, EUC-JP
          UTF options is -w[8[0],{16,32}[{B,L}[0]]]
 J/S/E/W  Specify input encoding ISO-2022-JP, Shift_JIS, EUC-JP
          UTF option is -W[8,[16,32][B,L]]
 m[BQSN0] MIME decode [B:base64,Q:quoted,S:strict,N:nonstrict,0:no decode]
 M[BQ]    MIME encode [B:base64 Q:quoted]
 f/F      Folding: -f60 or -f or -f60-10 (fold margin 10) F preserve nl
 Z[0-4]   Default/0: Convert JISX0208 Alphabet to ASCII
          1: Kankaku to one space  2: to two spaces  3: HTML Entity
          4: JISX0208 Katakana to JISX0201 Katakana
 X,x      Convert Halfwidth Katakana to Fullwidth or preserve it
 O        Output to File (DEFAULT 'nkf.out')
 L[uwm]   Line mode u:LF w:CRLF m:CR (DEFAULT noconversion)
 --ic=<encoding>        Specify the input encoding
 --oc=<encoding>        Specify the output encoding
 --hiragana --katakana  Hiragana/Katakana Conversion
 --katakana-hiragana    Converts each other
 --{cap, url}-input     Convert hex after ':' or '%'
 --numchar-input        Convert Unicode Character Reference
 --fb-{skip, html, xml, perl, java, subchar}
                        Specify unassigned character's replacement
 --in-place[=SUF]       Overwrite original files
 --overwrite[=SUF]      Preserve timestamp of original files
 -g --guess             Guess the input code
 -v --version           Print the version
 --help/-V              Print this help / configuration
Network Kanji Filter Version 2.1.3 (2013-11-22)
Copyright (C) 1987, FUJITSU LTD. (I.Ichikawa).
Copyright (C) 1996-2013, The nkf Project.
```

License
=======

MIT license.

This repository contains contents released under several different licenses.

MIT License is stronger than Zlib License, so it should be MIT License as a whole.

* Original NKF: Zlib license.
 * Copyright (c) 1987, Fujitsu LTD. (Itaru ICHIKAWA).
 * Copyright (c) 1996-2013, The nkf Project.
* yalibnkf.c: MIT License.
 * Copyright (c) 2005 Matsumoto, Tadashi
 * Copyright (c) 2014-2015, snipsnipsnip
* tools/amalgamator.py: MIT license.
 * Copyright (c) 2014, snipsnipsnip
* Other files: Zlib license.
 * Copyright (c) 2014, snipsnipsnip
