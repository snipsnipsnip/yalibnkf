#ifndef INCLUDE_YALIBNKF_H_
#define INCLUDE_YALIBNKF_H_

/*
  yalibnkf
  Based on Python Interface to NKF
  Licensed under MIT (New BSD) License
  2014-2015, snipsnipsnip <snipsnipsnip@users.sourceforge.jp>
*/

/* define YALIBNKF_DLL to use DLL */
#ifndef YALIBNKF_API
#  ifdef YALIBNKF_DLL
#    ifdef YALIBNKF_BUILDING
/* We are building this library */
#      define YALIBNKF_API __declspec(dllexport)
#    else
/* We are using this library */
#      define YALIBNKF_API __declspec(dllimport)
#    endif
#  else
#    define YALIBNKF_API
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NUL-terminated string with length.
 * str may include more than one '\0' bytes before the end.
 * str may be NULL on failure.
 */
typedef struct yalibnkf_str
{
  char *str;
  size_t len;
}
yalibnkf_str;

/**
 * Performs kanji-code conversion on string str of strlen bytes with NKF.
 * Specify NKF option with string opts.
 * This function allocates the string dynamically.
 * You must free returned string with yalibnkf_free().
 * Returns { 0, 0 } on error.
 * Thread unsafe.
*/
YALIBNKF_API
yalibnkf_str
yalibnkf_convert(const char *opts, const char *str, size_t strlen);

/**
 * Guess encoding of string str of strlen bytes with NKF.
 * Returns a static constant string.
 * Thread unsafe.
*/
YALIBNKF_API
const char *
yalibnkf_guess(const char *str, size_t strlen);

/**
 * Frees string returned from yalibnkf_convert().
 */
YALIBNKF_API
void
yalibnkf_free(struct yalibnkf_str result);

/**
 * Frees internal working memory.
 */
YALIBNKF_API
void
yalibnkf_quit(void);

/**
 * Returns version.
 */
YALIBNKF_API
const char *
yalibnkf_version(void);

#ifdef __cplusplus
}
#endif

#endif /* !defined(INCLUDE_YALIBNKF_H_) */
