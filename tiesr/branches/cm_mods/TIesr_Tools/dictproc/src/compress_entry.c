/*=======================================================================
 
 *
 * compress_entry.c
 *
 * Compress a dictionary entry.
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

/* dictionary entry */
int  cur_entry;
char *entry[ MAX_ENTRY ];
char *entry_pron[ MAX_ENTRY ];

/* monophone */
char *phone[ MAX_PHONE ];
int n_phone;                /* 46 + "bbb" = 47 */

/* pronunciation match */
char *pron_dict;
char *pron_def;
int  len_dict;
int  len_def;
int  gerrc, gsubc, gdelc, ginsc;
char g_match_pattern[ MAX_PRON ];
int  g_match_pattern_cnt;

/* ---------------------------------------------------------------------------
   some string functions ...
   ---------------------------------------------------------------------- */

int get_strlen( char *str )
{
  int   len = 0;

  while ( *str++ ) len++;
  
  return len;
}

void copy_str(char *str1, char *str2)
{
  while ( *str2 )
    *str1++ = *str2++;
    
  *str1 = '\0';
}

int equal_str(char *str1, char *str2)
{
  while ( *str1 && *str2  && ( *str1 == *str2 ) ) {
    str1++;
    str2++;
  }

  if ( *str1 == '\0' && *str2 == '\0' ) return 1;
  else return 0;
}

/* ---------------------------------------------------------------------------
   return -1 0 1 if str1 < = > str2
   ---------------------------------------------------------------------- */

int compare_str(char *str1, char *str2)
{
  while ( *str1 && *str2 ) {

    if ( *str1 > *str2 ) 
      return 1;
    else if ( *str1 < *str2 ) 
      return -1;

    str1++;
    str2++;
  }

  if ( ( *str1 == 0 ) && ( *str2 != 0 ) )
    return -1;
  else if ( ( *str1 != 0 ) && ( *str2 == 0 ) )
    return 1;
  else
    return 0;
}

/* ---------------------------------------------------------------------------
   read monophone names
   ---------------------------------------------------------------------- */

void read_phone( char *fname )
{
  FILE  *fp;
  char  buf[ MAX_STR ];
  char  *ptr;

  n_phone = 0;

  fp = fopen(fname, "r");
  if( fp == NULL )
  {
      fprintf( stderr, "Failed to open phone file %s\n", fname );
      exit(1);
  }
  while ( fgets(buf, MAX_STR, fp) ) {
    ptr = (char *) strtok(buf, DELIMIT);
    phone[ n_phone ] = (char *) strdup( ptr );
    n_phone++;
  }

  fclose( fp );
}

int get_phone_idx(char *name)
{
  int i;

  for(i = 0; i < n_phone; i++) {
    if ( strcmp(name, phone[i]) == 0 )
      return i;
  }

  fprintf(stderr, "Error: %s not a phone\n", name );
  return( -1 );
}

/* ---------------------------------------------------------------------------
   pron2bin

   Convert ascii phone sequence to binary, where each phone is represented
   by its index in the input phone list as a byte.

   pron[0] = length in bytes, total length of data in pron is pron[0] + 1

   pron[0] can equal zero if str contains no phones.
   ---------------------------------------------------------------------- */

void pron2bin(char *str, char *pron)
{
  int   i = 1;
  char  *ptr;

  ptr = (char *) strtok(str, DELIMIT);
  while( ptr )
  {
     pron[ i++ ] = get_phone_idx( ptr );
     ptr = (char *) strtok(NULL, DELIMIT) ;
  }
  pron[ 0 ] = i - 1;
}

/* ---------------------------------------------------------------------------
   convert g_match_pattern sequence to pron_err
   pron_err[0] = length in bytes, total length is pron_err[0] + 1

   pron_dict set by calling function
   ---------------------------------------------------------------------- */

