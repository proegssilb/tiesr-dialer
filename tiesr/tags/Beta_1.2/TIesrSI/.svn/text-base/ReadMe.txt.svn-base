Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
ALL RIGHTS RESERVED

----------------


The TIesrSI API is the top-level API that an application designer uses to
perform speech recognition. It requires that the TIesrFlex API has already been
used to create a binary grammar network and acoustic model file set. The TIesrSI
API uses the TIesrEnginecore, TIesrEngineSI, and TIesrFA APIs.

Using the testtiesrsi Program
-----------------------------

The testtiesrsi program exercises the TIesrSI API. The source of the testtiesrsi
program provides an illustration of how the TIesrSI API can be used to recognize
speech. The detailed description of the user API functions supplied by the
TIesrSI API is found in the TIesrSI_User.h header file. THe command line for the
program is:

testtiesrsi mem_size grm_dir jac_file sbc_file aud_dev capture_file

mem_size: Working memory size in 2-byte shorts for the recognizer search engine
grm_dir:  Directory holding binary grammar and model file set from TIesrFlex
jac_file: Recognizer state file to use. Will be created if it does not exist.
sbc_file: A binary phone tree file. (Supplied with model data)
aud_dev:  Audio device, or file, to be used as the audio source

An example of running the testtiesrsi program from the top-level distribution
directory on a Linux OS is shown below. This program recognizes the grammar and
model file set created using TIesrFlex. The default example supplied with the
TIesr distribution recognizes the word "yes", or "no", or "maybe". Successful
use of the testtiesrsi program requires that you provide speech and say one of
the three words. To use testtiesrsi, start the program, wait for it to prompt
for you to speak, and then say one of the words "yes", or "no", or "maybe". 


Dist/LinuxReleaseGnu/bin/testtiesrsi \
    200000 \
    Data/GramDir \
    Data/testtiesrsi_linuxgnu.jac \
    Data/OffDT_GenDict_PhbVR_LE_MQ/English/hlrtree.bin \
    plughw:0,0 \
    Data/AudioDir/capture.raw


For example, if the word "maybe" is spoken after the "Speak now..." prompt, the
output will be:

Creating TIesrSI recognizer
Opening TIesrSI recognizer
Starting TIesrSI recognizer
Speak now...
Stopping TIesrSI recognizer
Recognized: maybe
Closing TIesrSI recognizer
Destroying TIesrSI recognizer
