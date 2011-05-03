Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
ALL RIGHTS RESERVED

----------------

The TIesrDict API generates a phonetic pronunciation of a word using either a
dictionary entry, or a decision tree algorithm.

Using the testtiesrdict Program
-----------------------------
 
The testtiesrdict program exercises the TIesrDict API. The command line for the
program is:

testtiesrdt  data_path language dict_dir 0 2 [namelist]

data_path: Directory where TIesrFlex data is located
language:  Language directory, such as "English"
dict_dir:  Dictionary directory name, usually "Dict"
use_dt:    Show TIesrDT pronunciation even if a word is in the dictionary
max_pron:  Number of pronunciations to show, if multiples are in the dictionary
[add_cl]:  Add closure phones prior to stop consonants. Enabled by default.
This option adds stop consonant phones (bcl,dcl,gcl,kcl,pcl,tcl) prior to the
consonant phones (b,d,g,k,p,t), and is somewhat specific to English. 
[namelist] Optional file of names.  If not given program is interactive.

An example of running this program from the top-level distribution directory on
a Linux OS is shown below. This test generates the dictionary pronunciations of
three words, "yes", "no", and "maybe".

Dist/LinuxReleaseGnu/bin/testtiesrdict \
      Data/OffDT_GenDict_PhbVR_LE_MQ/ \
      English \
      Dict \
      0 2 1\
      Data/TestTIesrDTWords.txt


The output should be:

yes ---> y eh s.
no ---> n ow.
maybe ---> m ey bcl b iy.

As can be seen, the dictionary lookup by TIesrDict corrects errors in
pronunciation that would be obtained by TIesrDT.

If the optional [namelist] is not specified, the program will first output a
list of all phones in use for the language and then prompt the user
to input a word, and will provide information about the pronunciations.  To
terminate the program, when prompted for a name, input only a period "."