void pron2err( char *pron_err )
{
  int   i;
  int   len, idx1, idx2;
  char  pron[ MAX_PRON ];

  if ( g_match_pattern_cnt > MAX_POS ) {
    fprintf(stderr, "Error: phone alignment length exceeds %d\n", MAX_POS);
    exit(1);
  }

  /* align, make space for insertion */

  idx1 = idx2 = 0;

  for (i = 0; i < g_match_pattern_cnt; i++) {

    if ( g_match_pattern[ i ] == MATCH ) {

      pron[ idx1++ ] = pron_dict[ idx2++ ];
      
    } else if ( g_match_pattern[ i ] == SUB ) {

      pron[ idx1++ ] = pron_dict[ idx2++ ];
      
    } else if ( g_match_pattern[ i ] == DEL ) {

      pron[ idx1++ ] = pron_dict[ idx2++ ];
      
    } else if ( g_match_pattern[ i ] == INS ) {

      idx1++;

    }
  }

  /* encode the error */

  len = 0;
  
  for (i = 0; i < g_match_pattern_cnt; i++) {
    
    if ( g_match_pattern[ i ] == SUB ) {
      
      pron_err[ 1 + len ] = SUB | i;         /* position */
      pron_err[ 1 + len + 1 ] = pron[ i ];   /* phone */
      len += 2;

    } else if ( g_match_pattern[ i ] == DEL ) {

      pron_err[ 1 + len ] = DEL | i;         /* position */
      pron_err[ 1 + len + 1 ] = pron[ i ];   /* phone */
      len += 2;

    } else if ( g_match_pattern[ i ] == INS ) {

      pron_err[ 1 + len ] = INS | i;         /* position */
      len++;

    }
  }

  pron_err[0] = len;
}

/* ---------------------------------------------------------------------------
   convert pron_err sequence to pron,
   pron[0] = length in bytes, total length is pron[0] + 1

   pron_def set by the calling function
   ---------------------------------------------------------------------- */

void err2pron( char *pron_err, char *pron )
{
  int   i;
  int   len, idx1, idx2, pos, insc;

  /* go through pron_err and reconstruct pron */

  insc = 0;
  idx1 = idx2 = 0;
  len = pron_err[0];

  i = 1;

  while ( i <= len ) {

    if ( ( pron_err[i] & MASK ) == SUB ) {            /* SUB */

      pos = pron_err[i] & 0x3f;   /* position */
      while ( idx1 + insc < pos )
	pron[ 1 + idx1++ ] = pron_def[ idx2++ ];

      pron[ 1 + idx1++ ] = pron_err[ i + 1 ];
      idx2++;

      i += 2;

    } else if ( ( pron_err[i] & MASK ) == DEL ) {     /* DEL */
      
      pos = pron_err[i] & 0x3f;   /* position */
      while ( idx1 + insc < pos )
	pron[ 1 + idx1++ ] = pron_def[ idx2++ ];

      pron[ 1 + idx1++ ] = pron_err[ i + 1 ];

      i += 2;

    } else {                                          /* INS */

      pos = pron_err[i] & 0x3f;   /* position */
      while ( idx1 + insc < pos )
	pron[ 1 + idx1++ ] = pron_def[ idx2++ ];

      idx2++;

      insc++;

      i++;
    }
  }

  /* the rest, including no err */

  while ( idx2 < len_def )
    pron[ 1 + idx1++ ] = pron_def[ idx2++ ];

  pron[0] = idx1;
}

 /* ---------------------------------------------------------------------------
    check if two binary pron's are equal
   ---------------------------------------------------------------------- */
 
int equal_pron(char *pron1, char *pron2)
{
  int   i, len;

  if ( pron1[0] != pron2[0] ) return 0;

  len = pron1[0];
  for (i = 1; i <= len; i++)
    if ( pron1[i] != pron2[i] ) return 0;

  return 1;
}

/* ---------------------------------------------------------------------------
   must do buf[0] = '\0' before calling expand_str()
   ---------------------------------------------------------------------- */

void expand_str(char *buf, int idx)
{
  int   len;

  if ( (signed char) entry[ idx ][ 0 ] >= 0 ) {

    copy_str(buf, entry[ idx ]);
    
  } else {
    
    expand_str(buf, idx - 1);
    len = - (signed char) entry[ idx ][ 0 ];
    buf += len;
    copy_str(buf, &entry[ idx ][ 1 ]);
  }
}

/* ---------------------------------------------------------------------------
   must do buf[0] = '\0' before calling expand_pron()
   very similar to expand_str(), but the format is different.

   "String" is terminated by '\0'.

   "Pronunciation" is prefixed by length, followed by length bytes,
   there is no termination char.
   ---------------------------------------------------------------------- */

void expand_pron(char *buf, int idx)
{
  int   len, base;
  int   i;

  if ( entry_pron[ idx ][ 0 ] == 0 ) {

    buf[0] = '\0';

  } else if ( ( entry_pron[ idx ][ 1 ] & MASK ) != MATCH ) {

    len = entry_pron[ idx ][ 0 ];
    for (i = 1; i <= len; i++) buf[ i ] = entry_pron[ idx ][ i ];
    buf[0] = len;
    
  } else {
    
    expand_pron(buf, idx - 1);
    len = entry_pron[ idx ][ 1 ];
    base = len + 1;
    len = entry_pron[ idx ][ 0 ] - 1;
    for (i = 0; i < len; i++) buf[ base + i ] = entry_pron[ idx ][ 2 + i ];
    buf[0] = base + len - 1;
  }
}

