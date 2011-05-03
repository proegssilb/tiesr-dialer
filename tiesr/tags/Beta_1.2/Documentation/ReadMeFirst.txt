Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
ALL RIGHTS RESERVED


Using This TIesr Distribution
-----------------------------
This TIesr distribution contains all of the APIs that make up the TIesr Speech
Recognizer. In addition, the distribution contains test programs that may be
used to test the APIs to ensure they are working correctly. Any speech
recognizer requires data for grammar and acoustic modeling, and this
distribution contains data for implementing general American English speech
recognition. A few tools are also supplied for diagnostics, or in case one just
wishes to explore operation of the recognizer in greater detail.  All of these
components are briefly described below.


TIesr APIs
----------

The TIesr speech recognizer consists of seven APIs in six directories. Normally
a user only needs to interact with the two top-level APIs, TIesrFlex and
TIesrSI. An API is located within a directory with a name indicating the API or
APIs contained. Within the API directory is a src directory containing all
source code. API directories also contain a resource directory which holds
versioning files for Linux builds. If an API includes a test program, then
documentation for that program is found within the API directory.

TIesr APIs are written in C and C++. The APIs have been built using the open
source NetBeans IDE (www.netbeans.org). Each API directory includes a NetBeans
project directory containing the NetBeans project to build the API.
Configurations exist in each project for building on a Windows platform with
cygwin (www.cygwin.com), and on a Linux platform. Also, configurations exist for
cross-compiling and building for an embedded ARM-based platform using the
Codesourcery G++ lite gnu toolchain (http://www.codesourcery.com/sgpp/lite/arm).


Preparation
-----------

If you wish to use the NetBeans IDE to perform the build, and do development of
the source, it must be downloaded to your development platform.  This can be
done from the NetBeans web site, www.netbeans.org.

If you wish to build and develop on a Windows platform using the NetBeans IDE,
then install cygwin on the Windows platform from the web site www.cygwin.com.
(cygwin should be installed at C:\cygwin). The cygwin download should include
the gnu development tools (gcc-core and gcc-g++, gdb) and support for Mingw32
(gcc-mingw-core and gcc-mingw-g++).

If you wish to cross-compile and build for the ARM-based platform, then you can
download the Codesourcery G++ Lite toolchain from
http://www.codesourcery.com/sgpp/lite/arm. You may load this toolchain anywhere,
but the TIesr distribution assumes that within the TIesr distribution's
Tools/ARM directory there is a link called "bin" to a directory containing the
Codesourcery gnu toolchain, and within the Tools/ARM directory there is a link
"include" to a directory containing any additional header files needed, and a
link "lib" to a directory containing any additional libraries needed for linking
APIs not included with the Codesourcery gnu toolchain (for example, the ALSA
sound library).

Once the tools are in place, NetBeans should be configured to use the tool sets
by selecting Tools->Options, selecting C/C++ and then the Build Tools tab.
Ensure that the locations of the tools are set for the Tool Collection to be
used for the project configurations of interest.

The NetBeans makefile for the TIesrFA_ALSAso API for configurations that build
for the ARM platform look for ALSA API header files in the Tools/ARM/include
directory and ALSA libraries in the Tools/ARM/lib directory. You must create the
"include" and "lib" directories and place the necessary files in these
directories or create the "include" and "lib" directories as links to the
directories where the headers and libraries reside.

During compilation or building in NetBeans, if you encounter a screen that says
the project was created for a different OS, this can normally be corrected by
right clicking on the project name in the Projects tab, selecting Properties,
and then clicking OK. This should regenerate the build files for the platform
you are using.


Quick Overview of TIesr Distribution Directories
------------------------------------------------

This section provides a quick overview of each of the directories in the TIesr
distribution.

Data
----
The Data directory contains data that the TIesr recognizer needs for building
grammars and acoustic models, and directories and files created by the TIesr
test programs. For further information, consult the ReadMe file in the Data
directory.

Dist
----
The Dist directory contains the TIesr API shared objects or DLL's when they are
built. It also contains the test programs when built. This would be a good place
to add build outputs for additional configurations for other platforms or
devices.

Documentation
-------------
This directory contains the top-level documentation and licensing information.

TIesrDict
---------
This directory contains the TIesrDict API. This API is responsible for looking
up the pronunciation(s) of a word in a dictionary. It is used by the TIesrFlex
API, and uses the TIesrDT API. It contains two NetBeans project directories;
TIesrDictso for building the API, and TestTIesrDict for building a test program
that exercises TIesrDict.

TIesrDT
-------
This directory contains the TIesrDT API. This API is responsible for creating an
initial hypothesis of the pronunciation of a word from the word spelling alone.
It is used by the TIesrDict API. It contains two NetBeans project directories;
TIesrDTso for building the API, and TestTIesrDT for building a test program that
exercises TIesrDT.

TIesrEngine
-----------
This directory contains the code that implements the actual speech recognizer
processing. There are two NetBeans project directories; TIesrEngineCoreso for
building the core speech recognition utilities, and TIesrEngineSIso for building
the speech recognizer adaptation and interface utilities.  TIesrEngineSIso uses
the TIesrEngineCoreso API, and TIesrEngineSIso is used by the top-level TIesrSI
API.

TIesrFA
-------
This directory contains the TIesrFA API. This API implements audio data
collection and buffering for the TIesr speech recognizer. You may have to
specifically implement this interface for a platform or OS, since it depends on
the particular audio interfaces. This API is multi-threaded, starting collection
in a separate high priority thread. Two implementation projects are supplied;
TIesrFAWinso is an implmementation for Window platforms that uses the Windows
Waveform Audio API, and TIesrFA_ALSAso is for Linux platforms using the ALSA
audio API. The directory also contains a TestTIesrFA project to build a test
program that exercises TIesrFA by collecting audio data.  Make sure the audio
data you collect is good quality before trying speech recognition.

TIesrFlex
---------
This directory contains the TIesrFlex API, which is the top-level API that a
user utilizes to prepare a grammar and acoustic model set for the TIesr
recognizer. The directory contains two NetBeans projects; TIesrFlexso for
building the TIesrFlex API, and TestTIesrFlex for building a test program to
exercise the TIesrFlex API by building a simple speech recognizer grammar and
model set that recognizes the words "yes", "no", or "maybe".

TIesrSI
-------
This directory contains the top-level API that a user utilizes to perform speech
recognition and obtain recognition results. The directory contains two projects;
TIesrSIso to build the TIesrSI API, and TestTIesrSI to build a test program that
tests TIesr by recognizing the words "yes", "no", or "maybe".

TIesr_Tools
-----------
This directory contains tools that can be used off-line to build data sets for
various languages and dictionaries, and for analyzing some of the data files
used by the TIesr recognizer. Many of the tools are perl scripts, and some are C
or C++ programs. The programs are not built as part of building and testing
TIesr as described below. The user must build these separately. See
documentation in the TIesr_Data_Training.txt file for further information.

Tools
-----
The user can place tools, or links to tools, for cross-compiling and building
the TIesr APIs for other platforms in this directory. An example is placing
links in the Tools/ARM directory to the Codesourcery GNU build toolchain for
ARM-based embedded platforms. This directory can also contain include
directories and libraries, or links to include directories and libraries, that
are needed in order to build TIesr for other platforms  For example, building
the TIesrFA API may require the ALSA include and library directories for the
platform of interest 

Building and Testing TIesr
--------------------------
Tools are provided to build the individual TIesr APIs. As the result of building
the APIs the libraries and executables are placed under the the directory named
"Dist".

Building the TIesr recognizer is best done by building the individual APIs and
testing them with the supplied test programs to ensure the APIs are operating
correctly. The release includes NetBeans IDE projects for each API and test
program. The NetBeans project files can be used to build and run the individual
APIs and test programs within the NetBeans IDE. For information regarding the
NetBeans IDE and obtaining it visit:

http://netbeans.org

During compilation or building in NetBeans, if you encounter a screen that says
the project was created for a different OS, this can normally be corrected by
right clicking on the project name in the Projects tab, selecting Properties,
and then clicking OK. This should regenerate the build files for the platform
you are using.

Alternatively, in the absence of NetBeans, the APIs and programs can be built by
using the make files supplied with the released NetBeans projects. In order to
automate this process, the top level of the release directory contains a single
Makefile. This Makefile can be used to clean or build any subset of the APIs and
test programs for any supported configuration. In order for this Makefile to
work, the Gnu compilation tools gcc, g++, and as (or for Cygwin on Windows
gcc.exe, g++.exe, as.exe) must be available and specified on the PATH such that
the tools can be executed from the command line using just the filename of the
tool. The format of the make command for the Makefile is:

make <CONFIGURATION> [PROJECTS='project ...'  STEP=<clean|build>]

The supported configurations are:
WindowsDebugMinGW
WindowsReleaseMinGW
LinuxDebugGnu
LinuxReleaseGnu
ArmLinuxDebugGnueabi
ArmLinuxReleaseGnueabi

If the variable PROJECTS is not given on the command line, then all projects for
the configuration will be built. Otherwise, only the projects specified by the
PROJECTS variable will be built. Valid project names are:

TIesrDTso
TestTIesrDT
TIesrDictso
TestTIesrDict
TIesrFlexso
TestTIesrFlex
TIesrFAWinso (only for Windows)
TIesrFA_ALSAso (only for Linux with ALSA support)

TestTIesrFA
TIesrEngineCoreso
TIesrEngineSIso
TIesrSIso
TestTIesrSI

For example, to build the TIesrDTso and TestTIesrDT projects for a Linux debug
build use:

make LinuxDebugGnu  PROJECTS='TIesrDTso TestTIesrDT'

The user may specify what action to take, build or clean, using the STEP
variable. The configuration "help" will display the latest information simi
lar to the above about the make file:
 
 make help


The remainder of this section describes the order for building and testing
the APIs. This should be read regardless of whether the Makefile or NetBeans is
used for building the APIs and test programs.

Three APIs make up the grammar network and model generation capability; TIesrDT,
TIesrDict, and TIesrFlex. First build the TIesrDT API, and then the TestTIesrDT
program. Run the TestTIesrDT program as described in the ReadMe.txt file in the
TIesrDT directory, and ensure the output is correct.

Build the TIesrDict API, and then the TestTIesrDict program. Run the
TestTIesrDict program as described in the ReadMe.txt file in the TIesrDict
directory, and ensure the output is correct.

Build the TIesrFlex API, and then the TestTIesrFlex program. Run the
TestTIesrFlex program as described in the ReadMe.txt file in the TIesrFlex
directory, and ensure the output is correct.

When these three APIs have been built and tested, then the grammar network and
model generation portion of TIesr are ready to be used.

Next, build the TIesrFA API for the OS you wish, either Windows, or Linux with
ALSA. If your platform does not match one of these, you will have to implement
your own version of the TIesrFA interface. The TIesrFA interface is fully
specified by the header file TIesrFA/src/TIesrFA_User.h. To write your own
implementation, you may wish to examine the either the Windows or Linux/ALSA
versions supplied and use those as a template for your implementation.
Once the TIesrFA API is built, then build the TestTIesrFA program. Run the
program as as described in the ReadMe.txt file in the TIesrFA directory. This
should capture audio data in a file. The data should be single channel 2-byte
signed integer PCM samples at 8kHz sampling rate. It is very important that the
audio data be examined carefully. Many of the problems encountered with TIesr
(or any speech recognizer) are traced to poor audio input. The speech collected
should have a peak absolute value >2000, and of course the signal should never
limit ( values of +32767 or-32768 ). You should play the captured signal using
good quality speakers, and listen for any distortion in the output that might
indicate a faulty audio channel or microphone.

After building the TIesrFA API, build the two recognizer TIesrEngine APIs,
TIesrEngineCore and TIesrEngineSI. These two APIs do not have a test program
written for them.

Build the TIesrSI API, and then the TestTIesrSI program. The TestTIesrSI program
exercises the TIesr speech recognizer, and uses the TIesrEngineCore,
TIesrEngineSI, and TIesrFA APIs. The TestTIesrSI program performs speech
recognition of one of the words "yes", "no" or "maybe". The TestTIesrSI program
may be run in a live mode where you supply audio from a microphone, or it may
use a file as input. See the ReadMe.txt file in the TIesrSI directory for more
information. When this program runs successfully, you have built and tested the
TIesr speech recognizer. You can use the TestTIesrSI source code as an example
of how to use the TIesr APIs in an application.
