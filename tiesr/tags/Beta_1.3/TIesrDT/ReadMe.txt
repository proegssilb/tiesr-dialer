Document License

 This work is licensed under the Creative Commons Attribution-Share Alike 3.0
 United States License. To view a copy of this license, visit 
 http://creativecommons.org/licenses/by-sa/3.0/us/
 or send a letter to Creative Commons, 171 Second Street, Suite 300,
 San Francisco, California, 94105, USA.

 
Contributors to this document

 Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/



----------------

The TIesrDT API provides a hypothesized pronunciation of a word based on the
word spelling.

Using the testtiesrdt Program
-----------------------------
 
The testtiesrdt program exercises the TIesrDT API. The command line for the
program is:

testtiesrdt  tree_path word_file preload_trees

tree_path: Directory where decision trees are located
word_file: Words for which to generate a pronunciation, one per line
preload_trees: (0/1) Load and keep all trees in memory for faster operation

An example of running this program from the top-level distribution directory on
a Linux OS is shown below. This test generates the hypothesized pronunciations
of three words, "yes", "no", and "maybe".

Dist/LinuxReleaseGnu/bin/testtiesrdt \
      Data/OffDT_GenDict_PhbVR_LE_MQ/English \
      Data/TestTIesrDTWords.txt \
      1

The output should be as follows:

yes:  y z
no:  n ow
maybe:  m ey b

Note that the TIesrDT pronunciations are not completely accurate. This is due to
the uncertainty of pronunciation of the "es" in "yes", since many words
pronounce "es" as "z". Similarly the "iy" phone is missing from the word
"maybe", since often a final "e" is not pronounced. These errors are corrected
by checking the dictionary pronunciation in TIesrDict.

You can determine the pronunciation of other words by supplying a list of words
other than the TestTIesrDTWords.txt file.