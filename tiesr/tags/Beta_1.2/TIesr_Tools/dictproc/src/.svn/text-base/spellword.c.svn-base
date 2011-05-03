/*=======================================================================
 spellword.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
======================================================================*/

/*-------------------------------------------------------------
 This source code is derived from the public domain code ENGLISH
 TO PHONEME TRANSLATION written by John A. Wasser, 4/15/85.
 --------------------------------------------------------------*/

#include <stdio.h>
#include "dmakeup.h"

static char *Ascii[] =
	{
"nUWl ","stAArt AXv hEHdER ","stAArt AXv tEHkst ","EHnd AXv tEHkst ",
"EHnd AXv trAEnsmIHSHAXn",
"EHnkwAYr ","AEk ","bEHl ","bAEkspEYs ","tAEb ","lIHnIYfIYd ",
"vERtIHkAXl tAEb ","fAOrmfIYd ","kAErAYj rIYtERn ","SHIHft AWt ",
"SHIHft IHn ","dIHlIYt ","dIHvIHs kAAntrAAl wAHn ","dIHvIHs kAAntrAAl tUW ",
"dIHvIHs kAAntrAAl THrIY ","dIHvIHs kAAntrAAl fOWr ","nAEk ","sIHnk ",
"EHnd tEHkst blAAk ","kAEnsEHl ","EHnd AXv mEHsIHj ","sUWbstIHtUWt ",
"EHskEYp ","fAYEHld sIYpERAEtER ","grUWp sIYpERAEtER ","rIYkAOrd sIYpERAEtER ",
"yUWnIHt sIYpERAEtER ","spEYs ","EHksklAEmEYSHAXn mAArk ","dAHbl kwOWt ",
"nUWmbER sAYn ","dAAlER sAYn ","pERsEHnt ","AEmpERsAEnd ","kwOWt ",
"OWpEHn pEHrEHn ","klOWz pEHrEHn ","AEstEHrIHsk ","plAHs ","kAAmmAX ",
"mIHnAHs ","pIYrIYAAd ","slAESH ",

"zIHrOW ","wAHn ","tUW ","THrIY ","fAOr ",
"fAYv ","sIHks ","sEHvAXn ","EYt ","nAYn ",

"kOWlAXn ","sEHmIHkOWlAXn ","lEHs DHAEn ","EHkwAXl sAYn ","grEYtER DHAEn ",
"kwEHsCHAXn mAArk ","AEt sAYn ",

"EY ","bIY ","sIY ","dIY ","IY ","EHf ","jIY  ",
"EYCH ","AY ","jEY ","kEY ","EHl ","EHm ","EHn ","OW ","pIY ",
"kyUW ","AAr ","EHs ","tIY ","yUW ","vIY ",
"dAHblyUW ","EHks ","wAY ","zIY ",

"lEHft brAEkEHt ","bAEkslAESH ","rAYt brAEkEHt ","kAErEHt ",
"AHndERskAOr ","AEpAAstrAAfIH ",

"EY ","bIY ","sIY ","dIY ","IY ","EHf ","jIY  ",
"EYtCH ","AY ","jEY ","kEY ","EHl ","EHm ","EHn ","AA ","pIY ",
"kw ","AAr ","EHz ","tIY ","AHw ","vIY ",
"dAHblyUWw ","EHks ","wAYIY ","zIY ",

"lEHft brEYs ","vERtIHkAXl bAAr ","rAYt brEYs ","tAYld ","dEHl ",
	};

void
dmk_say_ascii(int character)
{
  dmk_outstring(Ascii[character&0x7F]);
}

void
dmk_spell_word(char *word)
{
  for (word++ ; word[1] != '\0' ; word++)
    dmk_outstring(Ascii[(*word)&0x7F]);
}
