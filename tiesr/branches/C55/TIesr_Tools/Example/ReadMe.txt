Document License

 This work is licensed under the Creative Commons Attribution-Share Alike 3.0
 United States License. To view a copy of this license, visit 
 http://creativecommons.org/licenses/by-sa/3.0/us/
 or send a letter to Creative Commons, 171 Second Street, Suite 300,
 San Francisco, California, 94105, USA.

 
Contributors to this document

 Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/


Introduction
============
This directory contains files and sub-directories that illustrate the use of the
TIesr tools to create a new set of data files for use with the TIesr speech
recognizer. The example uses the tools in the TIesr_Tools directory, and follows
the discussion provided in the Documentation/TIesr_Data_Training.txt file. Since
this is a tutorial example, it is not intended to produce a data set that
provides very good performance, but rather a data set that will work with the
TIesr recognizer and illustrates the concepts of data file creation. Comments in
this document will make mention of areas where additional steps could be taken
to refine performance for a real-life task.

It is assumed that all of the external and TIesr tools mentioned in the
Documentation/TIesr_Data_Training.txt document have been obtained and built, so
that all tools are available. 

Many steps use the HTK tools. These tools are often case-sensitive, input text
files usually require a newline at the end of the last line, and any directory
names as output arguments appearing on the command line must already exist.
Having the wrong case, omitting the newline, or specifying a non-existant
directory name will cause the tool to appear to fail, so you must use care when
running the HTK tools.

It would be possible to combine many of the steps described below into
higher-level script files. However, the TIesr tools allow the user the
flexibility to influence the data file creation process at every step. The
purpose of this example is to understand each of the steps in preparing the data
files, so each step is explained and possible modifications to processing are
mentioned.

This example provides commands as they would be entered on a Linux OS
development system. Appropriate adjustments must be made for other systems.
 
 
Step 1: Data Preparation
========================
For this tutorial example, assume that the vocabulary of a simple language
contains the three words:

NO
OKAY
YES

Step 1a: Preparing a pronunciation dictionary
---------------------------------------------
We construct a text file dictionary that explains the pronunciation of the words
in terms of phonetic sounds. For this example, assume a nominal pronunciation as
in English. The dictionary is found in the file words.dic and it consists of the
three entries:
 
NO   n ow
OKAY ow k ey
YES  y eh s

Here we enter the words in upper case and the phones in lower case for easy
distinction, although this is not necessary. Enter the words in the dictionary
alphabetically, since this is important for some of the tools to work correctly.
The dictionary defines a set of monophones ( eh, ey, k, n ow, s, y ) that make
up the phonetic sounds in the language. Normally there would be many more words
in the language, so that the number of monophones would be much fewer than the
number of words in the dictionary. In a real-life situation, the text dictionary
should be made up of a large set of common words in the language; enough words
to be able to estimate spelling-to-pronunciation rules.

Step 1b: Preparing the speech database
--------------------------------------
The next step prepares the speech database files for use in training HMMs for
models. For this example, the Data directory contains the raw recordings of five
females and five males saying each word. The files are stored in the gzipped tar
file Data/AUDIO.tar.gz. Unzip these files to the Data directory. Each file has
the .raw file extension. These files contain acoustic data sampled at 8kHz. Each
sample is a signed 16-bit number stored in little-endian format. The file names
indicate the gender, speaker number, and word. It is a good idea to provide
descriptive directory and file names for the speech database directories and
files so that one can search and create lists of speech files easily.


Step 2: Building Pronunciation Decision Tree Data Files
=======================================================
Pronunciation decision trees allow one to determine the pronunciation of words
from their spelling. For this example, suppose the word OKNO may exist in the
vocabulary, but is not included in the dictionary or speech database. We want to
be able to determine the phonetic pronunciation of this word. This is the
purpose of the decision tree data.

Step 2a: Constructing a letter-to-phone dictionary
--------------------------------------------------
In this step we construct our best guess of the mapping between letters and
monophones. Suppose in our example, we don't really know if the letter A or
the letter Y in the word OKAY should map to the monophone ey. In addition, we
allow each letter to possibly be silent. Since the letter Y may be pronounced
using the phone "ey" or "y", this is included in the mapping. Additionally, we
guess that the letter E may be pronounced as "eh" or "ey". This yields the file
ltp.map which contains:

