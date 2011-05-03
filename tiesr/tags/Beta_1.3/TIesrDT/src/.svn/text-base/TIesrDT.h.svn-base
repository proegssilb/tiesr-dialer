/*=======================================================================

 *
 * TIesrDT.h
 *
 * Header file for local implementation of TIesrDT API.
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

 This header file contains the constants and the TIesrDT structure
 used to process a word and produce a phonetic pronunciation.

======================================================================*/

#ifndef _TIESRDT_H
#define _TIESRDT_H


/* This version uses offset information to achieve speed-up. 
   In addition, leaf set is further compressed to save 
   pronunciation of the sub-set with the largest number of 
   leaves. The leaves are not saved so that search on the leaves
   is not conducted. 
*/
#define DEV_OFFSET


/*--------------------------------
  Standard file names and extentions for TIesrDT
  --------------------------------*/

/* File defining ASCII character and phonetic class attribute values. */
static const char* const FN_ATTVALUE = "cAttValue.txt";

/* File defining attribute context types */
static const char* const FN_ATTTYPE = "cAttType.txt";

/* File defining phones, pseudo-phones, and their phonetic classes. */
#ifdef DEV_OFFSET
static const char* const FN_NEWPHONE =  "onewphone.list";
#else
static const char* const FN_NEWPHONE =  "cnewphone.list";
#endif

/* Compressed decision tree file extension */
#ifdef DEV_OFFSET
//static const char* const EXT_LMDTPM =  ".clmdtpm";
static const char* const EXT_LMDTPM =  ".olmdtpm"; // offset-information DTPM
#else
static const char* const EXT_LMDTPM =  ".rlmdtpm"; // revised DTPM
#endif

/* Decision tree files have the basename of their ASCII character.
   This can be a problem for some characters that are interpreted by the shell,
   such as '.' or '\''.  So, these characters must have special names.  The mapping
   is given here. */

#define NUM_FN_CHAR  3 
static const char  FNCHAR[] = { '\'', '-', '.' };

static const char* const FNNAME[] = 
{
   "up",
   "dash",
   "dot"
};

/*-------------------------------- 
  Attributes 

  Decision tree attributes are specified as unsigned char indices that
  represent the presence of either 1) a particular ascii character, or
  2) a particular phonetic class such as voiced, unvoiced or short
  pause.  The number of attributes is limited in order to specify them
  in a limited number of bits in the decision tree files.  Attributes
  are enumerated in the FN_ATTVALUE file in the order of their
  respective indices.

 --------------------------------*/

/* Total number of attribute values allowed */
#define  NUM_ATT_VAL 64
#define  VAL_BITS  6

/* UNUSED_ATT specifies the index to be assigned for an attribute not used. */
#define UNUSED_ATT 255

/* Number of phonetic class attributes */
#define NUM_PHONE_CLASS 4

/* The phonetic class attributes are listed in the FN_ATTVALUE file as
   follows. The "NO_ATTRIBUTE" class must occur in the FN_ATTVALUE file, 
   but the others are optional.  If you add more class attributes, the
   "NO_ATTRIBUTE" class must remain the last one. */
static const char* const PHONECLASSSTRING[] = 
{
   "VOICED",
   "UNVOICED",
   "SHORT_PAUSE",
   "NO_ATTRIBUTE"
};

/* Index of phonetic classes in the classAtt array */
enum Phone_Class_t
{
   VOICED,
   UNVOICED, 
   SHORT_PAUSE,
   NO_ATTRIBUTE
};


/* ASCII characters allowed in a word are 0 .. MAX_ASCII.  However,
 note that due to the limited number of attributes, not all ASCII
 characters can be used, since they will not have an associated
 character attribute. Usually most words will only use A-Z
 anyway. */
#define MAX_ASCII  127


/*--------------------------------
  Attribute Contexts

  The attribute contexts define the context in which an attribute will
  be tested.  Context types include, for example, one charater to the
  right, two characters to the left, prior phonetic class context,
  etc.  

  --------------------------------*/

/* Number of allowable attribute contexts */
#define NUM_ATT_TYPE 16
#define TYPE_BITS  4

/* Left character, right character, and phonetic class context lengths
   in decision trees. Sum must be <= NUM_ATT_TYPE. The contexts, as
   defined below will eventually be configuration parameters in a
   configuration file, allowing the code to handle decision trees with
   various context lengths. */
#define DT_LEFT_CHAR  4
#define DT_RIGHT_CHAR  4
#define DT_LEFT_CLASS  4
#define  DT_CONTEXT_SIZE  12

/* In the code, the testing for context is done by accessing a context
   array based on the ordering of left character, right character, and
   left phone class context as defined above.  I am including an
   enumeration here that defines the indices for the contexts.
   However, this is not needed, but included just for
   documentation. */
