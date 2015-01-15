/*
  yalibnkf
  Based on Python Interface to NKF
  Licensed under MIT (New BSD) License
  2014-2015, snipsnipsnip <snipsnipsnip@users.sourceforge.jp>
*/
/*
Changes.
2009.6.2    Remove WISH_TRUE, use get_guessed_code() for nkf-2.0.9
                 by SATOH Fumiyasu (fumiyas @ osstech co jp)
2008.7.17   Change the type of strlen from long to int, by SATOH Fumiyasu.
2007.2.1    Add guess() function.
2007.1.13   Remove yalibnkf_parseopts(), by SATOH Fumiyasu.
*/
/**  Python Interface to NKF
***************************************************************************
**  Copyright (c) 2005 Matsumoto, Tadashi <ma2@city.plala.jp>
**  All Rights Reserved.
**
**    Everyone is permitted to do anything on this program
**    including copying, modifying, improving,
**    as long as you don't try to pretend that you wrote it.
**    i.e., the above copyright notice has to appear in all copies.
**    Binary distribution requires original version messages.
**    You don't have to ask before copying, redistribution or publishing.
**    THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE.
***************************************************************************/

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include "yalibnkf.h"

#undef getc
#define getc(f)         yalibnkf_getc(f)

#undef putchar
#undef TRUE
#undef FALSE
#define putchar(c)      yalibnkf_putchar(c)

static size_t yalibnkf_ibufsize, yalibnkf_obufsize;
static const char *yalibnkf_inbuf;
static char *yalibnkf_outbuf;
static size_t yalibnkf_icount,yalibnkf_ocount;
static char *yalibnkf_optr;
static jmp_buf env;
static int yalibnkf_guess_flag;
static size_t yalibnkf_writecount;

static int
yalibnkf_getc(FILE *f)
{
  unsigned char c;
  if (yalibnkf_icount >= yalibnkf_ibufsize) return EOF;
  c = yalibnkf_inbuf[yalibnkf_icount];
  yalibnkf_icount++;
  return (int)c;
}

static void
yalibnkf_putchar(int c)
{
  if (yalibnkf_guess_flag) {
    return;
  }

  if (yalibnkf_ocount--){
    *yalibnkf_optr++ = (unsigned char)c;
  }else{
    size_t size = yalibnkf_obufsize + yalibnkf_obufsize;
    char *p = (char *)realloc(yalibnkf_outbuf, size + 1);
    if (p == NULL){ longjmp(env, 1); }
    yalibnkf_outbuf = p;
    yalibnkf_optr = yalibnkf_outbuf + yalibnkf_obufsize;
    yalibnkf_ocount = yalibnkf_obufsize;
    yalibnkf_obufsize = size;
    *yalibnkf_optr++ = (unsigned char)c;
    yalibnkf_ocount--;
  }
  yalibnkf_writecount++;
}

#define PERL_XS 1
#define DEFAULT_CODE_JIS
#include "nkf/utf8tbl.c"
#include "nkf/nkf.c"

/**
 * Split opts with space character and feed to nkf's option().
 * We need to handle space-delimited option one by one since nkf's handling
 * seems buggy.
 */
static int load_nkf_options(const char *opts)
{
  size_t len = strlen(opts);
  unsigned char *optchunk = (unsigned char *)malloc(len + 1);
  size_t i = 0;

  if (optchunk == NULL) {
    return -1;
  }

  while (i < len) {
    size_t skip = strcspn(&opts[i], " ");

    if (skip > 0) {
      memcpy(optchunk, &opts[i], skip);
      optchunk[skip] = '\0';

      if (options(optchunk) != 0) {
        free(optchunk);
        return -1;
      }
    }
    i += skip + strspn(&opts[i + skip], " ");
  }

  free(optchunk);

  return 0;
}

struct yalibnkf_str
yalibnkf_convert(const char *opts, const char *str, size_t strlen)
{
  struct yalibnkf_str ret;
  ret.len = 0;
  ret.str = NULL;

  if (yalibnkf_outbuf != NULL) {
    return ret;
  }

  yalibnkf_ibufsize = strlen;
  yalibnkf_obufsize = yalibnkf_ibufsize * 3 / 2 + 256;
  yalibnkf_outbuf = (char *)malloc(yalibnkf_obufsize);
  if (yalibnkf_outbuf == NULL){
    return ret;
  }
  yalibnkf_outbuf[0] = '\0';
  yalibnkf_ocount = yalibnkf_obufsize;
  yalibnkf_optr = yalibnkf_outbuf;
  yalibnkf_icount = 0;
  yalibnkf_writecount = 0;
  yalibnkf_inbuf  = str;
  yalibnkf_guess_flag = 0;

  if (setjmp(env) == 0){
    reinit();

    if (load_nkf_options(opts) != 0 || kanji_convert(NULL) != 0) {
      return ret;
    }
  }else{
    free(yalibnkf_outbuf);
    return ret;
  }

  ret.len = yalibnkf_writecount;
  ret.str = yalibnkf_outbuf;

  yalibnkf_outbuf = NULL;

  return ret;
}

const char *
yalibnkf_guess(const char *str, size_t strlen)
{
  yalibnkf_ibufsize = strlen;
  yalibnkf_icount = 0;
  yalibnkf_inbuf  = str;

  yalibnkf_guess_flag = 1;
  reinit();
  guess_f = 1;

  kanji_convert(NULL);

  return get_guessed_code();
}

void
yalibnkf_free(struct yalibnkf_str result)
{
  free((void *)result.str);
}

const char *
yalibnkf_version(void)
{
  return "yalibnkf 0.0.0 based on Network Kanji Filter Version "
    NKF_VERSION " (" NKF_RELEASE_DATE ") \n" COPY_RIGHT;
}
