/*=======================================================================

 *
 * dmakeup.h
 *
 * Module header for method to get Engish word pronuncition from rules.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation version 2.1 of the License.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 * whether express or implied; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 
  The source code in this module is derived from the public domain code
  ENGLISH TO PHONEME TRANSLATION written by John A. Wasser, 4/15/85.
======================================================================*/
/* The following are all internal functions. */
extern void  dmk_outstring(char *string);
extern void  dmk_word(char *word);
extern void  dmk_say_cardinal(long int value);
extern void  dmk_say_ordinal(long int value);
extern void  dmk_say_ascii(int character);
extern void  dmk_spell_word(char *word);
extern char *dmk_map_phones(char *buff_in);

#define MAX_BUF 1024

#if defined sun && ! defined __svr4__ /* SunOS4 needs these */
extern int fprintf(FILE *stream, const char *format, ...);
extern int fputs(const char *s, FILE *stream);
extern int fflush(FILE *stream);
extern int tolower(int c);
extern int toupper(int c);
#endif
