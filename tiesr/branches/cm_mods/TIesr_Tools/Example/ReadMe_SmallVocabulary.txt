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

This document is a modification of the ReadMe.txt file in this directory, and
documents a reduced set of step-by-step instructions for creating the TIesr data
files for small vocabulary applications. These instructions bypass the need to
create the pronunciation decision tree files, since one can put all words to be
recognized in the pronunciation dictionary. The dictionary may express the
pronunciation of a word as a sequence of phoneme models, or as a single
word-specific model. Using word-specific models allows one to also skip the
steps of creating triphone models.

This directory contains files and sub-directories that illustrate the use of the
subset of TIesr tools to create the set of data files for use with the TIesr
speech recognizer for small vocabularies. The example uses some of the tools in
the TIesr_Tools directory, and follows most of the discussion provided in the
Documentation/TIesr_Data_Training.txt file, except that training of
pronunciation decision trees is not necessary. Since this is a tutorial example,
it is not intended to produce a data set that provides very good performance,
but rather a data set that will work with the TIesr recognizer and illustrates
the concepts of data file creation. Comments in this document will make mention
of areas where additional steps could be taken to refine performance for a
real-life task.

It is assumed that the external and TIesr tools mentioned in the
TIesr_Data_Training.txt document have been obtained and built, so that all tools
are available. Since pronunciation decision trees are not used, the c4.5
decision tree training tool is not necessary.

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
We construct a text file dictionary that explains the pronunciation of the
words. In this example, instead of the words being expressed in terms of
phonetic sounds, we choose to have each word represented by a word-specific
model. The dictionary is found in the file sv_words.dic and it consists of the
three entries:
 
NO  no
OKAY okay
YES  yes

Here we enter the words in upper case and the models in lower case for easy
distinction, although this is not necessary. Enter the words in the dictionary
alphabetically, since this is important for some of the tools to work correctly.
In this case, the dictionary defines a set of models (no, okay, and yes) for the
words.

Step 1b: Preparing the speech database
--------------------------------------
The next step prepares the speech database files for use in training HMMs for
the word models. For this example, the Data directory contains the raw
recordings of five females and five males saying each word. The files are stored
in the gzipped tar file Data/AUDIO.tar.gz. Unzip these files to the Data
directory. Each file has the .raw file extension. These files contain acoustic
data sampled at 8kHz. Each sample is a signed 16-bit number stored in
little-endian format. The file names indicate the gender, speaker number, and
word. It is a good idea to provide descriptive directory and file names for the
speech database directories and files so that one can search and create lists of
speech files easily.


Step 2: Making the Letter File
==============================
Instead of creating pronunciation decision trees, for small vocabularies the
only file necessary is one that contains a sorted list of all characters making
up words in the dictionary. This must be put in a file named cAttValue.txt,
and we put it in Data/SV_Lang/cAttValue.txt. Each character should be a single
byte.


Step 3: Building the Compressed Binary Dictionary Files
=======================================================
We must convert the dictionary file into a binary form for subsequent processing
steps, since the TIesr tools use a binary dictionary.  The two files created
must be put in the same directory, and the files must be named "phone.lis" and
"dict.bin".

First we create a list of models in the dictionary in alphabetical order. For
small vocabularies this can easily be done by hand, but it can also be done
using the HTK HDMan tool. The list is put in the file SV_Lang/Dict/phone.lis.
While the models are not really phone models, TIesr will treat the models as if
they were single phones.

mkdir Data/SV_Lang/Dict
HDMan -n temp.list /dev/null sv_words.dic
sort temp.list >Data/SV_Lang/Dict/phone.lis; rm temp.list

Finally, we create the binary dictionary using the dictproc tool. The last
integer option on the dictproc tool command line is sets a flag to bypass using
pronunciation decision trees while creating the pronunciation dictionary.

dictproc sv_words.dic Data/SV_Lang/Dict/phone.lis Data/SV_Lang/Dict/dict.bin \
         little_endian 0 Data/SV_Lang 1

This creates the Data/SV_Lang/Dict/dict.bin binary dictionary.

If you wish, you can test the new dictionary just created using the
testtiesrdict program. The last integer option on the testtiesrdict command line
is a flag telling the TIesr APIs that the binary dictionary has been created
without using pronunciation decision trees. Testtiesrdict can be run in an
interactive mode, so you can try any number of words. When you enter a
dictionary word, the pronunciation is output if it exists. There is never a rule
output because we are not using decision trees.  For example if we interactively
run testiesrdict as follows, and input the words "YES" and "KNOW" we obtain:

