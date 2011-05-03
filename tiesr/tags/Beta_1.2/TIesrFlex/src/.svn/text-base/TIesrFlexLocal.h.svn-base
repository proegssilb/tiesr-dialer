/*=======================================================================
 TIesrFlexLocal.h

 Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
 ALL RIGHTS RESERVED


 This header contains a few constants used in TIesrFlex class that
 are not used to declare structure sizes of member variables.

======================================================================*/

/*----------------------------------------------------------------
  TIesrFlexLocal.h

  This header contains a few constants used in TIesrFlex class that
  are not used to declare structure sizes of member variables.

  ----------------------------------------------------------------*/
#ifndef _TIESRFLEXLOCAL_H
#define _TIESRFLEXLOCAL_H


//--------------------------------
// Defined function macros 
//
#define   MAX(a,b)  ((a) > (b) ? (a): (b))
#define   MIN(a,b)  ((a) < (b) ? (a): (b))
#define long2short(x) (short)(MIN(MAX(x, -32768),32767)) /* clip */



//--------------------------------
//  Constants

// Mean vector to add
static short mean_bias[] = {14951, -2789, -811, 1563, -3013, -1346, 973, -884, 355, -496}; 

// The prefix representing special silence words, used to
// manually insert silence words within the grammar.
char const *const CTIesrFlex::SIL = "_SIL";


//--------------------------------
// These are the arrays for executing parsing of the ascii input string
// according to the parser.

// yy parser token number defines needed
static const int       START  = 257;
static const int       NAME   = 258;
static const int       ARROW  = 259;
static const int       LPRN   = 260;
static const int       RPRN   = 261;
static const int       RBKT   = 262;
static const int       LBKT   = 263;
static const int       VBAR   = 264;
static const int       PERIOD = 265;


// yy parser  static table variables
const char CTIesrFlex::yytranslate[] = {     0,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
7,     8,     9,    10,    11
};

const short CTIesrFlex::yyr1[] = {     0,
12,    12,    13,    13,    14,    15,    15,    15,    16,    16,
17,    17
};

const short CTIesrFlex::yyr2[] = {     0,
5,     6,     1,     2,     4,     1,     3,     3,     1,     2,
1,     3
};

const short CTIesrFlex::yydefact[] = {     0,
0,     0,     0,     0,     1,     0,     2,     3,     0,     4,
6,     0,     0,     9,    11,     0,     0,     0,    10,     0,
5,     7,     8,    12,     0,     0,     0
};

const short CTIesrFlex::yydefgoto[] = {    25,
7,     8,    14,    15,    16
};

const short CTIesrFlex::yypact[] = {     4,
-2,     9,    10,     5,    14,    15,    14,-32768,    -3,-32768,
-32768,    -3,    -3,-32768,    -3,     1,    -5,     0,-32768,    -3,
-32768,-32768,-32768,    -3,    19,    21,-32768
};

const short CTIesrFlex::yypgoto[] = {-32768,
-32768,    16,   -15,     6,     2
};

const short CTIesrFlex::yytable[] = {    19,
11,    22,    12,     2,    20,    13,     1,    23,    19,    20,
20,    21,     3,    17,    18,     5,     4,     6,    26,     9,
27,     0,    10,     0,     0,    24
};

const short CTIesrFlex::yycheck[] = {    15,
4,     7,     6,     6,    10,     9,     3,     8,    24,    10,
10,    11,     4,    12,    13,    11,     7,     4,     0,     5,
0,    -1,     7,    -1,    -1,    20
};


#endif // _TIESRFLEXLOCAL_H
