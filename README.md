yalibnkf v0.0.0 (nkf v2.1.3)
========

Yet Another wrapper LIBrary for [NKF]( http://sourceforge.jp/projects/nkf/ ): yet another kanji code converter which ''converts input kanji code to designated kanji code such as ISO-2022-JP, Shift_JIS, EUC-JP, UTF-8, UTF-16 or UTF-32''.

C言語のプログラムに組み込むための薄い[NKF]( http://sourceforge.jp/projects/nkf/ )のラッパーです。
[clib]( https://github.com/clibs/clib/ )に対応しています。
Python2のバインディングをほぼそのまま利用しました。スレッドセーフではありません。

Installation
========

Install with [clib]( https://github.com/clibs/clib ).

```
$ clib install snipsnipsnip/yalibnkf
```

API
===

yalibnkf_convert
------

```c
/**
 * Performs conversion on string str of strlen bytes with NKF.
 * Specify option with string opts.
 * You must free returned string with yalibnkf_free().
 * Returns NULL on error.
 * Thread unsafe.
*/
const unsigned char *
yalibnkf_convert(unsigned char* str, int strlen, unsigned char* opts);
```

長さstrlenバイトの文字列strをnkfにかけます。オプションはoptsで指定します。
使用後はyalibnkf_free()で開放してください。
失敗の際はNULLを返します。

```c
const char *result = yalibnkf_convert("=?ISO-2022-JP?B?GyRCJFskMhsoQg==?=", 34, "-w");
if (result != NULL) { puts(result); }
yalibnkf_free(result);
```

yalibnkf_guess
------

```c
/**
 * Guess encoding of string str of strlen bytes with NKF.
 * You must not free returned string.
 * Thread unsafe.
*/
const char *
yalibnkf_guess(unsigned char* str, int strlen);
```

文字列のエンコーディングを推測します。推測結果は文字列で返します。

yalibnkf_free
------

```c
/**
 * Frees string returned from yalibnkf_convert().
 */
void
yalibnkf_free(const char *str);
```

`yalibnkf_convert`の返す文字列のメモリを開放します。

yalibnkf_version
------

```c
/**
 * Returns version.
 */
const char *
yalibnkf_version(void);
```

バージョン情報を返します。

Available Options
=======

nkf v2.1.3のusageを置いておきます。

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

This repository contains contents released under several different licenses.

* Original NKF: Zlib license.
 * Copyright (c) 1987, Fujitsu LTD. (Itaru ICHIKAWA).
 * Copyright (c) 1996-2013, The nkf Project.
* yalibnkf.c: BSD License.
 * Copyright (c) 2005 Matsumoto, Tadashi
 * Copyright (c) 2014, snipsnipsnip
* tools/amalgamator.py: MIT license.
 * Copyright (c) 2014, snipsnipsnip
* Other files: Zlib license.
 * Copyright (c) 2014, snipsnipsnip