enum Context_t
{
   CXT_LEFT4,
   CXT_LEFT3,
   CXT_LEFT2,
   CXT_LEFT1,
   CXT_RIGHT1,
   CXT_RIGHT2,
   CXT_RIGHT3,
   CXT_RIGHT4,
   CXT_LEFTCLASS4,
   CXT_LEFTCLASS3,
   CXT_LEFTCLASS2,
   CXT_LEFTCLASS1,

   #ifdef DEV_OFFSET
   /* there are 12 questions asked so far.
      since only 4 bits are used for these questions, the 0xf is reserved as the 
      context-free question */
   CXT_NONE = 15  
#else
   CXT_NONE = 255
#endif

};


/*--------------------------------
  Phones

  Phones include both phones, and pseudo phones.  Pseudo phones are
  multiple phones which map to a single ascii character, and are
  specified by separating multiple phones by '_', for example the
  character 'X' sometimes is mapped to the pseudo phone
  "eh_k_s". Phones are enumerated in the FN_NEWPHONE file, which also
  specifies the phone class for each phone or pseudo phone.  The
  number of phones allowed is limited in order to represent them in
  the decision trees in a reduced number of bits.  The decision trees
  represent phones by indices based on their order in the FN_NEWPHONE
  file.  The format of the FN_NEWPHONE file is one phone name on a
  line, followed by whitespace followed by the phones class type as
  specified by one of the phoneClassString array strings.

  --------------------------------*/

/* Number of phones allowed. */
#define  NUM_PHONES  128
#define  PHONE_BITS  7



/*-------------------------------- 
  Decision tree bit allocation.

  The decision tree consists of an array of bytes specifing 1) a branch set,
 2) a question node or 3) a leaf set. 
 
 A leaf node set is a sequence of
  bytes consisting of:

  - The size of the leaf set, with the ms bit = 0
  - For each phone index that occurs in the leaf node set:
    - The phone index
    - The number of leaf nodes corresponding to this phone
    - a sequence of indices specifying the attribute values for the leaf nodes

  The sequence of attribute values is encoded such that negative
  numbers indicate a range of attributes.  For example, if the
  attributes corresponding to a phone have the indices 1,2, 3 through 8, 9,
  10, then this will be encoded as the sequence of bytes as:
  1,2,3,-8,9,10
  
  A branch set node consists of a byte suquence encoded in the same as above
 for leaf set. The bit fields consists:
  - The size of the branch set, with ms code 0xc0 or 11 in the top two bits
  - Run-lengh code of the branch attribute values
  Offset of the branches inside the branch set node is coded as following
  - The number of branches in byte
  - For each branch, use a short to save branch leaf attribute type (question) 
and offset to its sibling branch

 A question node consists one byte. The bit fields consits:
  - The ms as 0x80, i.e., 10 in the top two bits of the byte
  - attribute type in 6 bits

  --------------------------------*/
typedef enum{
  LEAFSET = 0,
    QUESTION,
    BRANCHSET} eNodeType;

static const unsigned char LEAFBIT = 0x80;
#ifdef DEV_OFFSET
static const unsigned char NODE_QUESTION = 0x40;
static const unsigned char NODE_BRANCHSET = 0xc0;
static const unsigned char NODE_LEAFSET = 0x00; 
#endif

static const unsigned short BRANCHCONTEXT = 0x7800;
static const unsigned short BCXTSHIFT = 11;
static const unsigned short BRANCHATTRIBUTE = 0x007F;
static const unsigned short BRANCHLEAFCONTEXT = 0x0780;
static const unsigned short BLEAFCXTSHIFT = 7;
#ifdef DEV_OFFSET
static const unsigned short BRANCHOFFSET = 0x0fff; 
static const unsigned short BRANCHLEAFCONTEXT_RLM = 0xf000;
static const unsigned short BLEAFCXTSHIFT_SHIFT = 12; 
static const unsigned char  QUESTION_IN_QUESTION_NODE = 0x3f; 
#else
static const unsigned short BRANCHPHONE = 0x001f;
static const unsigned short BRANCHATTRIBUTE_RLM = 0x7e00; 
static const unsigned short BATTSHIRT = 9;
static const unsigned short BRANCHLEAFCONTEXT_RLM = 0x01e0;
static const unsigned short BLEAFCXTSHIFT_RLM = 5;
#endif

/*--------------------------------
  Other constants
  --------------------------------*/

/* Maximum string size allowed for phones, attributes, class characters */
#define  MAX_STRING  32

/* Maximum full file name size */
#define MAX_FILENAME 256

