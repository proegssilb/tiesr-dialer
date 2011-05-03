/*=======================================================================

 *
 * parse.c
 *
 * Method to parse English spelling to pronunciation.
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "dmakeup.h"

#define MAX_LENGTH 128

static FILE *In_file = NULL;
static char *buff_in;
static char buff_out[MAX_BUF];
static int bout_idx = 0;
static int bin_idx = 0;

static int Char, Char1, Char2, Char3;

static void outchar(int chr);
static int  inchar(FILE *fp);
static int  new_char(void);
static void have_dollars(void);
static void have_special(void);
static void have_number(void);
static void have_letter(void);
static void abbrev(char *buff);

void
dmk_outstring(char *string)
{
  while (*string != '\0')
    outchar(*string++);
}

static void
outchar(int chr)
{
  if (chr == '\n') {
    buff_out[bout_idx] = '\0';
    bout_idx = 0;
  }
  else if (bout_idx < MAX_BUF - 1) {
    buff_out[bout_idx++] = chr;
    buff_out[bout_idx] = '\0';
  }
  else 
    fprintf(stderr, "Warning: buffer size exceeded (%d)\n", MAX_BUF);
}


static int
inchar(FILE *fp)
{
  if (buff_in[bin_idx])
    return buff_in[bin_idx++];
  else
    return EOF;
}

static int
makeupper(int character)
{
  if (islower(character))
    return toupper(character);
  else
    return character;
}

static int
new_char(void)
{
  /*
     If the cache is full of newline, time to prime the look-ahead
     again.  If an EOF is found, fill the remainder of the queue with
     EOF's.
     */
  if (Char == '\n'  && Char1 == '\n' && Char2 == '\n' && Char3 == '\n')
    {				/* prime the pump again */
      Char = inchar(In_file);
      if (Char == EOF)
	{
	  Char1 = EOF;
	  Char2 = EOF;
	  Char3 = EOF;
	  return Char;
	}
      else if (Char == '\n')
	return Char;

      Char1 = inchar(In_file);
      if (Char1 == EOF)
	{
	  Char2 = EOF;
	  Char3 = EOF;
	  return Char;
	}
      else if (Char1 == '\n')
	return Char;

      Char2 = inchar(In_file);
      if (Char2 == EOF)
	{
	  Char3 = EOF;
	  return Char;
	}
      else if (Char2 == '\n')
	return Char;

      Char3 = inchar(In_file);
    }
  else
    {
      /*
	 Buffer not full of newline, shuffle the characters and
	 either get a new one or propagate a newline or EOF.
	 */
      Char = Char1;
      Char1 = Char2;
      Char2 = Char3;
      if (Char3 != '\n' && Char3 != EOF) {
	Char3 = inchar(In_file);
      }		
    }
  return Char;
}

int
epd_make_string(char *lexeme, char *lx_entry, size_t max_buf)
/* Make up the textual lexical entry LX_ENTRY with maximum size
   MAX_BUF for string LEXEME. */
{
  char *map_buff;

  buff_in = lexeme;
  bin_idx = 0;
  bout_idx = 0;

  /* Prime the queue */
  Char = '\n';
  Char1 = '\n';
  Char2 = '\n';
  Char3 = '\n';
  new_char();			/* Fill Char, Char1, Char2 and Char3 */

  while (Char != EOF) {		/* All of the words in the file */
    if (isdigit(Char))
      have_number();
    else if (isalpha(Char) || Char == '\'')
      have_letter();
    else if (Char == '$' && isdigit(Char1))
      have_dollars();
    else
      have_special();
  }

  
  map_buff = dmk_map_phones(buff_out);
  if (map_buff[0])
    sprintf(lx_entry, "%s", map_buff);
  else {
    fprintf(stderr, "WARNING: bogus dmakeup pronunciation (%s).\n", lexeme);
    sprintf(lx_entry, "%s", "_FAIL_");
  }

  return 1;
}

static void
have_dollars(void)
{
  long int value;

  value = 0L;
  for (new_char() ; isdigit(Char) || Char == ',' ; new_char())
    {
      if (Char != ',')
	value = 10 * value + (Char-'0');
    }

  dmk_say_cardinal(value);		/* Say number of whole dollars */

  /* Found a character that is a non-digit and non-comma */

  /* Check for no decimal or no cents digits */
  if (Char != '.' || !isdigit(Char1))
    {
      if (value == 1L)
	dmk_outstring("dAAlER ");
      else
	dmk_outstring("dAAlERz ");
      return;
    }

  /* We have '.' followed by a digit */

  new_char();			/* Skip the period */

  /* If it is ".dd " say as " DOLLARS AND n CENTS " */
  if (isdigit(Char1) && !isdigit(Char2))
    {
      if (value == 1L)
	dmk_outstring("dAAlER ");
      else
	dmk_outstring("dAAlERz ");
      if (Char == '0' && Char1 == '0')
	{
	  new_char();		/* Skip tens digit */
	  new_char();		/* Skip units digit */
	  return;
	}

      dmk_outstring("AEnd ");
      value = (Char-'0')*10 + Char1-'0';
      dmk_say_cardinal(value);

      if (value == 1L)
	dmk_outstring("sEHnt ");
      else
	dmk_outstring("sEHnts ");
      new_char();		/* Used Char (tens digit) */
      new_char();		/* Used Char1 (units digit) */
      return;
    }

  /* Otherwise say as "n POINT ddd DOLLARS " */

  dmk_outstring("pOYnt ");
  for ( ; isdigit(Char) ; new_char())
    {
      dmk_say_ascii(Char);
    }

  dmk_outstring("dAAlERz ");

  return;
}

