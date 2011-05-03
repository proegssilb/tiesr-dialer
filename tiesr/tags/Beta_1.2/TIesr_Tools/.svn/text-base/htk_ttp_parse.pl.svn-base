#!/usr/bin/perl
#----------------------------------------------------------------
#  htk_ttp_parse.pl

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


$USAGE = "
htk_ttp_parse -r recofile -m modelmapfile -o outputdict";


$DOCUMENTATION = "
This script processes the output of the HVite recognition to determine
the mapping of letters to phones and pseudo-phones.  The -r option
specifies the recognition output file. The -m option specifies the
state to phone mapping file which allows each state recognized to be
mapped to a phone. The -o option specifies the output letter-to-phone
dictionary.";

#----------------------------------------------------------------

use Getopt::Long;


GetOptions( "Help" => \$help,
	    "r=s" => \$recofile,
	    "m=s" => \$modelmap,
	    "o=s" => \$outputfile
	    );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}


if( ! -f $modelmap ||
    ! -f $recofile ||
    ! $outputfile )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    die "htk_ttp_parse: Invalid input\n";
}


# Read in the model map file which gives the name of the phone
# corresponding to each emitting state.  Create hash of state
# arrays.
open( MMAP, "$modelmap" ) or die "htk_ttp_parse: Could not open $modelmap\n";

while( <MMAP> )
{
    @mdlarr = split(" ");
    $mname = shift @mdlarr;
    $mhash{$mname} = [ @mdlarr ];
}

close( MMAP );

# Open the output file and the HVite recognition file
open( OUT, ">$outputfile" ) or die "htk_ttp_parse: Could not open $outputfile\n";
open( REC, "$recofile" ) or die "htk_ttp_parse: Could not open $recofile\n";


# Parse reco file
while( <REC> )
{

    # Skip first line 
    next if /^\#!MLF!\#$/;


    # If word line, extract word
    if( /^\"(?:.*\/)?([^\/]+)_\d+\.rec/ )
    {
	$word = $1;

	parse_reco( *REC, *OUT, $word, \%mhash );
    }

    else
    {
	die "htk_ttp_parse: Unknown line: $_\n";
    }
}

close( REC );
close( OUT );


sub parse_reco
{
    local( *REC ) = shift;
    local( *OUT ) = shift;
    my( $htkword ) = shift;
    my( $rmhash ) = shift;

    # Convert HTK word back to original word
    my $word = $htkword;
    $word =~ s/p/\./g;

    # Output the word to the dictionary
    printf OUT "%-20s", "\"${word}\"";

    chomp($word);

    @chars = split( //, $word );
    $wmodels = "_NUL " . join(' _NUL ', @chars) . " _NUL";
    @wmodels = split( " ", $wmodels );
    
    # Parse each recognized state for the word
    $model = shift @wmodels;
    $endparse = 0;
    while( ($lin = <REC>) !~ /^\.$/ )
    {

	# if end of parse requested then abandon the parse
	next if $endparse;

	# Determine reco state and model
	($rstate, $rscore, $rmodel) = 
	    $lin =~ /^s([0-9]+)\s+([-.0-9]+)\s*\"?([^\s\"]*)\"?/;
	$savmodel = $rmodel if $rmodel;

	# Determine model phone
	$rphone = $rmhash->{$savmodel}[$rstate-2] if $savmodel;


	# If score is too low, then we have a misalignment due
	# to improper modeling of the word
	if( $rscore < -7.0 )
	{
	    print "Misaligned word: $word\n";
	    print OUT " *";
	    $endparse = 1;

	    # Undefine $model only so the 
	    # "Premature reco end..."  error message is not output
	    undef $model;
	    next;
	}	    


	# If whole model has been parsed, but recognition
	# output remains, end this parse
	if( ! $model )
	{
	    print "Premature end of $word\n";
	    print OUT "\n";
	    $endparse = 1;
	    next;
	}

	# Ignore non-tee matches
	if( $model eq '_NUL' && $rmodel eq '_NUL' )
	{
	    $model = shift @wmodels;
	    next;
	}

	# Model for this letter used tee to bypass phonetic output
	if( $model ne '_NUL' && $rmodel eq '_NUL' )
	{
	    print OUT " _";
	    # shift past the subsequent non-tee
	    $model = shift @wmodels;
	    $model = shift @wmodels;
	    next;
	}

	# If model matches recognized model first state
	# output first state
	if( $model eq $rmodel )
	{
	    $rphone = "_" if $rphone eq "_nul";
	    print OUT " $rphone";
	    $model = shift @wmodels;
	    next;
	}

	

	# If this is a continuation of a pseudo-phone for a model
	# output the next phone. A "_nul" phone is output as "_"
	if( ! $rmodel )
	{
	    $rphone = "_" if $rphone eq "_nul";
	    print OUT "$rphone";
	    next;
	}

	# The output does not parse properly, so end the parse
	print OUT "\n";
	print "Invalid parse for $word\n";
	$endparse = 1;
    }

    #Ensure that parse completed normally
    if( $model )
    {
	print "Premature reco end for $word\n";
    }

    print OUT "\n";
}
