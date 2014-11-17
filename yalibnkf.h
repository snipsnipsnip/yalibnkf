#ifndef INCLUDE_YALIBNKF_H_
#define INCLUDE_YALIBNKF_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Performs conversion on string str of strlen bytes with NKF.
 * Specify option with string opts.
 * You must free returned string with yalibnkf_free().
 * Thread unsafe.
*/
const unsigned char *
yalibnkf_convert(unsigned char* str, int strlen, unsigned char* opts);

/**
 * Guess encoding of string str of strlen bytes with NKF.
 * You must not free returned string.
 * Thread unsafe.
*/
const char *
yalibnkf_convert_guess(unsigned char* str, int strlen);

/**
 * Frees string returned from yalibnkf_convert().
 */
void
yalibnkf_free(const char *str);

/**
 * Returns version.
 */
const char *
yalibnkf_version(void);

#ifdef __cplusplus
}
#endif

#endif /* !defined(INCLUDE_YALIBNKF_H_) */