/* TRUE and FALSE */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct RootInfo_s
{
  unsigned char numBranch; 
  char *AttVal; 
  int BranchOffset[NUM_ATT_VAL]; 
} RootInfo;

// the information of the number of answers per question 
typedef struct stInfoAtts_s
{
  short numAttTypes; 
  short numAttValPerAttType[NUM_ATT_VAL]; 
} stInfoAtts; 

/*--------------------------------
 The TIesrDT instance structure
 --------------------------------*/
typedef struct TIesrDT_Struct
{
   /* Path to the decision trees and support files */
   char* treePath;
   
   /* Flag indicating that trees are preloaded, and don't need to
      be loaded for each letter of each word */
   int preLoaded;
   
   /* Pointers to the tree for each character attribute */
   unsigned char *treePointers[MAX_ASCII+1];
   
   /* Number of attribute values */
   short numAtt;
   
   /* Attributes for each ASCII character, for example charAtt['A']. */
   unsigned char charAtt[MAX_ASCII+1];
   
   /* Phoneme class attribute values.  The attribute values that
    represent voiced, unvoiced, short pause, etc. */
   unsigned char classAtt[NUM_PHONE_CLASS];
   
   /* Number of phonemes, and string of each */
   short numPhones;
   char* phone[NUM_PHONES];
   
   /* Class attribute values for each phoneme.  These indicate
    whether the phone is voiced, unvoiced, short pause, or
    perhaps maybe some other attributes in the future. */
   unsigned char phoneAtt[NUM_PHONES];
   
   
   /* Tree structure parameters used during processing */
   
   RootInfo root;
   
   /*
   the information on the number of answers per question
    */
   stInfoAtts stAttInfo;
   
   /* Size of tree in bytes */
   int treeSize;
   
   /* Pointer to the tree */
   unsigned char* tree;
   
   /* Current byte offset into the tree. */
   int treeOffset;
   
   /* Top branch context */
   unsigned char topBranchContext;
   unsigned char cxtfrPhone; // context free phone of the letter
} TIesrDT_Struct_t;


/*----------------------------------------------------------------
  Local API functions 
  ----------------------------------------------------------------*/


/*----------------------------------------------------------------
  LoadAttributes

  This function loads the character attributes value array in the
  TIesrDT structure, and the class attribute value array.  The
  character attributes value array, charAtt, is an unsigned byte
  array, indexed by the ascii value of the character.  It returns the
  attribute value corresponding to a character, for example,
  attValue['A'].  The charAtt array is sized to hold attribute values
  for ascii characters up to and including ascii value MAX_ASCII.
  However, the decision tree code is such that it limits the number of
  attribute values that can be assigned to NUM_ATT_VAL.  Thus some
  charAtt array entries may not be used.

  The class attribute value array classAtt, specifies that attribute value
  for each of the class attributes; voiced, unvoiced, etc.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadAttributes( TIesrDT_t aTIesrDT );


/*----------------------------------------------------------------
  LoadPhones

  This function loads the phones and their respective class attributes
  from the FN_NEWPHONE file.  This file has the format one entry per
  line consisting of a phone string followed by whitespace followed by
  the phone's class string.  The class string is one of the strings in
  the phoneClassString array.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadPhones( TIesrDT_t aTIesrDT );


/*----------------------------------------------------------------
 PreLoadTrees

 This function loads all of the decision trees from the location
 specified when TIesrDT_Create is called.  Space is allocated on the
 heap for the trees, and the treePointers array is filled with
 pointers to each loaded tree.

 -----------------------------------------------------------------*/
static TIesrDT_Error_t PreLoadTrees( TIesrDT_t aTIesrDT );


/*-----------------------------------------------------------------
 GetTreeFileName

 Obtain the tree file name corresponding to a character attribute.
 The aFileName must point to a character array large enough to
 hold the file name.
 -------------------------------------------------------------------*/
static TIesrDT_Error_t GetTreeFileName( TIesrDT_t aTIesrDT, char aChar, char *aFileName );


/*----------------------------------------------------------------
 GetDTPhoneIndex

 This function traverses a decision tree to determine the phone index
 that should be output for the given character of a word in its context.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t  GetDTPhoneIndex( TIesrDT_t aTIesrDT, char aChar,
					 const unsigned char* aContext, 
					 unsigned char* aPhoneIndex );


/*----------------------------------------------------------------
  LoadTreeFile

  This function loads the compressed tree file corresponding to the
  given character from the path in the TIesrDT object, and puts the
  pointer to the tree in the TIesrDT object.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t LoadTreeFile( TIesrDT_t aTIesrDT, char aChar );


/*----------------------------------------------------------------
  AtLeaf

  This function determines if the present offset is at a leaf node
  set, which is indicated by the top bit of the current byte having a
  zero.  This can be put in a macro later, but for now it is a
  function for debugging.

  ----------------------------------------------------------------*/
