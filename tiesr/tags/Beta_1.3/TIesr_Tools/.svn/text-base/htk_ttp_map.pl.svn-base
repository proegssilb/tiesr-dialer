#!/usr/bin/perl
#-------------------------------------------------------------

 #
 # htk_ttp_map.pl
 #
 # Create dictionary of text to phone mapping.
 #
 # Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 #
 # This program is free software; you can redistribute it and/or modify 
 # it under the terms of the GNU Lesser General Public License as
 # published by the Free Software Foundation version 2.1 of the License.
 #
 # This program is distributed "as is" WITHOUT ANY WARRANTY of any kind,
 # whether express or implied; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Lesser General Public License for more details.
 #
 
#-------------------------------------------------------------


$USAGE = "
htk_ttp_map [-C config] -d dictionary -l letterhmms -w workingdir
             -n numnophone -t trainingpasses -o outputdict";


$DOCUMENTATION = "
This script creates a dictionary where each letter of each word is aligned
one-to-one with a phone, a pseudo-phone such as k_s indicating the k s phone
sequence, or the '_' symbol indicating null output corresponding to the
letter. The script uses the HTK training tool HERest and recognition tool
HVite to create a maximum likelihood estimate of the best letter-to-phone
alignment. The input dictionary specified by the -d option must be in the form
of a simple HTK dictionary with lines of the form 'word [pron] phone
phone...', with '[pron]' optional. The user must specify a file using the -l
option that encodes the structure of a set of letter HMMs that define the
possible phonetic transcriptions corresponding to a letter. The format of this
file is one letter per line.  The line starts with the letter followed by ':'
and then sequences of phones followed by probabilities and separated by
commas.  The special phone '_sil' indicates the probability that the letter is
silent (has no phone output).  For example the line 'X: g s .2, z .3, k s .2,
_sil .3' means that the letter 'X' in a word may generate the phone string 'g
s' with a probability 0.2, the phone 'z' with probability 0.3, the string 'k
s' with probability 0.2, or no phone at all with probability 0.3.  These
probabilities must add to 1.0.  This script will create the HTK MMF file from
this input.  The -w option is used to specify a working directory. The -n
option specifies the maximum number of sequential letters that may be silent
in a word. The -t option specifies the number of HTK training passes. The -o
option specifies name of the output dictionary file, and the optional -C
option can be used to specify configuration parameters for the HTK tools.";

#----------------------------------------------------------------
use Getopt::Long;

GetOptions( "Help" => \$help,
	    "d=s" => \$dictionary,
	    "l=s" => \$letterhmms,
	    "w=s" => \$wkdir,
	    "n=i" => \$numnophone,
	    "t=i" => \$numtrain,
	    "o=s" => \$outputdict,
	    "C=s" => \$config );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}


# Check for reasonable input
if( ! defined $numnophone ||
    ! defined $numtrain || 
    ! defined $outputdict ||
    ! -f $dictionary ||
    ! -f $letterhmms )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    die "htk_ttp_map: Invalid input\n";
}

# Create working directory if it does not exist
if( ! -d $wkdir )
{
    mkdir $wkdir, 0770 or die "htk_ttp_map: Can not make $wkdir\n";
}



# Create the dictionary mlf file, letter list, phone list, and HTK data files
print "Making HTK word mlf and data files, and support files\n";
if( ! -d "${wkdir}/htkdata" )
{
    mkdir "${wkdir}/htkdata", 0770 or die "htk_ttp_map: Can not make data directory\n";
}

$numnophone++;
$cmd = "htk_dict_to_mlf.pl -d $dictionary -m ${wkdir}/words.mlf -l ${wkdir}/words.let"
    . " -p ${wkdir}/words.phn -v ${wkdir}/htkdata -t $numnophone";

system( $cmd ) == 0 or die "htk_ttp_map: Could not create mlf, letter, phone, and/or data files\n";


# Dummy letter dictionary
open( LET, "${wkdir}/words.let" ) or die "htk_ttp_map: Can not open words.let\n";
@letters = <LET>;
close(LET);
open( LDICT, ">${wkdir}/letters.dict" ) or die "htk_ttp_map: Can not open letters.dict\n";
foreach $letter (sort @letters)
{
    chomp $letter;
    print LDICT "$letter $letter\n";
}
close(LDICT);


# Script file of all HTK data files to train and recognize
open( SCP, ">${wkdir}/words.scp" ) or die "htk_ttp_map: Can not open words.scp\n";
opendir( DIR, "${wkdir}/htkdata" ) or die "htk_ttp_map: Can not open htkdata\n";
while( $file = readdir DIR )
{
    next if $file =~  /^\.\.?$/;
    chomp $file;
    print SCP "\"${wkdir}/htkdata/$file\"\n";
}
close(SCP);
closedir(DIR);


# Create the hmm models for each letter
print "Creating HTK letter HMM models\n";
if( ! -d "${wkdir}/hmm0" )
{
    mkdir "${wkdir}/hmm0", 0770 or die "htk_ttp_map: Can not make hmm directory\n";
}
$cmd = "htk_letter_mmf.pl -p ${wkdir}/words.phn -h ${letterhmms}"
    . " -o ${wkdir}/hmm0/letters.mmf -m ${wkdir}/letters.map";

system( $cmd ) == 0 or die "htk_ttp_map: Could not create letter hmms and map\n";


# Run multiple passes of training
for( $pass = 1; $pass <= $numtrain; $pass++ )
{
    print "Running HERest pass $pass\n";
    $hmmdir = "${wkdir}/hmm${pass}";

    if( ! -d $hmmdir )
    {
	mkdir $hmmdir, 0770 or die "htk_ttp_map: Can not make data directory\n";
    }
    
    $lastpass = $pass - 1;
    $lasthmmdir="${wkdir}/hmm${lastpass}";


    $cmd = "HERest -C $config -T 1 -u t -m 1 -s ${wkdir}/letters${pass}.stats"
	. " -I ${wkdir}/words.mlf " 
	. " -H ${lasthmmdir}/letters.mmf -M $hmmdir" 
	. " -S ${wkdir}/words.scp  ${wkdir}/words.let"
	. " 1> ${wkdir}/herest.log 2> ${wkdir}/herest.err";

    system( $cmd ) == 0 or die "htk_ttp_map: Failed HERest pass\n";
}


# Do the recognition to determine letter to phone mapping
print "Running HVite word letter to phone alignment\n";
$cmd = "HVite -C $config -a -f -i ${wkdir}/words.reco -l '*' -m -o TW"
    . " -H ${hmmdir}/letters.mmf"
    . " -I ${wkdir}/words.mlf -S ${wkdir}/words.scp"
    . " ${wkdir}/letters.dict ${wkdir}/words.let"
    . " 1>${wkdir}/hvite.log 2>${wkdir}/hvite.err";

system( $cmd ) == 0 or die "htk_ttp_map: Failed HVite\n";


# Write out the final letter to phone dictionary
print "Parsing HTK HVite alignment into output letter to phone dictionary\n";
$cmd = "htk_ttp_parse.pl -r ${wkdir}/words.reco -m ${wkdir}/letters.map -o $outputdict";
system( $cmd ) == 0 or die "htk_ttp_map: Failed to parse recognition output\n";


print "Processing complete\n";