static void
have_special(void)
	{
	if (Char == '\n')
		outchar('\n');
	else
	if (!isspace(Char))
		dmk_say_ascii(Char);

	new_char();
	return;
	}


static void
have_number(void)
{
  long int value;
  int lastdigit;

  value = Char - '0';
  lastdigit = Char;

  for (new_char() ; isdigit(Char) ; new_char())
    {
      value = 10 * value + (Char-'0');
      lastdigit = Char;
    }

  /* Recognize ordinals based on last digit of number */
  switch (lastdigit)
    {
    case '1':			/* ST */
      if (makeupper(Char) == 'S' && makeupper(Char1) == 'T' &&
	  !isalpha(Char2) && !isdigit(Char2))
	{
	  dmk_say_ordinal(value);
	  new_char();		/* Used Char */
	  new_char();		/* Used Char1 */
	  return;
	}
      break;

    case '2':			/* ND */
      if (makeupper(Char) == 'N' && makeupper(Char1) == 'D' &&
	  !isalpha(Char2) && !isdigit(Char2))
	{
	  dmk_say_ordinal(value);
	  new_char();		/* Used Char */
	  new_char();		/* Used Char1 */
	  return;
	}
      break;

    case '3':			/* RD */
      if (makeupper(Char) == 'R' && makeupper(Char1) == 'D' &&
	  !isalpha(Char2) && !isdigit(Char2))
	{
	  dmk_say_ordinal(value);
	  new_char();		/* Used Char */
	  new_char();		/* Used Char1 */
	  return;
	}
      break;

    case '0':			/* TH */
    case '4':			/* TH */
    case '5':			/* TH */
    case '6':			/* TH */
    case '7':			/* TH */
    case '8':			/* TH */
    case '9':			/* TH */
      if (makeupper(Char) == 'T' && makeupper(Char1) == 'H' &&
	  !isalpha(Char2) && !isdigit(Char2))
	{
	  dmk_say_ordinal(value);
	  new_char();		/* Used Char */
	  new_char();		/* Used Char1 */
	  return;
	}
      break;
    }

  dmk_say_cardinal(value);

  /* Recognize decimal points */
  if (Char == '.' && isdigit(Char1))
    {
      dmk_outstring("pOYnt ");
      for (new_char() ; isdigit(Char) ; new_char())
	{
	  dmk_say_ascii(Char);
	}
    }

  /* Spell out trailing abbreviations */
  if (isalpha(Char))
    {
      while (isalpha(Char))
	{
	  dmk_say_ascii(Char);
	  new_char();
	}
    }

  return;
}


static void
have_letter(void)
{
  char buff[MAX_LENGTH];
  int count;

  count = 0;
  buff[count++] = ' ';		/* Required initial blank */

  buff[count++] = makeupper(Char);

  for (new_char() ; 
       isalpha(Char) || Char == '\'' || Char == '_'; 
       new_char())
    {
      buff[count++] = makeupper(Char);
      if (count > MAX_LENGTH-2)
	{
	  buff[count++] = ' ';
	  buff[count++] = '\0';
	  dmk_word(buff);
	  count = 1;
	}
    }

  buff[count++] = ' ';		/* Required terminating blank */
  buff[count++] = '\0';

  /* Check for AAANNN type abbreviations */
  if (isdigit(Char))
    {
      dmk_spell_word(buff);
      return;
    }
  else
    if (strlen(buff) == 3)	/* one character, two spaces */
      dmk_say_ascii(buff[1]);
    else
      if (Char == '.')		/* Possible abbreviation */
	abbrev(buff);
      else
	dmk_word(buff);

  if (Char == '-' && isalpha(Char1))
    new_char();			/* Skip hyphens */

}

static void
abbrev(char *buff)
/* Handle abbreviations.  Text in buff was followed by '.' */
{
  if (strcmp(buff, " DR ") == 0)
    {
      dmk_word(" DOCTOR ");
      new_char();
    }
  else
    if (strcmp(buff, " MR ") == 0)
      {
	dmk_word(" MISTER ");
	new_char();
      }
    else
      if (strcmp(buff, " MRS ") == 0)
	{
	  dmk_word(" MISSUS ");
	  new_char();
	}
      else
	if (strcmp(buff, " PHD ") == 0)
	  {
	    dmk_spell_word(" PHD ");
	    new_char();
	  }
	else
	  dmk_word(buff);
}