static short AtLeaf( const unsigned char* aTree, int aOffset );


/*----------------------------------------------------------------
  GetBranchInfo

  This function retrieves the information encoded in the bit fields of
  the two-byte branch node pointed to by the offset into the tree.
  This can be put in a macro later, but for now it is a function for
  debugging.

  ----------------------------------------------------------------*/
#ifdef DEV_OFFSET
static TIesrDT_Error_t GetBranchInfo( const unsigned char* aTree,
				      int aOffset,
				      unsigned short* aBranchOffset,
				      unsigned char* aBranchLeafContext,
				      unsigned int * pSize );
#else
static TIesrDT_Error_t GetBranchInfo( const unsigned char* aTree,
				      int aOffset,
				      unsigned char* aBranchContext,
				      unsigned char* aBranchAttribute,
				      unsigned char* aBranchLeafContext ,
				      unsigned int *pTreeOffset);
#endif

/*----------------------------------------------------------------
  FindTopBranchContext

  This function searches through the decision tree to find the first
  branch node.  Its context defines the top branch context.  If the
  decision tree has no branch nodes, then it is assumed that the 
  decision tree consists of a single leaf node that defines the output
  phone index for all possible contexts.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t FindTopBranchContext( TIesrDT_t aTIesrDT );


/*----------------------------------------------------------------
  SearchLeafSet

  This function searches a leaf node set to determine if any of the
  leaf nodes of the set contain the attribute value that matches the
  specified context.

  ----------------------------------------------------------------*/
#ifdef DEV_OFFSET
static TIesrDT_Error_t  SearchLeafSet( TIesrDT_t aTIesrDT, 
				       const unsigned char* aContext, 
				       short aMatchContext,
				       short *aPhoneIndex );
#else
static TIesrDT_Error_t  SearchLeafSet( TIesrDT_t aTIesrDT, 
				       const unsigned char* aContext, 
				       short aMatchContext,
				       short *aPhoneIndex ,
				       short *ptrNumLeaves);

/*----------------------------------------------------------------
  SkipPastLeafSet

  The offset pointer argument is pointing to the start of a leaf node
  set of a tree. Move the offset pointer past the leaf node set.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t SkipPastLeafSet( const unsigned char* aTree, 
					int *aOffset ,
					short * nVisitedLeaves );



/*----------------------------------------------------------------
  SearchTopBranch

  This function examines a top branch node to determine if the branch
  node context and attribute match the word context.  If so, this
  function continues down the branch to find the branch and leaf node
  of the branch that correspond to the word context.  The phone index
  of the matching leaf node is output.

  If this top branch node context and attribute do not match the word
  context, then the leaf nodes of the branch are skipped, and this
  function does not return a valid phoneIndex.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t SearchTopBranch( TIesrDT_t aTIesrDT,
					const unsigned char* aContext,
					short* aPhoneIndex );


/*----------------------------------------------------------------
  SearchBranch

  This function searches a branch to see if it matches the word
  context.  There are two conditions that must hold true in order to
  match the word context.

  First, the branch context and attribute must match the word context
  attribute.  If this does not match, then this function sets the
  aNoMatch flag, skips the leaf node set corresponding to the branch
  node, and returns.

  Second, the leaf node set associated with the branch must contain an
  attribute that matches the branch node leaf context specified in the
  branch node.  If a match is found, then the aPhoneIndex will contain
  a valid phone index.  Otherwise it will contain a negative number.
  
  iNumChecked return the number of branchs and leaves that this function 
  has checked/visited. 
  ----------------------------------------------------------------*/
static TIesrDT_Error_t SearchBranch( TIesrDT_t aTIesrDT,
				     unsigned char curContext, 
				     const unsigned char* aContext, 
				     short *aNoMatch, short *aPhoneIndex,
				     short *iNumChecked);


/*----------------------------------------------------------------
  FindNextBranch

  This function searches through the tree to find the next branch that
  has the desired branch context.  This function is used in decending
  into the tree nodes from an initial matching branch node context.

  ----------------------------------------------------------------*/
static TIesrDT_Error_t  FindNextBranch( TIesrDT_t aTIesrDT, unsigned char aBranchContext );

#endif

/*----------------------------------------------------------------
  ToUpper

  Make a string uppercase.

  ----------------------------------------------------------------*/
static void ToUpper(char *string );


/*----------------------------------------------------------------
  Deblank

  Find location past leading whitespace of a string, and make the string
  such that it is only the first non-blank portion of the original
  string.
  
 ----------------------------------------------------------------*/
static char*  Deblank( char *word );

#endif