testtiesrdict . Data/SV_Lang Dict 0 1 2

Number of phones used: 3
no
okay
yes

Enter Word: YES
YES has 1 entries in dictionary
Rule:  YES --->
Dict(1) for YES:  yes
Used now: yes

Enter Word: KNOW
KNOW has 0 entries in dictionary
Rule:  KNOW --->
Dict & Rule: No pronunciation available


From the output we see that the word "YES" is in the dictionary and 
uses the model "yes", and the word "KNOW" is not in the dictionary.
We also see that no rules are output for the words since we are not
using pronunciation decision trees.


Step 4: Building the Acoustic Model Data Files
==============================================
This step is the most involved and has a large number of variations, since it
requires training HMMs for each of the models using the HTK tools. Much research
is still being done on better ways to train HMMs for models. In this example we
present the steps needed to create a basic set of HMMs for TIesr and and comment
where some of the possible variations could be made.

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
Data/Words.mlf. This file defines the words spoken in each utterance
file.

Next we create a new MLF file containing the word-based models spoken in each
file. HTK provides the HLEd tool that can generate the MLF file. At the same
time, we add the "sil" model at the beginning and end of each utterance to
account for leading and trailing silence in the utterances. We create a label
editing command file in phones.led that contains the following:

EX
IS sil sil

Here EX is the command to expand from words to models according to the
pronunciation dictionary, and the IS command puts the "sil" model at the
beginning and end of each utterance. In a large vocabulary task or a task in
which multiple words are in an utterance, an additional model, or the sil model
must be included between words to model possible inter-word silence during
training. HTK documentation calls the additional model the "sp" phone model.
Consult the HTK documentation for methods of modeling inter-word silence.

Run HLEd as follows:

HLEd -d sv_words.dic -l '*' -i Data/SV_Models.mlf phones.led Data/Words.mlf


Step 4c: HTK training - Constructing initial HMMs for each phone
----------------------------------------------------------------
HTK training tools provide several different methods to create the initial HMMs.
They can be initialized without any detailed acoustic information, or they can
be initialized with acoustic data provided in training files. One can also
create label files corresponding to each training file that specify the time
segments in each training file containing each word or silence. For our example,
we create a master label file specifying the segments of each training file
containing words and silence in the file Data/SV_Segmented_models.mlf. Creating
the label files yields better initialization, but the process of creating the
segmented label files can be laborious. HTK also provides the option to use
training files that contain only a single word or silence instance, which will
avoid the need for segmenting the data and will be much faster. Since our
training data has no files that represent only silence, we use the segmented
label file. 