/* ---------------------------------------------------------------------------
   compare str with entry[ cur_entry - 1 ]
   ---------------------------------------------------------------------- */

void compare_entry(char *str, int *cnt)
{
  char  buf[ MAX_STR ], *ptr;
  
  buf[0] = '\0';
  expand_str(buf, cur_entry - 1);
  ptr = buf;

  *cnt = 0;
  while ( *str && *ptr && ( *str == *ptr ) ) {
    *cnt += 1;
    str++;
    ptr++;
  }
}

/* ---------------------------------------------------------------------------
   compare pron with entry_pron[ cur_entry - 1 ]
   ---------------------------------------------------------------------- */

void compare_entry_pron(char *pron, int *cnt)
{
  char  buf[ MAX_PRON ], *ptr;
  int   len;
  int   i;

  buf[0] = '\0';
  expand_pron(buf, cur_entry - 1);
  len = pron[0] < buf[0] ? pron[0] : buf[0];
  ptr = buf;

  pron++;
  ptr++;
  for (i = 0; i < len; i++) {
    if ( pron[i] != ptr[i] ) break;
  }

  *cnt = i;
}

/* ---------------------------------------------------------------------------
   encode str and put it into entry[ cur_entry ]
   cur_entry needs to be incremented by the calling function
   ---------------------------------------------------------------------- */

void encode_entry( char *str )
{
  int   match_cnt;
  int   len;
  char  buf[ MAX_STR ];

  if ( cur_entry - 1 >= 0 ) {

    compare_entry(str, &match_cnt);

    /* need to match more than one char to be useful */
    
    if ( match_cnt > 1 ) {

      buf[0] = (signed char) - match_cnt;
      copy_str(&buf[1], str + match_cnt);
      len = get_strlen( buf );
      entry[ cur_entry ] = (char *) malloc( len + 1 );
      copy_str( entry[ cur_entry ], buf );

    } else {

      entry[ cur_entry ] = (char *) strdup( str );
    }

  } else {
    
    entry[ cur_entry ] = (char *) strdup( str );
  }
}

/* ---------------------------------------------------------------------------
   encode pron and put it into entry_pron[ cur_entry ]
   cur_entry needs to be incremented by the calling function
   ---------------------------------------------------------------------- */

void encode_entry_pron( char *pron )
{
  int   match_cnt;
  int   len, base;
  int   i;

  /* no delta encode, to debug err encode alone */
  /*
  len = pron[0] + 1;
  entry_pron[ cur_entry ] = (char *) malloc( len );
  for (i = 0; i < len; i++) entry_pron[ cur_entry ][ i ] = pron[i];
  
  return;
  */
  /* delta encode */

  if ( cur_entry - 1 >= 0 ) {

    compare_entry_pron(pron, &match_cnt);

    /* need to match more than one char to be useful */
    
    if ( match_cnt > 1 ) {

      len = pron[0] - match_cnt + 1;
      entry_pron[ cur_entry ] = (char *) malloc( len + 1 );
      entry_pron[ cur_entry ][0] = len;
      entry_pron[ cur_entry ][1] = match_cnt;
      base = match_cnt + 1;
      len--;
      for (i = 0; i < len; i++) 
	entry_pron[ cur_entry ][ 2 + i ] = pron[ base + i ];

    } else {

      len = pron[0] + 1;
      entry_pron[ cur_entry ] = (char *) malloc( len );
      for (i = 0; i < len; i++) entry_pron[ cur_entry ][ i ] = pron[i];
    }

  } else {

    len = pron[0] + 1;
    entry_pron[ cur_entry ] = (char *) malloc( len );
    for (i = 0; i < len; i++) entry_pron[ cur_entry ][ i ] = pron[i];
  }
}

/* ---------------------------------------------------------------------------
   copy match pattern
   ---------------------------------------------------------------------- */

void copy_match_pattern(char *mp1, int *mpc1, char *mp2, int mpc2)
{
  int   i;

  for (i = 0; i < mpc2; i++)
    mp1[i] = mp2[i];

  *mpc1 = mpc2;
}

/* ---------------------------------------------------------------------------
   match

   Match the dictionary pronunciation with the default pronunciation.  This
   is a recursive function to determine the best way to encode the difference
   in pronunciation.
   ---------------------------------------------------------------------- */

