/*
  yalibnkf.c
  based on Python Interface to NKF
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
#undef ungetc
#define getc(f)         yalibnkf_getc(f)         
#define ungetc(c,f)     yalibnkf_ungetc(c,f)

#undef putchar
#undef TRUE
#undef FALSE
#define putchar(c)      yalibnkf_putchar(c)

static int yalibnkf_ibufsize, yalibnkf_obufsize;
static unsigned char *yalibnkf_inbuf, *yalibnkf_outbuf;
static int yalibnkf_icount,yalibnkf_ocount;
static unsigned char *yalibnkf_iptr, *yalibnkf_optr;
static jmp_buf env;
static int yalibnkf_guess_flag;

static int 
yalibnkf_getc(FILE *f)
{
  unsigned char c;
  if (yalibnkf_icount >= yalibnkf_ibufsize) return EOF;
  c = *yalibnkf_iptr++;
  yalibnkf_icount++;
  return (int)c;
}

static int 
yalibnkf_ungetc(int c, FILE *f)
{
  if (yalibnkf_icount--){
    *(--yalibnkf_iptr) = (unsigned char)c;
    return c;
  }else{ return EOF; }
}

static void
yalibnkf_putchar(int c)
{
  size_t size;
  unsigned char *p;

  if (yalibnkf_guess_flag) {
    return;
  }

  if (yalibnkf_ocount--){
    *yalibnkf_optr++ = (unsigned char)c;
  }else{
    size = yalibnkf_obufsize + yalibnkf_obufsize;
    p = (unsigned char *)realloc(yalibnkf_outbuf, size + 1);
    if (yalibnkf_outbuf == NULL){ longjmp(env, 1); }
    yalibnkf_outbuf = p;
    yalibnkf_optr = yalibnkf_outbuf + yalibnkf_obufsize;
    yalibnkf_ocount = yalibnkf_obufsize;
    yalibnkf_obufsize = size;
    *yalibnkf_optr++ = (unsigned char)c;
    yalibnkf_ocount--;
  }
}

#define PERL_XS 1
#include "nkf/utf8tbl.c"
#include "nkf/nkf.c"

const unsigned char *
yalibnkf_convert(unsigned char* str, int strlen, unsigned char* opts)
{
  const unsigned char *ret;

  if (yalibnkf_outbuf != NULL) {
    return NULL;
  }

  yalibnkf_ibufsize = strlen + 1;
  yalibnkf_obufsize = yalibnkf_ibufsize * 3 / 2 + 256;
  yalibnkf_outbuf = (unsigned char *)malloc(yalibnkf_obufsize);
  if (yalibnkf_outbuf == NULL){
    return NULL;
  }
  yalibnkf_outbuf[0] = '\0';
  yalibnkf_ocount = yalibnkf_obufsize;
  yalibnkf_optr = yalibnkf_outbuf;
  yalibnkf_icount = 0;
  yalibnkf_inbuf  = str;
  yalibnkf_iptr = yalibnkf_inbuf;
  yalibnkf_guess_flag = 0;

  if (setjmp(env) == 0){
    reinit();
    options(opts);

    kanji_convert(NULL);

  }else{
    free(yalibnkf_outbuf);
    return NULL;
  }

  *yalibnkf_optr = 0;
  ret = yalibnkf_outbuf;
  yalibnkf_outbuf = NULL;

  return ret;
}

const char *
yalibnkf_convert_guess(unsigned char* str, int strlen)
{
  yalibnkf_ibufsize = strlen + 1;
  yalibnkf_icount = 0;
  yalibnkf_inbuf  = str;
  yalibnkf_iptr = yalibnkf_inbuf;

  yalibnkf_guess_flag = 1;
  reinit();
  guess_f = 1;

  kanji_convert(NULL);

  return get_guessed_code();
}

void
yalibnkf_free(const char *str)
{
    free((void *)str);
}

const char *
yalibnkf_version(void)
{
    return "yalibnkf 0.0.0 based on nkf " NKF_VERSION " (" NKF_RELEASE_DATE ")";
}
