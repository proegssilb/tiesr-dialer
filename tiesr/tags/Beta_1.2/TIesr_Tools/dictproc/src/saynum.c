/*=======================================================================
 saynum.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
======================================================================*/

/*-------------------------------------------------------------
 This source code is derived from the public domain code ENGLISH
 TO PHONEME TRANSLATION written by John A. Wasser, 4/15/85.
 --------------------------------------------------------------*/

#include <stdio.h>
#include "dmakeup.h"

/*
**              Integer to Readable ASCII Conversion Routine.
**
** Synopsis:
**
**      dmk_say_cardinal(value)
**      	long int     value;          -- The number to output
**
**	The number is translated into a string of phonemes
**
*/

static char *Cardinals[] = 
	{
	"zIHrOW ",	"wAHn ",	"tUW ",		"THrIY ",
	"fAOr ",	"fAYv ",	"sIHks ",	"sEHvAXn ",
	"EYt ",		"nAYn ",		
	"tEHn ",	"IYlEHvAXn ",	"twEHlv ",	"THERtIYn ",
	"fAOrtIYn ",	"fIHftIYn ", 	"sIHkstIYn ",	"sEHvAXntIYn ",
	"EYtIYn ",	"nAYntIYn "
	} ;

static char *Twenties[] = 
	{
	"twEHntIY ",	"THERtIY ",	"fAOrtIY ",	"fIHftIY ",
	"sIHkstIY ",	"sEHvEHntIY ",	"EYtIY ",	"nAYntIY "
	} ;

static char *Ordinals[] = 
	{
	"zIHrOWEHTH ",	"fERst ",	"sEHkAHnd ",	"THERd ",
	"fAOrTH ",	"fIHfTH ",	"sIHksTH ",	"sEHvAXnTH ",
	"EYtTH ",	"nAYnTH ",		
	"tEHnTH ",	"IYlEHvAXnTH ",	"twEHlvTH ",	"THERtIYnTH ",
	"fAOrtIYnTH ",	"fIHftIYnTH ", 	"sIHkstIYnTH ",	"sEHvAXntIYnTH ",
	"EYtIYnTH ",	"nAYntIYnTH "
	} ;

static char *Ord_twenties[] = 
	{
	"twEHntIYEHTH ","THERtIYEHTH ",	"fOWrtIYEHTH ",	"fIHftIYEHTH ",
	"sIHkstIYEHTH ","sEHvEHntIYEHTH ","EYtIYEHTH ",	"nAYntIYEHTH "
	} ;


void
dmk_say_cardinal(long int value)
/*
   Translate a number to phonemes.  This version is for CARDINAL numbers.
   Note: this is recursive.
*/
{
  if (value < 0)
    {
      dmk_outstring("mAYnAHs ");
      value = (-value);
      if (value < 0)		/* Overflow!  -32768 */
	{
	  dmk_outstring("IHnfIHnIHtIY ");
	  return;
	}
    }

  if (value >= 1000000000L)	/* Billions */
    {
      dmk_say_cardinal(value/1000000000L);
      dmk_outstring("bIHlIYAXn ");
      value = value % 1000000000;
      if (value == 0)
	return;			/* Even billion */
      if (value < 100)		/* as in THREE BILLION AND FIVE */
	dmk_outstring("AEnd ");
    }

  if (value >= 1000000L)	/* Millions */
    {
      dmk_say_cardinal(value/1000000L);
      dmk_outstring("mIHlIYAXn ");
      value = value % 1000000L;
      if (value == 0)
	return;			/* Even million */
      if (value < 100)		/* as in THREE MILLION AND FIVE */
	dmk_outstring("AEnd ");
    }

  /* Thousands 1000..1099 2000..99999 */
  /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
  if ((value >= 1000L && value <= 1099L) || value >= 2000L)
    {
      dmk_say_cardinal(value/1000L);
      dmk_outstring("THAWzAEnd ");
      value = value % 1000L;
      if (value == 0)
	return;			/* Even thousand */
      if (value < 100)		/* as in THREE THOUSAND AND FIVE */
	dmk_outstring("AEnd ");
    }

  if (value >= 100L)
    {
      dmk_outstring(Cardinals[value/100]);
      dmk_outstring("hAHndrEHd ");
      value = value % 100;
      if (value == 0)
	return;			/* Even hundred */
    }

  if (value >= 20)
    {
      dmk_outstring(Twenties[(value-20)/ 10]);
      value = value % 10;
      if (value == 0)
	return;			/* Even ten */
    }

  dmk_outstring(Cardinals[value]);
  return;
} 


void
dmk_say_ordinal(long int value)
/*
   Translate a number to phonemes.  This version is for ORDINAL numbers.
   Note: this is recursive.
*/
{

  if (value < 0)
    {
      dmk_outstring("mAHnAXs ");
      value = (-value);
      if (value < 0)		/* Overflow!  -32768 */
	{
	  dmk_outstring("IHnfIHnIHtIY ");
	  return;
	}
    }

  if (value >= 1000000000L)	/* Billions */
    {
      dmk_say_cardinal(value/1000000000L);
      value = value % 1000000000;
      if (value == 0)
	{
	  dmk_outstring("bIHlIYAXnTH ");
	  return;		/* Even billion */
	}
      dmk_outstring("bIHlIYAXn ");
      if (value < 100)		/* as in THREE BILLION AND FIVE */
	dmk_outstring("AEnd ");
    }

  if (value >= 1000000L)	/* Millions */
    {
      dmk_say_cardinal(value/1000000L);
      value = value % 1000000L;
      if (value == 0)
	{
	  dmk_outstring("mIHlIYAXnTH ");
	  return;		/* Even million */
	}
      dmk_outstring("mIHlIYAXn ");
      if (value < 100)		/* as in THREE MILLION AND FIVE */
	dmk_outstring("AEnd ");
    }

  /* Thousands 1000..1099 2000..99999 */
  /* 1100 to 1999 is eleven-hunderd to ninteen-hunderd */
  if ((value >= 1000L && value <= 1099L) || value >= 2000L)
    {
      dmk_say_cardinal(value/1000L);
      value = value % 1000L;
      if (value == 0)
	{
	  dmk_outstring("THAWzAEndTH ");
	  return;		/* Even thousand */
	}
      dmk_outstring("THAWzAEnd ");
      if (value < 100)		/* as in THREE THOUSAND AND FIVE */
	dmk_outstring("AEnd ");
    }

  if (value >= 100L)
    {
      dmk_outstring(Cardinals[value/100]);
      value = value % 100;
      if (value == 0)
	{
	  dmk_outstring("hAHndrEHdTH ");
	  return;		/* Even hundred */
	}
      dmk_outstring("hAHndrEHd ");
    }

  if (value >= 20)
    {
      if ((value%10) == 0)
	{
	  dmk_outstring(Ord_twenties[(value-20)/ 10]);
	  return;		/* Even ten */
	}
      dmk_outstring(Twenties[(value-20)/ 10]);
      value = value % 10;
    }

  dmk_outstring(Ordinals[value]);
  return;
} 
