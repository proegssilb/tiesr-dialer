Document License

 This work is licensed under the Creative Commons Attribution-Share Alike 3.0
 United States License. To view a copy of this license, visit 
 http://creativecommons.org/licenses/by-sa/3.0/us/
 or send a letter to Creative Commons, 171 Second Street, Suite 300,
 San Francisco, California, 94105, USA.

 
Contributors to this document

 Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/


-----------------

This directory and its sub-directory tree contain data used by the TIesrFlex,
TIesrDict, and TIesrDT APIs.  The data is for general American English. A user
utilizes the TIesrFlex API to create a set of binary grammar network and model
acoustic data files for an input grammar.  The sub-directories are described
below. All information is in a standard little-endian format so that it can be
used with Linux, Windows, or any other operating system using little-endian
format.

The English sub-directory holds files that contain binary pronunciation decision
trees for each letter and several punctuation marks, plus some additional
supporting files.  The TIesrDT API uses the decision trees to determine a word
pronunciation from the spelling of the word. The decision trees contain data
based on training over a large number of word pronunciations. The decision trees
provide a phonetic pronunciation for each letter of the word based on the
surrounding context of the letter. The binary decision tree file format was
designed to reduce file size while containing offset information for fast node
location so that the generation of word pronunciations can be accomplished with
minimal processing.

The English/Dict sub-directory contains a binary phonetic pronunciation
dictionary in the file dict.bin containing over 96000 entries. The entries
include common English words and names, and includes multiple pronunciations for
some words. The size of the binary dictionary file is minimized by encoding only
the variance between the decision tree pronunciation of the word and the actual
accepted pronunciation. The sub-directory also contains a listing file,
phone.lis, that lists the phones used in the dictionary.

The English/Models sub-directory contains binary acoustic model information that
is used to construct the acoustic phonetic models needed to recognize the words
specified in a grammar. TIesrFlex uses this data to create a set of
gender-dependent triphone acoustic Hidden Markov Models (HMMs) which are
concatenated to define the the acoustic models forming words. Compression
techniques have been used to reduce the size of the binary files.  The HMM model
information is in compressed byte format with vq clustered transition and mean
vector data.

