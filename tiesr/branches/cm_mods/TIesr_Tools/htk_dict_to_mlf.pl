#!/usr/bin/perl
#-------------------------------------------------------------

 #
 # htk_dict_to_mlf.pl
 #
 # Create word spelling MLF file from HTK dictionary.
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
htk_dict_to_mlf -d dictfile -m mlffile [-l letterfile -p phonefile -v vqdir -t
teeskips]";


$DOCUMENTATION = "
This script creates a word spelling MLF file from a simple HTK
dictionary for use in letter-to-phone dictionary training. The
input dictionary is of the form:

 word [output] phn1 phn2 ...

 It outputs an MLF file with entries of the form:

#!MLF!#
*/WORD_i
_NUL
W
_NUL
O
_NUL
R
_NUL
D
_NUL
# .

Here the '_i' represents underscore followed by an integer,
representing the possible alternate pronunciations of a word.  Note
the capitalization.  We assume letters of a word will be represented
in capitals, and phones will be represented as lower case strings.
The idea here is that each letter will be represented by a discrete
HMM. Note that there are lines with _NUL in them.  This refers to a
special model _NUL which is a generic non-tee model.  Since each
letter HMM may be a tee model, and HTK does not allow successive tee
models, we must provide non-tee models inbetween each letter HMM.
Also, in order to output the training phone index data files, you must
specify the maximum number of letters that can be silent in a word in
order to put the skip phones in the data files to implement skipping
of letters in a spelling.

This script can also output a listing of letters and phones
it found in the dictionary.  The special non-tee model NUL and
its corresponding phone state 'nul' are also output.

This script can also output HTK style data files that contain vq 
indices for each word in the training dictionary.  This creates the
training and testing data for doing the letter- to-phone alignment
training. This utility does filtering on the dictionary ensuring that
there are not multiple duplicate pronunciations of the same word.";

#----------------------------------------------------------------
use Getopt::Long;


GetOptions( "Help" => \$help,
	    "d=s" => \$dictionary,
	    "m=s" => \$mlffile,
	    "l:s" => \$letterfile,
	    "p:s" => \$phonefile,
	    "v:s" => \$vqdirectory,
	    "t:i" => \$teeskips
	    );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}


if( !($dictionary && $mlffile) or $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    die "htk_dict_to_mlf: Invalid input\n";
}


die "htk_dict_to_mlf: Invalid dictionary $dictionary\n" if( ! -f $dictionary );

$teeskips = 0 unless defined $teeskips;

open( DICT, "$dictionary" ) or die "htk_dict_to_mlf: Can not open $dictionary\n";

open(MLF, ">$mlffile" ) or die "htk_dict_to_mlf: Can not open $mlffile for output\n";
print MLF "#!MLF!#\n";

# Include the non-tee _NUL HMM model in letter hash,
# and the _nul phone in the phone hash.
$lhash{"_NUL"} = "_NUL";
$phash{"_nul"} = "_nul";

DICTLOOP: while( <DICT> )
{
    chomp;

    # do not process comments and blank lines
    next if( /^\s*$|^\s*\#/ );

    # parse a dictionary entry
    @parse =  /^\s*([^\s]+)(?:\s+\[.*\])?\s+(.*?)\s*$/;

    # Extract the word and pronunciation
    $word = shift @parse;
    $word = uc $word;
    $word =~ s/\s//g;
    $pron = shift  @parse;
    $pron = lc $pron;
    $pron =~ s/\s+/ /g;


    # Update hash that tracks number of dictionary entries of this word,
    # and the different pronunciations
    if( exists $whash{$word} )
    {
	foreach $wpron ( @{$whash{$word}{PRON}} )
	{
	    next DICTLOOP if $wpron eq $pron;
	}
	$whash{$word}{COUNT}++;
	push( @{$whash{$word}{PRON}}, $pron );
    }
    else
    {
	$whash{$word}{COUNT} = 1;
	push( @{$whash{$word}{PRON}}, $pron );
    }

    # Update letter hash list
    @letters = split( //, $word );
    @lhash{@letters} = @letters;

    # Update the phone hash list
    @phones = split( " ", $pron );
    @phash{@phones} = @phones;


    # Output the MLF entry for this word. Since the  HTK word 
    # can not have "." in the name, substitute "p" for "."
    $htkword = $word;
    $htkword =~ s/\./p/g;
    print MLF "\"*/${htkword}_$whash{$word}{COUNT}.lab\"\n";
    foreach $let (@letters)
    {
	print MLF "\"_NUL\"\n\"$let\"\n";
    }
    print MLF "\"_NUL\"\n.\n";
}

close(MLF);
close(DICT);

# Output letter list
if( $letterfile )
{
    open(LET, ">$letterfile") or die "htk_dict_to_mlf: Can not open $letterfile\n";
    foreach $let (sort keys(%lhash) )
    {
	print LET "\"$let\"\n";
    }
    close(LET);
}

# Output phone list
if( $phonefile )
{
    open(PHONE, ">$phonefile") or die "htk_dict_to_mlf: Can not open $phonefile\n";

    foreach $phone (sort keys(%phash) )
    {
	print PHONE "$phone\n";
    }
    close(PHONE);
}


exit 0  if( !defined $vqdirectory || ! -d $vqdirectory );

# Make a hash that contains the indices for each phone
@pindex{ sort keys %phash} = ( 1 .. scalar(keys(%phash)) );


# Make all of the phone-based training/recognition HTK 
# data files.
chdir $vqdirectory;

$ntphoneidx = $pindex{"_nul"};

foreach $wrd ( sort keys(%whash) )
{
    $npron = $whash{$wrd}{COUNT};

    for( $pron=1; $pron<=$npron; $pron++ )
    {
	$fname = "${wrd}_$pron";
	@phones = split(" ", $whash{$wrd}{PRON}[$pron-1] );
	$nphones = @phones;

	# HTK file name can not contain ".", substitute "p"
	$htkfname = $fname;
	$htkfname =~ s/\./p/g;

	#Open HTK data file and write header (discrete data)
	open( DATA, ">$htkfname" );
	print DATA pack "IISS", $nphones + ($nphones+1)*$teeskips, 1, 2, 10;

	# Output all phone indices
	foreach $phn ( @phones )
	{
	    for( $tee = 0; $tee < $teeskips; $tee++ )
	    {
		print DATA pack "S",  $ntphoneidx;
	    }

	    print DATA pack "S",  $pindex{$phn};
	}

	for( $tee = 0; $tee < $teeskips; $tee++ )
	{
	    print DATA pack "S",  $ntphoneidx;
	}

	close(DATA);
    }
}
