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
 * Custom output. Expected to behave like putchar().
 */
typedef void (*yalibnkf_putchar_t)(int c);

/**
 * Performs kanji-code conversion on string str of strlen bytes with NKF
 * and output to allocated string.
 * Specify NKF option with string opts.
 * This function dynamically allocates the string to return.
 * You must free the result with yalibnkf_free().
 * Returns { 0, 0 } on error.
 * Thread unsafe.
 */
YALIBNKF_API
yalibnkf_str
yalibnkf_convert(const char *opts, const char *str, size_t strlen);

/**
 * Performs kanji-code conversion on string str of strlen bytes with NKF
 * and output through specified custom function.
 * Specify NKF option with string opts.
 * out will be called repeatedly during conversion.
 * Returns 0 on error. Returns 1 otherwise.
 * Thread unsafe.
 */
YALIBNKF_API
int
yalibnkf_print_with(const char *opts, const char *str, size_t strlen, yalibnkf_putchar_t out);

/**
 * Performs kanji-code conversion on string str of strlen bytes with NKF.
 * without any output, just counting.
 * Used to determine the length needed for yalibnkf_write.
 * Specify NKF option with string opts.
 * Returns SIZE_MAX on error. Returns byte count of output otherwise.
 * Thread unsafe.
 */
YALIBNKF_API
size_t
yalibnkf_count(const char *opts, const char *str, size_t strlen);

/**
 * Performs kanji-code conversion on string str of strlen bytes with NKF.
 * and output through specified custom function.
 * Used to determine the length needed for yalibnkf_write.
 * Specify NKF option with string opts.
 * Returns SIZE_MAX on error. Returns byte count of output otherwise.
 * Thread unsafe.
 */
YALIBNKF_API
size_t
yalibnkf_write(const char *opts, const char *str, size_t strlen, char *dst, size_t dstlen);

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
