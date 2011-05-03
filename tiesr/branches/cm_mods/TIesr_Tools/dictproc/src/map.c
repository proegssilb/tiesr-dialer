/*=======================================================================

 *
 * map.c
 *
 * Map stop consonants to closure plus stop.
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
 
======================================================================*/

#include   "dictproc.h"

#define    N_PAIR     8

struct mapping_pair {
  char  *from;
  char  *to;
} mapping_pair[ N_PAIR ] = {
  { "b", "bcl b" },
  { "d", "dcl d" },
  { "g", "gcl g" },
  { "jh", "dcl jh" },
  { "p", "pcl p" },
  { "t", "tcl t" },
  { "k", "kcl k" },
  { "ch", "tcl ch" }
};



void
map_phone(char *iphone, char *ophone)
{
  int   i;

  for ( i = 0; i < N_PAIR; i++ ) {
    if ( strcmp( iphone, mapping_pair[i].from ) == 0 ) {
      strcpy( ophone, mapping_pair[i].to );
      return;
    }
  }

  strcpy( ophone, iphone );
}

void
map_pron(char *ipron, char *opron)
{
  char  *ptr;
  char  buf[ MAX_STR ], ophone[ MAX_STR ];

  *opron = '\0';
  strcpy( buf, ipron );

  ptr = (char *) strtok( buf, DELIMIT );

  while ( ptr ) {

    map_phone( ptr, ophone );
    strcat( opron, ophone );
    strcat( opron, " " );

    ptr = (char *) strtok( NULL, DELIMIT );
  }
}