We create a simple HTK configuration file, UserDelta.cnf, so that the delta
feature parameters will be dynamically calculated from the features in the
Data/*.htk database files.

We create an HTK Master Model File (MMF) containing prototype HMMs for each of
the word-specific models and the sil model in Data/HMMPROTO/sv_models.mmf.
The actual values of the HMM state distribution means and variances are not
important, just the vector size and number of means and variances, since
training will update these. In contrast to phonetic model HMMs which usually
each have a small fixed number of emitting states, the number of states in
word-based models should reflect the duration and number of phones in the word. 

In addition we create a second MMF containing a single global prototype HMM for
the purpose of determining the global mean and variance of all of the training
data. This is used as a means to create a vFloors MMF to limit the variance
estimate of all of the HMMs since we have so little training data. The
global HMM is at Data/HMMPROTO/global.mmf. The global HMM will only be used to
determine the global variance floor in the vFloors MMF. The global mean,
variance, and the vFloors MMF are determined using the HTK HCompV tool.

mkdir Data/HMM0
HCompV -C UserDelta.cnf -f 0.01 -m -H Data/HMMPROTO/global.mmf -M Data/HMM0 \
       -S Data/HTK_files.list global

Having obtained a lower floor on the estimate of variances in vFloors, we
may now initialize the means and variances of each of the HMMs using the HTK
HInit tool. This creates a separate MMF file for each word-based HMM model
using the segment information for each training file.

for hmm in sil no okay yes;
 do HInit -C UserDelta.cnf -H Data/HMMPROTO/sv_models.mmf -H Data/HMM0/vFloors \
          -I Data/SV_Segmented_models.mlf -l $hmm -S Data/HTK_files.list \
          -u mv -M Data/HMM0 $hmm
    mv Data/HMM0/sv_models.mmf Data/HMM0/${hmm}.mmf
 done
 
Following the initialization, we can do an initial isolated word Baum-Welch
re-estimation of the means and variances of HMM model parameters for each word.

mkdir Data/HMM1
for hmm in sil no okay yes; \
 do HRest -C UserDelta.cnf -H Data/HMM0/${hmm}.mmf -H Data/HMM0/vFloors \
          -I Data/SV_Segmented_models.mlf -l $hmm -S Data/HTK_files.list \
          -u mv -M Data/HMM1 $hmm
 done

We can now use the HTK HHEd tool to combine all models into one combined MMF
file for ease in future processing. This requires a list of all word models
which we obtain using the HLEd tool. 

HLEd -n Data/SV_Models.list -i /dev/null /dev/null Data/SV_Models.mlf

Now we combine all of the models together in a single file.

HHEd -H Data/HMM1/sil.mmf -H Data/HMM1/no.mmf -H Data/HMM1/okay.mmf \
     -H Data/HMM1/yes.mmf -H Data/HMM1/vFloors \
     -w Data/HMM1/sv_models.mmf /dev/null Data/SV_Models.list

Now we can use the HTK HERest tool multiple times to train all parameters of the
HMMs for each word and the silence model. Each time HERest is run, a single pass
of training occurs, so it should be run multiple passes. The number of passes
must be determined by trial. The number of passes will depend on the type of
model initialization and training to this point. Here for illustration we use
two passes.

mkdir Data/HMM2
HERest -C UserDelta.cnf -H Data/HMM1/sv_models.mmf -I Data/SV_Models.mlf \
       -S Data/HTK_files.list -M Data/HMM2 Data/SV_Models.list
       
mkdir Data/HMM3
HERest -C UserDelta.cnf -H Data/HMM2/sv_models.mmf -I Data/SV_Models.mlf \
       -S Data/HTK_files.list -M Data/HMM3 Data/SV_Models.list
     
Step 4d: Training HMMs for separate sets (genders)
--------------------------------------------------
We now have the trained HMMs for all of the word models and the silence model.
We could stop here, but TIesr allows for multiple HMM sets based on certain
common characteristics. In this step we split the word models into two separate
sets of word models, one for females, and one for males. This has been shown to
often provide better recognition performance. Other sets are possible, such as
models for children, or models for various accents, etc.

We first need to create two sets of HMMs that are gender-dependent in a new MMF
file. We do this by using the HHEd command to duplicate the original word HMMs
into two separate gender-dependent sets. However, since we have so little
training data, the HMM transition matrices and the variance vectors for each HMM
state will be shared between genders; that is, the transition matrices and
variances will be gender-independent. This is an illustration of being able to
tie HMM parameters so that they are shared between models. Further, we will use
a single gender-indpendent silence model, sil. The file sv_make_fm.hed contains
the command DP to create two duplicate sets of the original HMMs, and TI
commands to tie the transition matrices corresponding to the gender-dependent
phones and variance vectors.

mkdir Data/HMM4
HHEd -H Data/HMM3/sv_models.mmf -w Data/HMM4/sv_models_fm.mmf sv_make_fm.hed \
     Data/SV_Models.list

Next we need to modify the MLF file that contains the pronunciation of each
utterance. This is most easily done by splitting the MLF file Data/SV_Models.mlf
into two separate MLF files, one for females in Data/SV_FModels.mlf and one for
males in Data/SV_MModels.mlf, and then using the HLEd tool to mark the model
labels, except the sil model, to make them gender-dependent:

HLEd  -l '*'  -i Data/SV_Models_f.mlf sv_models_f.led Data/SV_FModels.mlf 
HLEd  -l '*' -i Data/SV_Models_m.mlf sv_models_m.led  Data/SV_MModels.mlf

Then make a list of all of the gender-dependent models:

HLEd  -n Data/SV_Models_fm.list -i /dev/null /dev/null Data/SV_Models_f.mlf \
      Data/SV_Models_m.mlf

Now again use the HTK HERest tool to train all of the parameters of the
gender-dependent HMM sets. Several passes of training should be performed. Since
the MLF files for female and male utterances specifically require the
gender-dependent models, we can run training for female and male utterances in
the same HERest training pass.

mkdir Data/HMM5
HERest -C UserDelta.cnf -H Data/HMM4/sv_models_fm.mmf \
       -I Data/SV_Models_f.mlf -I Data/SV_Models_m.mlf \
       -S Data/HTK_files.list -M Data/HMM5 Data/SV_Models_fm.list
       
mkdir Data/HMM6
HERest -C UserDelta.cnf -H Data/HMM5/sv_models_fm.mmf \
       -I Data/SV_Models_f.mlf -I Data/SV_Models_m.mlf \
       -S Data/HTK_files.list -M Data/HMM6 Data/SV_Models_fm.list

This completes training of the HMM models. Since we are creating word-based
models, we do not need to be concerned about training for triphone context, or
acoustic decision trees.


Step 4e: Converting HTK data to TIesr-compatible acoustic data files
---------------------------------------------------------------------
Now the trained HMM data needs to be put in a compressed binary TIesr-compatible
format, as described in the TIesr documentation file TIesr_Data_Training.txt.

We first compress the data in the MMF file, creating a set of files in the
Data/SV_Lang/Models directory using one of the TIesr_Tools. Here, since we are
not training triphone models, we tell the tool that we are not using acoustic
decision trees using the -notrees option.

htk_mmf_to_tiesr.pl -H Data/HMM6/sv_models_fm.mmf -g ':m,:f' -sb -le \
  -notrees -d Data/SV_Lang/Models


Step 5: Creating the Hierarchical Linear Regression cluster tree file
============================================================================
This step uses the results of word model training to determine a linear
regression tree for the HMM models. When using TIesr for small vocabulary
recognition with word-based models, it is possible to build TIesr so that it
does not use the linear regression trees. This is accomplished by modifying the
config.h file in the TIesrEngine* tools so that the SBC processing is not
enabled. This will result in lower memory and processing requirements. For
completeness, we cover the steps to create word-based regression tree here, so
that TIesr can be used with the SBC processing enabled.

We first use the htk_model_compare.pl tool to measure the similarity of word
models. It is important to list the gender-independent models in the same order
as specified in the Data/SV_Lang/Models/monophone.list and
Data/SV_Lang/Dict/phone.lis, and with the silence phone added to the head of
this list.

echo sil >workdir/temp.list; cat Data/SV_Lang/Dict/phone.lis >>workdir/temp.list
htk_model_compare.pl -H Data/HMM3/sv_models.mmf  -l workdir/temp.list \
                      -s -o workdir/model_compare.txt -w workdir

Then use the cluster_models.pl tool to create the phone cluster tree file. This
file should be put in the same drm workdir/*irectory as the pronunciation
decision trees.
 
cluster_models.pl -r workdir/model_compare.txt -o Data/SV_Lang/hlrtree.bin -le
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

In this example there are only 51 mean vectors in the word-based models, so we
cluster these into 20 clusters for illustrative purposes. Real-world
full-language triphone modeling typically uses 128 clusters.

cd Data/SV_Lang/Models
gaussiancluster . xtempmean.txt xtempvar.txt xinfo.txt 20 1
cd -


Step 7: Testing the data files
==============================
All data files needed for TIesr are now created in the Data/SV_Lang directory
and its Dict and Model sub-directories. As mentioned above, some of the files in
the Data/SV_Lang/Model directory are listing files for information purposes
only, and are not used by TIesr. The list of files in Data/SV_Lang/Models needed
by TIesr for this small-vocabulary example is:

ROM2cls.bin
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
recognize single instances of the words NO, OKAY, and YES. The grammar file
is at sv_test.grm.

Use the testtiesrflex program to generate the acoustic model and grammar
network files for the test grammar in the workdir directory, with a flag
indicating that no pronunciation decision trees are to be used:

testtiesrflex sv_test.grm workdir  Data SV_Lang 2 0 1 1 0 0 2

Now, run the testtiesrsi program to recognize each of the original audio files
used for creating the data files:

for file in Data/*.raw
  do 
  echo ' '
  echo $file
  testtiesrsi 200000 workdir workdir/test.jac Data/SV_Lang/hlrtree.bin $file
done

If you wish, and you have an active audio device connected to your development
platform, you can run a live test. For a Linux development platform with ALSA
support, run the testtiesrsi program as:

testtiesrsi 200000 workdir workdir/test.jac Data/SV_Lang/hlrtree.bin plughw:0,0

Other platforms should substitute the proper audio channel device. For Windows
platforms, you can just use some arbitrary non-file name.
