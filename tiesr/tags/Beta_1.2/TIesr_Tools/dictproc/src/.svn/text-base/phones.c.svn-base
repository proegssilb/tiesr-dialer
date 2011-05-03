/*=======================================================================
 phones.c

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED
======================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "dmakeup.h"

#define MAX_BUF 1024

char *
dmk_map_phones(char *buff_in)
/* Map from the original set to a local set.  Some of this amounts to
   post processing.  Could be embedded some day, but that's a lot of
   work. */
{
  static char buff_out[MAX_BUF];
  int j;
  
  for (j = 0; *buff_in; buff_in++) {
    if (j > MAX_BUF - 5) {	/* max amount put in */
      fprintf(stderr, "Error: exceeded output buffer size (%d)\n", MAX_BUF);
      break;
    }      
    if (strchr("ptkbdgnm", *buff_in) && buff_in[0] == buff_in[1]) { /* e.g. s/tt/t/g */
      buff_out[j++] = *buff_in++;
    }
    else if (buff_in[0] == 'A' && buff_in[1] == 'H') {
      buff_in++;
      strcpy(&buff_out[j], "ah");
      j += 2;
    }
    else if (buff_in[0] == 'I' && buff_in[1] == 'H') {
      buff_in++;
      strcpy(&buff_out[j], "ih");
      j += 2;
    }
    else if (*buff_in == 'h') {
      strcpy(&buff_out[j], "hh");
      j += 2;
    }
    else if (*buff_in == 'j') {
      strcpy(&buff_out[j], "jh");
      j += 2;
    }
    else if (isupper((int)(*buff_in))) { /* e.g., s/IY/IY /g */
      buff_out[j++] = tolower(*buff_in++);
      buff_out[j++] = tolower(*buff_in);
    }
    else if (*buff_in != ' ')
      buff_out[j++] = *buff_in;

    
    if (buff_in[1] != '\0')
      if (!(buff_in[1] == ' ' && buff_in[2] == '\0'))
	buff_out[j++] = ' ';
  }

  
  buff_out[j] = '\0';
  return buff_out;
}
