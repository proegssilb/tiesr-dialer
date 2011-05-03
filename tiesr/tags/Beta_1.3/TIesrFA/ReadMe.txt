Document License

 This work is licensed under the Creative Commons Attribution-Share Alike 3.0
 United States License. To view a copy of this license, visit 
 http://creativecommons.org/licenses/by-sa/3.0/us/
 or send a letter to Creative Commons, 171 Second Street, Suite 300,
 San Francisco, California, 94105, USA.

 
Contributors to this document

 Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/


----------------


The TIesrFA API collects audio data and places the data into frames of a defined
number of samples for use by the TIesr speech recognizer tools.  The API
interface is completely specified by the TIesrFA_User.h file found in the src
directory.  The purpose of the creation of this interface is to provide a
consistent interface for use by all of the other TIesr speech recognizer tools.

Each hardware device and OS will likely have one or more unique APIs for
collecting audio input.  Since these methods will be OS and hardware dependent,
the TIesrFA API interface will have to be implemented for each device and OS. 
This will be the responsibility of the target OS/device application designer.

Using the testtiesrfa Program
-----------------------------

The testtiesrfa program exercises the TIesrFA API. The command line for the
program is:

testtiesrfa sample_rate frm_samples num_frames num_circ num_audio read_rate
            device out_file

sample_rate: Sample rate in samples/second
frm_samples: Number of samples in a frame
num_frames:  Number of frames of samples to write to out_file
num_circ:    Number of frames held in a circular buffer of frames
num_audio:   Number of input audio buffer frames, see below
read_rate:   Device specific parameter, see below
device:      Name of audio device to interact with, see below
out_file:    Output raw audio file


testtiesrfa collects audio data and places it into a circular buffer of frames
that are then written to an output file. The number of samples in a frame is
specified by frm_samples, and the number of frames in the frame circular buffer
is given by num_circ.  The testtiesrfa program will output num_frames of frames
of audio data to out_file. 

The arguments num_audio, read_rate, and device will take on different meanings
depending on the platform audio interface, since these parameters specify how to
set up the driver parameters for the audio. In all cases, if the device argument
specifies the name of an existing file, then audio data is input from the file
instead of a real audio device. 

The current release of TIesr contains two implementations of TIesrFA and
testtiesrfa. The implementation of TIesrFA for Windows uses the Windows Waveform
Audio API to interface with Windows audio devices. In this case the num_audio
argument specifies the number of audio buffers that are queued to the audio
driver to collect data. The number of samples in an audio buffer is a multiple
of the frm_samples, and the multiple is specified by the read_rate argument.
Typically read_rate is a small integer. If the device argument is not the name
of an existing file, then it is ignored and TIesrFA uses the Waveform Audio API
to locate the audio driver to use.

The implementation of TIesrFA for Linux uses the ALSA library API. In this case,
the read_rate argument specifies the duration between reads of the audio driver
ring data buffer in microseconds. The num_audio argument specifies a multiplier
that sets the size of the ALSA ring buffer as a multiple of the number of
samples occurring in the read_rate duration. The device argument specifies the
ALSA pcm device to use, for example "plughw:0,0".

An example of running the testtiesrfa program from the top-level distribution
directory on a Linux OS is shown below. In this example, data is collected at
8000 samples/second, with a circular buffer frame size of 160 samples (20 msec).
200 frames, or four seconds of audio is recorded. The circular frame buffer can
hold 50 frames. The ALSA ring buffer will hold 10 times the read rate in
samples.  Since the read rate is 20000 microseconds (20msec), the ALSA ring
buffer will hold 200 msec of data. The ALSA plughw:0,0 device is used, and the
four seconds of speech is output to the file Data/AudioDir/test.raw.

Dist/LinuxReleaseGnu/bin/testtiesrfa \
      8000 \
      160 \
      200 \
      50 \
      10 \
      20000 \
      plughw:0,0 \
      Data/AudioDir/test.raw