void
match(int subc,       /* substitution count */
      int delc,       /* deletion count */
      int insc,       /* insertion count */
      int idx_dict,   /* index to dictionary pronunciation */
      int idx_def,    /* index to default pronunciation */
      char *match_pattern,
      int match_pattern_cnt)
{
  char  match_pattern_2[ MAX_PRON ];
  int   match_pattern_cnt_2;
  int   errc;
  int   i;
  
  errc = subc + delc + insc;
  copy_match_pattern(match_pattern_2, &match_pattern_cnt_2,
		     match_pattern, match_pattern_cnt);

  if ( idx_dict >= len_dict ) {          /* dict pron run out, insertion ? */

    insc = insc + len_def - idx_def;
    errc = errc + len_def - idx_def;
    for (i = 0; i < len_def - idx_def; i++) {
      match_pattern_2[ match_pattern_cnt_2 + i ] = INS;
    }
    match_pattern_cnt_2 += len_def - idx_def;

    if ( errc < gerrc ) {
      gsubc = subc;
      gdelc = delc;
      ginsc = insc;
      gerrc = errc;
      copy_match_pattern(g_match_pattern, &g_match_pattern_cnt,
			 match_pattern_2, match_pattern_cnt_2);
    }

  } else if ( idx_def >= len_def ) {    /* def pron run out, deletion ? */
  
    delc = delc + len_dict - idx_dict;
    errc = errc + len_dict - idx_dict;
    for (i = 0; i < len_dict - idx_dict; i++) {
      match_pattern_2[ match_pattern_cnt_2 + i ] = DEL;
    }
    match_pattern_cnt_2 += len_dict - idx_dict;

    if ( errc < gerrc ) {
      gsubc = subc;
      gdelc = delc;
      ginsc = insc;
      gerrc = errc;
      copy_match_pattern(g_match_pattern, &g_match_pattern_cnt,
			 match_pattern_2, match_pattern_cnt_2);
    }

  } else if ( pron_dict[ idx_dict ] == pron_def[ idx_def ] ) {

    if ( errc < gerrc ) {               /* match */

      match_pattern_2[ match_pattern_cnt_2 ] = MATCH;
      match_pattern_cnt_2++;
      match(subc, delc, insc, idx_dict + 1, idx_def + 1,
	    match_pattern_2, match_pattern_cnt_2);
    }

    if ( errc + 1 < gerrc ) {           /* deletion */

      copy_match_pattern(match_pattern_2, &match_pattern_cnt_2,
			 match_pattern, match_pattern_cnt);
      match_pattern_2[ match_pattern_cnt_2 ] = DEL;
      match_pattern_cnt_2++;
      match(subc, delc + 1, insc, idx_dict + 1, idx_def,
	    match_pattern_2, match_pattern_cnt_2);
    }

    if ( errc + 1 < gerrc ) {           /* insertion */

      copy_match_pattern(match_pattern_2, &match_pattern_cnt_2,
			 match_pattern, match_pattern_cnt);
      match_pattern_2[ match_pattern_cnt_2 ] = INS;
      match_pattern_cnt_2++;
      match(subc, delc, insc + 1, idx_dict, idx_def + 1,
	    match_pattern_2, match_pattern_cnt_2);
    }

  } else {
    
    if ( errc + 1 < gerrc ) {           /* substitution */
      
      match_pattern_2[ match_pattern_cnt_2 ] = SUB;
      match_pattern_cnt_2++;
      match(subc + 1, delc, insc, idx_dict + 1, idx_def + 1,
	    match_pattern_2, match_pattern_cnt_2);
    }
    
    if ( errc + 1 < gerrc ) {           /* deletion */
      
      copy_match_pattern(match_pattern_2, &match_pattern_cnt_2,
			 match_pattern, match_pattern_cnt);
      match_pattern_2[ match_pattern_cnt_2 ] = DEL;
      match_pattern_cnt_2++;
      match(subc, delc + 1, insc, idx_dict + 1, idx_def,
	    match_pattern_2, match_pattern_cnt_2);
    }

    if ( errc + 1 < gerrc ) {           /* insertion */

      copy_match_pattern(match_pattern_2, &match_pattern_cnt_2,
			 match_pattern, match_pattern_cnt);
      match_pattern_2[ match_pattern_cnt_2 ] = INS;
      match_pattern_cnt_2++;
      match(subc, delc, insc + 1, idx_dict, idx_def + 1,
	    match_pattern_2, match_pattern_cnt_2);
    }

  }
}