A: ey .8, _sil .2
E: eh .6, ey .2, _sil .2
K: k .8, _sil .2
N: n .8, _sil .2
O: ow .8, _sil .2
S: s .8, _sil .2
Y: y .6, ey .2, _sil .2

The exact probabilities and mappings is not all that important, since training
will establish the most likely resulting mappings for each word in the
dictionary. However, it is important that the mapping in ltp.map be flexible
enough to cover the words in the dictionary. For a real-life vocabulary this
usually requires that each letter map to many different phones, combinations
of phones known as pseudophones, or silence. To enter a combination of phones
as a pseudophone, just enter several phones separated by spaces prior to the
phone probability.

To train the dictionary that performs a one-to-one mapping of letters to
monophones, create a directory, here named workdir, and a file that specifies
that the HTK tools used within the perl script should read and write in natural
machine endianness, as in the file NaturalOrder.cnf, then run the TIesr tool:

mkdir workdir
htk_ttp_map.pl -C NaturalOrder.cnf \
  -d words.dic \
  -l ltp.map \
  -w workdir -n 4 -t 3 \
  -o words_ltp.dic

This produces the words_ltp.dic file. The directory workdir contains quite a few
files and directories generated during training of the words_ltp.dic dictionary,
and can get quite large. Usually there is no need to examine the detailed
maximum-likelihood training letter-to-phone training results, but it is in
workdir if you wish to see it. You will probably want to delete the contents of
the directory. The words_ltp.dic file contains:

"NO"                 n ow
"OKAY"               ow k ey _
"YES"                y eh s

This maps each letter of a word to either a monophone, or a null symbol (_).
Thus in our simple case, the mapping chose to associate the letter A, not the
letter Y, with the monophone "ey" in the word OKAY.

In a real-world letter-to-phone dictionary output there may be mappings from a
letter to a pseudophone consisting of more than one phone if the ltp.map file
contains a sequence of phones that can map to a letter. In real-world mapping
results, you may also notice phone mappings that include "*", indicating failure
to successfully create a dictionary entry for that word. This is due to a
letter-to-phone map file that does not contain a mapping that can explain the
word, or the -n option to htk_ttp_map.pl does not specify a large enough
sequence of null outputs for a sequence of letters. For large vocabularies you
will likely want to modify the letter-to-phone map file and iterate running the
htk_ttp_map.pl tool until you are satisfied with the letter-to-phone dictionary.

A small number of words may not successfully be mapped in the letter-to-phone
dictionary, and this is usually not a problem. This is often due to foreign
words that have pronunciations that are quite different than what would be
determined using pronunciation rules normally associated with a language.

When you are satisfied with the letter_to_phone dictionary, remove any remaining
entries that did not map successfully that contain the "*" character.

Step 2b: Creating the DTPM files
--------------------------------

First we remove any existing directories and files in the work directory.

