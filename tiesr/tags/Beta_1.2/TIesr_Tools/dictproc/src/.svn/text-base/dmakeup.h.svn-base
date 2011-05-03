/*=======================================================================
 dmakeup.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
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
