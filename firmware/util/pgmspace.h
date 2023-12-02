/* z80ctrl (https://github.com/jblang/z80ctrl)
 * Copyright 2018-2023 J.B. Langston
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file pgmspace.h Compatibility shims for AVR pgmspace.h macros and functions
 */

#ifndef UTIL_PGMSPACE_H
#define UTIL_PGMSPACE_H

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define PGM_P const char*
#define PGM_VOID_P const void*

#define PSTR(s) (s)
#define pgm_read_byte_near(p) (*(p))
#define pgm_read_word_near(p) (*(p))
#define pgm_read_dword_near(p) (*(p))
#define pgm_read_float_near(p) (*(p))
#define pgm_read_ptr_near(p) (*(p))
#define pgm_read_byte_far(p) (*(p))
#define pgm_read_word_far(p) (*(p))
#define pgm_read_dword_far(p) (*(p))
#define pgm_read_float_far(p) (*(p))
#define pgm_read_ptr_far(p) (*(p))
#define pgm_read_byte(p) (*(p))
#define pgm_read_word(p) (*(p))
#define pgm_read_dword(p) (*(p))
#define pgm_read_float(p) (*(p))
#define pgm_read_ptr(p) (*(p))

#define pgm_get_far_address(var) (var)

#define memccpy_P memccpy
#define memchr_P memchr
#define memcmp_P memcmp
#define memcmp_PF memcmp
#define memcpy_P memcpy
#define memcpy_PF memcpy
#define memmem_P memmem
#define memrchr_P memrchr
#define strcasecmp_P strcasecmp
#define strcasecmp_PF strcasecmp
#define strcasestr_P strcasestr
#define strcat_P strcat
#define strcat_PF strcat
#define strchr_P strchr
#define strchrnul_P strchrnul
#define strcmp_P strcmp
#define strcmp_PF strcmp
#define strcpy_P strcpy
#define strcpy_PF strcpy
#define strcspn_P strcspn
#define strlcat_P strlcat
#define strlcat_PF strlcat
#define strlcpy_P strlcpy
#define strlcpy_PF strlcpy
#define strlen_P strlen
#define strlen_PF strlen
#define strncasecmp_P strncasecmp
#define strncasecmp_PF strncasecmp
#define strncat_P strncat
#define strncat_PF strncat
#define strncmp_P strncmp
#define strncmp_PF strncmp
#define strncpy_P strncpy
#define strncpy_PF strncpy
#define strnlen_P strnlen
#define strnlen_PF strnlen
#define strpbrk_P strpbrk
#define strrchr_P strrchr
#define strsep_P strsep
#define strspn_P strspn
#define strstr_P strstr
#define strstr_PF strstr
#define strtok_P strtok
#define strtok_rP strtok_r
#endif

#endif