rm -rf workdir/*

Next, we create the labels for all of the phones and pseudophones used in the
ltp.dic file. This requires a list of all of the phones and pseudophones in the
words_ltp.dic file. You can use the HTK tool HDMan to generate this list, or
generate the list manually. For this example, the HDMan tool is used to show how
to make the listing in the file pseudophone.list. The list of phones and
pseudophones created by HDMan is sorted in alphabetic order in the
pseudophone.list file. While a sorted list is not absolutely necessary, it is
recommended since eventually the order of phones will be important. Keeping the
phone order sorted will remove the possibility of an error due to ordering.

HDMan -n temp.list /dev/null words_ltp.dic 
sort temp.list >pseudophone.list
rm temp.list

After creating the pseudophone.list file we edit it to assign labels to phones
and pseudophones. We assign each phone or pseudophone a label of "VOICED",
"UNVOICED", or "SHORT_PAUSE" for the "_". The final pseudophone.list file
contains:

_   SHORT_PAUSE
eh  VOICED
ey  VOICED
k   UNVOICED
n   VOICED
ow  VOICED
s   UNVOICED
y   VOICED


Next, we create the training data for the decision trees using the
ttp_dict_tree_data.pl tool. For our example we run the tool with options set to
comply with the parameters that the TIesrDT API is presently set to use by
specifying the pseudophone.list file and the option -pL 4. The
ttp_dict_tree_data.pl tool provides options that can create a flexible set of
parameters for pronunciation decision tree training. While the TIesrDT API can
be modified to accommodate a different sets of parameters, this requires
modification of the TIesrDT API code.

mkdir Data/Lang
ttp_dict_tree_data.pl -d words_ltp.dic -p pseudophone.list -pL 4 \
                      -o Data/Lang -w workdir


The output of this tool are three of the desired support files in the Data/Lang
directory, and data files to be used for training the pronunciation decision
trees in the workdir directory.


Now the decision trees can be trained using the train_trees.pl tool. Here the
confidence factor is set to 25, which will produce smaller but perhaps less
accurate trees. You can modify the confidence factor between 1 and 99 to
determine the trade-off between size of trees and accuracy of pronunciation
prediction. For this example the decision trees degenerate into using a single
phone for each letter.

train_trees.pl -D workdir  -CF 25 

This results in a set of trained decision tree files in workdir.

The final step in creating decision trees is to compress the data into binary
files. This uses the odtcompress.pl tool.

odtcompress.pl -D workdir -o Data/Lang -CF 25 

This results in the decision tree data files *.olmdtpm placed in Data/Lang. At
this time you can remove the files in workdir

rm workdir/*

If you wish, you can test the decision tree files using the testtiesrdt program
supplied with the release. Remember that in our simple example, the only
letters that exist are A,E,K,N,O,S,Y.  For example, to see what the decision
trees provide for the pronunciation of the made-up word OKNO that was discussed
earlier we can run:

echo OKNO >xtemp.txt ; testtiesrdt Data/Lang xtemp.txt 1; rm xtemp.txt

This yields the output showing how to pronounce the word:

OKNO:  ow k n ow

You can try other made-up words as well to see how the decision trees perform.
In real-world training of decision trees, there will be some cases in which
the pronunciation is not deduced correctly from the spelling since the decision
trees usually will not be able to classify all pronunciations correctly.

Step 3: Building the Compressed Binary Dictionary Files
=======================================================
The dictionary files hold pronunciations for a set of vocabulary words. The
purpose of the dictionary files is to override pronunciations that the decision
trees produce, and to add entries of words that have multiple pronunciations.

The dictionary files are by convention usually held in a directory named Dict
under the decision trees, so for the example we create the directory
Data/Lang/Dict.

In this example we start with the dictionary prepared in Step 1a, in words.dic.
Suppose that we want to add an additional new word, KES, which is normally
pronounced as "k eh s" but may also be pronounced occasionally as "k ey s". We
edit a copy of the words.dic dictionary to add the pronunciations, remembering
to sort the dictionary in word alphabetical order. If there are multiple
pronunciations for a word in the dictionary, the preferred pronunciation should
come first. The new dictionary in allwords.dic contains:

KES  k eh s
KES  k ey s
NO   n ow
OKAY ow k ey
YES  y eh s

Next we create a list of the phones in the dictionary in alphabetical order,
again using the HDMan tool and putting the list in Lang/Dict/phone.lis:

mkdir Data/Lang/Dict
HDMan -n temp.list /dev/null allwords.dic
sort temp.list >Data/Lang/Dict/phone.lis; rm temp.list

Finally, we create the compressed binary dictionary using the dictproc tool.
The compressed dictionary contains only the differences between the decision
tree rule pronunciation and the actual pronunciation in the prepared text
dictionary. 

dictproc allwords.dic Data/Lang/Dict/phone.lis Data/Lang/Dict/dict.bin \
         little_endian 0 Data/Lang
         
This creates the Data/Lang/Dict/dict.bin binary dictionary.

If you wish you can test the new dictionary just created using the testtiesrdict
program, again keeping in mind that in our simple example, the only letters that
exist are A,E,K,N,O,S,Y. Testtiesrdict can be run in an interactive mode, so
you can try any number of words. Note that when you enter a dictionary word,
the dictionary pronunciation is output, but other made-up words use the
decision trees to obtain a pronunciation. For example if we interactively run
testiesrdict as follows and input the words "KES" and "YES" we obtain:

testtiesrdict . Data/Lang Dict 0 2 0

Number of phones used: 7
eh
ey
k
n
ow
s
y

Enter Name: KES
KES has 2 entries in dictionary
Rule:  KES ---> k eh s
Dict(1) for KES:  k eh s
Dict(2) for KES:  k ey s
Used now: k eh s

Enter Name: YES
YES has 1 entries in dictionary
Rule:  YES ---> eh s
Dict(1) for YES:  y eh s
Used now: y eh s

From the output we see that the pronunciations from the dictionary for the word
"KES" both appear, as well as noting that the decision tree rule matches the
first pronunciation. For the word "YES", the decision tree rule does not match
the dictionary, but the dictionary pronunciation is available and corrects the
decision tree rule.

Step 4: Building the Acoustic Model Data Files
==============================================
This step is the most involved and has a large number of variations, since it
requires training HMMs for each of the phones using the HTK tools. Much research
is still being done on better ways to train HMMs for phonetic models. In this
example we present the steps needed to create a basic set of HMMs for TIesr and
and comment where some of the possible variations could be made.

Step 4a: Convert raw data to HTK format
---------------------------------------
This step converts the raw data files into an HTK-compatible binary format file
containing feature vector data that is compatible with the TIesr recognizer.
The sample_to_htk.pl tool performs the conversion from the 16-bit/sample 8kHz
sample rate data.
 
 for file in Data/*.raw; do sample_to_htk.pl -l 0.5 -o Data $file; done

This will convert all of the *.raw files to *.htk files in the Data directory.
For future use, we create a file that lists all of the *.htk files:

ls -1 Data/*.htk > Data/HTK_files.list

Step 4b: HTK training - Create label files for each utterance
-------------------------------------------------------------
Following the HTK documentation, we create label files for each of the
utterances in the *.htk files. Rather than create a separate label file for each
utterance, we create a master label file (MLF) holding the utterance for each
file. For a small set of data such as our example, this can be done by hand. For
a large speech database for a language, this would usually require developing
a script file to generate the MLF. The MLF for the example is located in
Data/Words.mlf. This file defines the words spoken in each utterance file.

Next we create a new MLF file containing the phones spoken in each file. HTK
provides the HLEd tool that can generate the MLF file. At the same time, we add
the "sil" model at the beginning and end of each utterance to account for
leading and trailing silence in the utterances. We create a label editing
command file in phones.led that contains the following:

EX
IS sil sil

Here EX is the command to expand from words to phones according to the
pronunciation dictionary, and the IS command puts the "sil" model at the
beginning and end of each utterance. In a large vocabulary task or a task in
which multiple words are in an utterance, an additional model, or the sil model
must be included between words to model possible inter-word silence during
training. HTK documentation calls the additional model the "sp" phone model.
Consult the HTK documentation for methods of modeling inter-word silence.

Run HLEd as follows:

HLEd -d words.dic -l '*' -i Data/Phones.mlf phones.led  Data/Words.mlf

HTK training tools provide several different methods to create the initial HMMs.
They can be initialized without any detailed acoustic information, or they can
be initialized with acoustic data specified by time segment information in the
label files. For our example, we start by providing time segments for each of
the phones in the acoustic files. We do this by making a copy of the
Data/Phones.mlf file in Data/Segmented_phones.mlf and editing the file to
specify time segments of each phone found by inspection of the data in each
file. This can be a very laborious process, so for large language databases one
can either use HTK to start training HMMs without any time-segmented data, or
provide time segments for a very small portion of the database. Sometimes large
databases provide phonetic time segmentation for some of the utterances, in
which case a script could be written to generate the segmented MLF file.

Step 4c: HTK training - Constructing initial HMMs for each phone
----------------------------------------------------------------
First, we create a simple HTK configuration file, UserDelta.cnf so that the
delta feature parameters will be dynamically calculated from the
features in the Data/*.htk database files.

We create an HTK Master Model File (MMF) containing prototype HMMs for each of
the phones and the sil model in Data/HMMPROTO/phones.mmf. The actual values of
the HMM state distribution means and variances is not important, just the vector
size and number of means and variances, since training will update these. Each
HMM has three emitting states, with a single Gaussian distribution for each
state.

In addition we create a second MMF containing a single global prototype HMM for
the purpose of determining the global mean and variance of all of the training
data. This is used as a means to create a vFloors MMF to limit the variance
estimate of all of the phone HMMs since we have so little training data. The
global HMM is at Data/HMMPROTO/global.mmf. The global HMM will only be used to
determine the global variance floor in the vFloors MMF. The global mean,
variance, and the vFloors MMF are determined using the HTK HCompV tool.

mkdir Data/HMM0
HCompV -C UserDelta.cnf -f 0.01 -m -H Data/HMMPROTO/global.mmf -M Data/HMM0 \
       -S Data/HTK_files.list global

Having obtained a lower floor on the estimate of variances in vFloors, we
may now initialize the means and variances of each of the HMMs given the
segmented database file audio data using the HTK HInit tool. This creates a
separate MMF file for each phone HMM.

for hmm in sil eh ey k n ow s y;
 do HInit -C UserDelta.cnf -H Data/HMMPROTO/phones.mmf -H Data/HMM0/vFloors \
          -I Data/Segmented_phones.mlf -l $hmm -S Data/HTK_files.list \
          -u mv -M Data/HMM0 $hmm
    mv Data/HMM0/phones.mmf Data/HMM0/${hmm}.mmf
 done
 
Following the initialization, we can do an initial isolated phone Baum-Welch
re-estimation of the means and variances of HMM model parameters for each phone.

mkdir Data/HMM1
for hmm in sil eh ey k n ow s y; \
 do HRest -C UserDelta.cnf -H Data/HMM0/${hmm}.mmf -H Data/HMM0/vFloors \
          -I Data/Segmented_phones.mlf -l $hmm -S Data/HTK_files.list \
          -u mv -M Data/HMM1 $hmm
 done

We can now use the HTK HHEd tool to combine all phones into one combined MMF
file for ease in future processing. This requires a list of all phones, which we
obtain using the HLEd tool. 

HLEd -n Data/Phones.list -i /dev/null /dev/null Data/Phones.mlf

HHEd -H Data/HMM1/sil.mmf -H Data/HMM1/eh.mmf -H Data/HMM1/ey.mmf \
     -H Data/HMM1/k.mmf -H Data/HMM1/n.mmf -H Data/HMM1/ow.mmf \
     -H Data/HMM1/s.mmf -H Data/HMM1/y.mmf -H Data/HMM1/vFloors \
     -w Data/HMM1/phones.mmf /dev/null Data/Phones.list

Finally, we use the HTK HERest tool to train all parameters of the HMMs using
embedded training to enable cross-phone context to influence the HMM parameters.
Each time HERest is run, a single pass of training occurs, so it should be run
multiple passes. The number of passes must be determined by trial. Here
for illustration we use two passes. Three passes are often used. 

mkdir Data/HMM2
HERest -C UserDelta.cnf -H Data/HMM1/phones.mmf -I Data/Phones.mlf \
       -S Data/HTK_files.list -M Data/HMM2 Data/Phones.list
       
mkdir Data/HMM3
HERest -C UserDelta.cnf -H Data/HMM2/phones.mmf -I Data/Phones.mlf \
       -S Data/HTK_files.list -M Data/HMM3 Data/Phones.list
     
Step 4d: Training HMMs for separate sets (genders)
--------------------------------------------------
We now have the trained HMMs for all of the phones and the silence model. We
could stop here, but TIesr allows for multiple HMM sets based on certain common
characteristics. In this step we split the phone models into two separate sets
of phone models, one for females, and one for males. This has been shown to
often provide better recognition performance. Other sets are possible, such as
models for children, or models for various accents, etc.

We first need to create two sets of HMMs that are gender-dependent in a new MMF
file. We do this by using the HHEd command to duplicate the original phone HMMs
into two separate gender-dependent sets. However, since we have so little
training data, the HMM transition matrices and the variance vectors for each HMM
state will be shared between genders; that is, the transition matrices and
variances will be gender-independent. This is an illustration of being able to
tie HMM parameters so that they are shared between models. Further, we will use
a single gender-indpendent silence model, sil. The file make_fm.hed contains the
command DP to create two duplicate sets of the original HMMs, and TI commands to
tie the transition matrices corresponding to the gender-dependent phones and
variance vectors.

mkdir Data/HMM4
HHEd -H Data/HMM3/phones.mmf -w Data/HMM4/phones_fm.mmf make_fm.hed \
     Data/Phones.list

Next we need to modify the MLF file that contains the pronunciation of each
utterance. This is most easily done by splitting the MLF file Data/Phones.mlf
into two separate MLF files, one for females in Data/Fphones.mlf and one
for males in Data/Mphones.mlf, and then using the HLEd tool to mark the
phone labels, except the sil model, to make them gender-dependent:

HLEd  -l '*'  -i Data/Phones_f.mlf phones_f.led Data/Fphones.mlf 
HLEd  -l '*' -i Data/Phones_m.mlf phones_m.led Data/Mphones.mlf

Then make a list of all of the gender-dependent phones:

HLEd  -n Data/Phones_fm.list -i /dev/null /dev/null Data/Phones_f.mlf \
      Data/Phones_m.mlf

Now again use the HTK HERest tool to train all of the parameters of the
gender-dependent HMM sets. Several passes of training should be performed.
Notice that since the MLF files for female and male utterances specifically
require the gender-dependent models, we can run training for female and male
utterances in the same HERest training pass.

mkdir Data/HMM5
HERest -C UserDelta.cnf -H Data/HMM4/phones_fm.mmf \
       -I Data/Phones_f.mlf -I Data/Phones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM5 Data/Phones_fm.list
       
mkdir Data/HMM6
HERest -C UserDelta.cnf -H Data/HMM5/phones_fm.mmf \
       -I Data/Phones_f.mlf -I Data/Phones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM6 Data/Phones_fm.list

Step 4e: Training HMMs for triphone context
-------------------------------------------
We now have the trained gender-dependent HMM sets for all of the phones and the
common silence model. Again, we could stop here, but TIesr allows for phones
that are context dependent. For example, for word "yes" spoken by a
female, we could replace the gender-dependent phone eh:f with it's context
dependent phone y-eh:f+s, which indicates a phone eh:f in the context of a
leading y phone and a trailing s phone. There is an issue of what should be
done for context-dependent phone models at word boundaries. Here we will create
within-word context triphones, which do not cross word boundaries. The word
boundary is denoted by "sp" in the triphone name. So for example the
within-word triphone sequence for the female word "yes" is:

sp-y:f+eh   y-eh:f+s  eh-s:f+sp

We again start by creating the MLF files with triphone model context using the
HLEd tool to generate the gender-independent triphone HMM labels, and sed to
provide the gender-dependent tags. HLEd is also used to create a
gender-dependent triphone list.

HLEd -l '*' -i temp.mlf triphones.led Data/Fphones.mlf;
  cat temp.mlf | sed 's/\+/:f+/' >Data/Triphones_f.mlf; rm temp.mlf
  
HLEd -l '*' -i temp.mlf triphones.led Data/Mphones.mlf;
  cat temp.mlf | sed 's/\+/:m+/' >Data/Triphones_m.mlf; rm temp.mlf
 
HLEd -n Data/Triphones_fm.list -i /dev/null /dev/null Data/Triphones_f.mlf \
     Data/Triphones_m.mlf


Next we create the context-dependent triphone HMM models using HHEd, by a
command that creates cloned versions of the gender-dependent monophone sets
with triphone names.

mkdir Data/HMM7
HHEd -H Data/HMM6/phones_fm.mmf -w Data/HMM7/triphones_fm.mmf \
     make_triphones_fm.hed Data/Phones_fm.list


Finally, we again train these triphones multiple passes with HERest.

mkdir Data/HMM8
HERest -C UserDelta.cnf -H Data/HMM7/triphones_fm.mmf \
       -I Data/Triphones_f.mlf -I Data/Triphones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM8 Data/Triphones_fm.list
       
mkdir Data/HMM9
HERest -C UserDelta.cnf -H Data/HMM8/triphones_fm.mmf \
       -I Data/Triphones_f.mlf -I Data/Triphones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM9 Data/Triphones_fm.list

mkdir Data/HMM10
HERest -C UserDelta.cnf -H Data/HMM9/triphones_fm.mmf \
       -I Data/Triphones_f.mlf -I Data/Triphones_m.mlf \
       -s Data/HMM9/hmm9.stats \
       -S Data/HTK_files.list -M Data/HMM10 Data/Triphones_fm.list

Step 4f: Creating acoustic decision trees for triphones
-------------------------------------------------------
We have now trained the gender-dependent triphone models. However, once triphone
models are created, there is a problem with unseen triphones. Using the example
above in Step 3, Building the Compressed Binary Dictionary Files, the new word
"kes" is introduced. The trained triphone HMMs do contain the triphone HMMs
necessary to model this new word. For example, the triphones needed for the word
"kes" for females are:

sp-k:f+eh  k-eh:f+s  eh-s:f+sp

and these triphones are not in the trained data. In order to develop these
models from the trained triphones we need to find a way to map the existing
model parameters to the unseen triphones. This is the purpose of the acoustic
decision trees. To create the acoustic decision trees, we first must define a
set of questions that can be used to categorize the trained triphones and
cluster the information. Consult the HTK Book for detailed information on
creating lists of questions and using HHEd to cluster the triphones. 

This example is only illustrative of the steps necessary to create the acoustic
decision cluster trees. Since this example has so few words and training
utterances, very little can be accomplished by clustering. The only
gender-dependent phones that have more than one triphone context are ow:f and
ow:m (the contexts are: n-ow:f+sp and sp-ow:f+k for females, and n-ow:m+sp and
sp-ow:m+k for males). In this example, the questions and commands used for
clustering the trained triphones are found in make_cluster_triphone_fm.hed. The
questions and parameters used for clustering thresholds in this file should not
be considered as typical, since this is a limited example. Normally there would
be a large number of questions, and experimentation would need to be performed
to adjust the parameters for the level of clustering desired.

We use the HHEd tool to cluster the gender-dependent triphone state
distributions. Clustering requires a statistics file of state occupation from
prior HERest training. Notice that in the above training the last HERest pass
contained the -s option to generate the statistics file. During that HERest
training pass, the statistics file generated is for the input model set, not the
output model set, hence the statistics are for the HMMs found in the directory
Data/HMM9. When the HHEd tool performs clustering, it outputs a log file that
shows the results of clustering for each gender-dependent to the file
make_cluster_triphones_fm.log, and the file containing the trees is in
Data/Cluster_triphones_fm.trees. The commands to cluster the triphone states
are:

mkdir Data/HMM11
HHEd -H Data/HMM9/triphones_fm.mmf -w Data/HMM11/triphones_fm.mmf \
     make_cluster_triphones_fm.hed Data/Triphones_fm.list \
     > make_cluster_triphones_fm.log
    
Examination of the log file and the Data/Cluster_triphones_fm.trees file shows
that the third states of the triphones n-ow:f+sp and sp-ow:f+k are clustered
into a single shared state. The same is true for fourth states. The second
states are split into two different distributions by a decision tree. The
triphones n-ow:m+sp and sp-ow:m+k are clustered similarly.

After the clustering is performed, several passes of training should be run.

mkdir Data/HMM12
HERest -C UserDelta.cnf -H Data/HMM11/triphones_fm.mmf \
       -I Data/Triphones_f.mlf -I Data/Triphones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM12 Data/Triphones_fm.list

mkdir Data/HMM13
HERest -C UserDelta.cnf -H Data/HMM12/triphones_fm.mmf \
       -I Data/Triphones_f.mlf -I Data/Triphones_m.mlf \
       -S Data/HTK_files.list -M Data/HMM13 Data/Triphones_fm.list

Step 4g: Converting HTK data to TIesr-compatible acoustic data files
---------------------------------------------------------------------
We have now completed training of gender-dependent triphones and performed
clustering. This completes the HMM training. Now the trained HMM data needs to
be put in a compressed binary TIesr-compatible format, as described in the
TIesr documentation file Data_Training.txt.

We first compress the data in the MMF file, creating a set of files in the
Data/Lang/Models directory:

htk_mmf_to_tiesr.pl -H Data/HMM13/triphones_fm.mmf -g ':m,:f' -sb \
                    -le -d Data/Lang/Models


Then compress the decision tree data:

htk_trees_to_tiesr.pl -T Data/Cluster_triphones_fm.trees -C Data/Lang/Models \
                      -g ':m,:f' -le -d Data/Lang/Models


Step 5: Creating the Hierarchical Linear Regression phone cluster tree file
============================================================================
This step uses the results of monophone training to determine a linear
regression tree for the HMM monophone models.

We first use the htk_model_compare.pl tool to measure the similarity of
monophone models. It is important to list the monophones in the same order as
specified in the Data/Lang/Models/monophone.list and Data/Lang/Dict/phone.lis,
and with the silence phone added to the head of this list.

echo sil >workdir/temp.list; cat Data/Lang/Dict/phone.lis >>workdir/temp.list
htk_model_compare.pl -H Data/HMM3/phones.mmf  -l workdir/temp.list \
                      -s -o workdir/model_compare.txt -w workdir

Then use the cluster_models.pl tool to create the phone cluster tree file. This
file should be put in the same directory as the pronunciation decision trees.
 
cluster_models.pl -r workdir/model_compare.txt -o Data/Lang/hlrtree.bin -le
rm workdir/*


Step 6: Creating the Gaussian cluster files
===========================================
Since there are so few Gaussian vectors in this example, Gaussian clustering
will not provide significant speed-up. The TIesr recognizer does require the
clustering information, so this step is illustrative only. Gaussian clustering
requires the Gaussian mean vectors in 16-bit integer format, so if in a
real-world training byte means and variances were created by
htk_mmf_to_tiesr.pl, then it would have to be run again to generate 16-bit means
and variances in another temporary directory.

In this example there are only 47 mean vectors in the triphone models, so we
cluster these into 20 clusters for illustrative purposes. Real-world
full-language triphone modeling typically uses 128 clusters.

cd Data/Lang/Models
gaussiancluster . xtempmean.txt xtempvar.txt xinfo.txt 20 1
cd -


Step 7: Testing the data files
==============================
All data files needed for TIesr are now created in the Data/Lang directory and
its Dict and Model sub-directories. As mentioned above, some of the files in
the Data/Lang/Model directory are listing files for information purposes only,
and are not used by TIesr. The list of files in Data/Lang/Models needed by TIesr
is:

ROM2cls.bin
_qs.bin
_qstbl.bin
_tree.bin
_treetbl.bin
cls_centr.bin
fxclust.cb
fxclust.off
fxconfig.bin
fxgconst.cb
fxmean.cb
fxsil.hmm
fxtran.cb
fxtran.off
fxvar.cb
monophone.list

All other files may be removed.

It is possible to run a quick test of the TIesr recognizer using the original
audio files, and by voice. First, create a file that defines a simple grammar to
recognize single instances of the words KES, NO, OKAY, and YES. The grammar file
is at Test.grm.

Use the testtiesrflex program to generate the acoustic model and grammar
network files for the test grammar in the workdir directory:

testtiesrflex Test.grm workdir  Data Lang 2 0 1 1 0 0 0

Now, run the testtiesrsi program to recognize each of the original audio files
used for creating the data files:

for file in Data/*.raw
  do 
  echo ' '
  echo $file
  testtiesrsi 200000 workdir workdir/test.jac Data/Lang/hlrtree.bin $file
done

If you wish, and  you have an active audio device connected to your development
platform, you can run a live test. For a Linux development platform with ALSA
support, run the testtiesrsi program as:

testtiesrsi 200000 workdir workdir/test.jac Data/Lang/hlrtree.bin plughw:0,0

Other platforms should substitute the proper audio channel device. For Windows
platforms, you can just use some arbitrary non-file name.
