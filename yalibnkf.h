#ifndef INCLUDE_YALIBNKF_H_
#define INCLUDE_YALIBNKF_H_

/*
  yalibnkf
  Based on Python Interface to NKF
  Licensed under MIT (New BSD) License
  2014, snipsnipsnip <snipsnipsnip@users.sourceforge.jp>
*/

/* define YALIBNKF_DLL to use DLL */
#ifdef YALIBNKF_DLL
#  ifdef YALIBNKF_BUILDING
/* We are building this library */
#    define YALIBNKF_API __declspec(dllexport)
#  else
/* We are using this library */
#    define YALIBNKF_API __declspec(dllimport)
#  endif
#else
#  define YALIBNKF_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Non '\0'-terminated string, which may include more than one '\0' bytes, with its length.
 * str may be NULL on failure.
 */
struct yalibnkf_result_t
{
  char *str;
  size_t len;
};

/**
 * Performs conversion on string str of strlen bytes with NKF.
 * Specify option with string opts.
 * You must free returned string with yalibnkf_free().
 * Returns { 0, NULL } on error.
 * Thread unsafe.
*/
YALIBNKF_API
struct yalibnkf_result_t
yalibnkf_convert(const char *str, size_t strlen, const char *opts);

/**
 * Guess encoding of string str of strlen bytes with NKF.
 * You must not free returned string.
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
yalibnkf_free(struct yalibnkf_result_t result);

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
