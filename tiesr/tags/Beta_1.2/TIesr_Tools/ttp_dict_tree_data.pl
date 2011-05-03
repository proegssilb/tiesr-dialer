#!/usr/bin/perl
#----------------------------------------------------------------
#  ttp_dict_tree_data.pl

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


$USAGE = "
ttp_dict_tree_data [-Help] -d dictfile [ -L letterleft -R letterright ]
                   [-p phonefile -pL phoneleft -pR phone right ]
                   [-o outdir -w workdir ]";


$DOCUMENTATION = "
This script accepts as input the letter-to-phone mapping dictionary,
and creates the .names and .data files for each letter found in the
dictionary. The .names and .data files will be used by the C4.5
decision tree training program to make decision trees for each
letter. The -d option specifies the location of the letter-to-phone
mapping dictionary file. The -L option specifies how many letter
attributes to the left should be generated (default 4). The -R option
specifies how many letter attributes to the right should be generated
(default 4). The -p file specifies the name of the phone label
file. The -pL specifies how many phone attributes to the left should
be generated (default 0). The -pR specifies how many phone attributes
to the right should be generated (default 0). The -o option specifies
the output directory, and the -w option specifies a working
directory. ";

#----------------------------------------------------------------

use Getopt::Long;

use File::Copy;


GetOptions( "Help" => \$help,
	    "d=s" => \$dictfile,
	    "L=i" => \$numleftcxt,
	    "R=i" => \$numrightcxt,
	    "p=s" => \$phnlblfile,
	    "pL=i" => \$numlblleftcxt,
	    "pR=i" => \$numlblrightcxt,
	    "o=s" => \$outdir,
	    "w=s" => \$workdir
	    );


if( $help )
{
    print "$USAGE\n\n";
    print "$DOCUMENTATION\n";
    exit 1;
}

# Assign defaults
$numleftcxt = 4 unless defined( $numleftcxt );
$numrightcxt = 4 unless defined( $numrightcxt );

$numlblleftcxt = 0 unless defined( $numlblleftcxt );
$numlblrightcxt = 0 unless defined( $numlblrightcxt );

$outdir = "." unless defined( $outdir );
$workdir = "." unless defined( $workdir );


# $dictfile must exist. $phnclsfile must exist if using phone attributes.
if( ! -f $dictfile ||
    ( ! -f $phnlblfile && 
      ($numlblleftcxt > 0 || $numlblrightcxt > 0 ) )  )
{
    print "$USAGE\n\n";
    print "$DOCUMENTATION\n";

    die "ttp_dict_tree_data: Invalid input\n";
}


# Read in the phone label matrix
if( $numlblleftcxt > 0 || $numlblrightcxt > 0 )
{
    open( LBL, "<$phnlblfile" ) or die "ttp_dict_tree_data: Could not open $phnlblfile\n";
    
    $numphnlbl = 0;
    while( $lin = <LBL> )
    {
	chomp $lin;

	@el = split( /\s+/, $lin );

	# phone name
	$phn = shift @el;

	# phone values for the labels
	$phnlbls{$phn} = [@el];

	# number of phone labels
	$numphnlbls = scalar(@el) unless $numphnlbls > scalar(@el);


	# hash of phone label values for each phone label
	# and for all labels
	for( $lbl = 0; $lbl <= $#el; $lbl++ )
	{
	    $lblvals[$lbl]{ $el[$lbl] } = 1;
	    $alllblvals{ $el[$lbl] } = 1;
	}
    }

    close( LBL );
    $dophnlbl = 1;


    # Create TIESR phone label value file
    $phnlblfile = "${outdir}/onewphone.list";
    open( LBL, ">$phnlblfile" ) or die "ttp_dict_tree_data: Could not open $phnlblfile\n";
    foreach $phn ( sort( keys( %phnlbls ) ) )
    {
	print LBL "$phn ", join( " ", @{$phnlbls{$phn}} ), "\n";
    }
    close( LBL );
}


# Create the TIesr decision tree attribute name listing file.
$attnames = $outdir . "/cAttType.txt";
open( TYP, ">$attnames" ) or die "ttp_dict_tree_data: Could not open $attnames\n";

# Letter left context
for( $cxt = $numleftcxt; $cxt > 0; $cxt-- )
{
    print TYP "L$cxt\n";
}

# Letter right context
for( $cxt = 1; $cxt <= $numrightcxt; $cxt++ )
{
    print TYP "R$cxt\n";
}

if( $dophnlbl )
{
    # For each phone label
    for( $plbl = 1; $plbl <= $numphnlbls; $plbl++ )
    {
	
	# Left phone class label context
	for( $cxt = $numlblleftcxt; $cxt > 0; $cxt-- )
	{
	    print TYP "P${plbl}L$cxt\n";
	}
	
	# Right phone class label context
	for( $cxt = 1; $cxt <= $numlblrightcxt; $cxt++ )
	{
	    print TYP "P${plbl}R$cxt\n";
	}
    }
}

close( TYP );


# Parse the dictionary into tree data files and gather
# letter to phone mappings.
open( DICT, "<$dictfile" ) or die "ttp_dict_tree_data: Could not open $dictfile\n";

%ltpmap = ();
%lethash = ();

while( $lin = <DICT> )
{
    chomp $lin;

    parse_dict_line( $lin, $workdir );
}
close( DICT );


# Output the C4.5 name files for each letter encountered in the 
# dictionary
output_names_files( $workdir );


# Create the attribute values file. This includes both the letter
# values and the phone label values that can be assigned.
$attvalues = $outdir . "/cAttValue.txt";
open( VAL, ">$attvalues" ) or die "ttp_dict_tree_data: Could not open $attvalues\n";

foreach $let ( sort keys %lethash )
{
    print VAL "$let\n";
}
foreach $val ( sort keys %alllblvals )
{
    print VAL "$val\n";
}

# The default no-attribute label
print VAL "NO_ATTRIBUTE\n" if ! exists( $alllblvals{ "NO_ATTRIBUTE" } );

close( VAL );


#-------------------------------------------------------------
sub parse_dict_line
{

    my( $lin, $workdir) = @_;


    # Extract word, pseudo-phones making up the word, and
    # letters in the word
    my( @wdphones ) = split( " ", $lin );
    my( $word ) = shift @wdphones;
    $word =~ s/^\"?(.*?)\"?$/$1/;
    my( @letters ) = split('', $word );

    # Sanity check of dictionary
    die "ttp_dict_tree_data: Invalid dictionary entry: $lin\n" unless $#letters == $#wdphones;

    # Process each letter in the word, building list of pseudo-phones mapping
    # to the letter.  Build the work files sequentially.
    my $letidx;
    for( $letidx = 0; $letidx <= $#letters; $letidx++ )
    {
	# Current letter of word
	my ($letter) = $letters[$letidx];


	# Open the letter decision tree data file for this letter
	# Open in append if the letter already encountered.
	my $fn;
	$fn = sub_letter( $letter );
	$fn = ">$workdir/${fn}.data";
	$fn = ">$fn" if exists $lethash{$letter};
	open( DATA, $fn ) or die "ttp_dict_tree_data: Could not open $fn\n";


	# Update hash of letters and letter-to-phone map
	$lethash{$letter} = 1;
	$ltpmap{$letter}{$wdphones[$letidx]} = 1;


	# Output data for this letter in its contexts
	# Letter left context
	my $firstval = 1;
	my $cxt;
	for( $cxt = $letidx-$numleftcxt; $cxt < $letidx; $cxt++ )
	{
	    print DATA "," if !$firstval;

	    if( $cxt < 0 )
	    {
		print DATA "no";
	    }
	    else
	    {
		$cletter = sub_letter( $letters[$cxt] );
		print DATA $cletter;
	    }
	    $firstval = 0;
	}


        # Letter right context
	for( $cxt = $letidx + 1; $cxt <= $numrightcxt + $letidx; $cxt++ )
	{
	    print DATA "," if !$firstval;

	    if( $cxt > $#letters )
	    {
		print DATA "no";
	    }
	    else
	    {
		$cletter = sub_letter( $letters[$cxt] );
		print DATA $cletter;
	    }
	    $firstval = 0;
	}


	# Phone label contexts
	if( $dophnlbl )
	{
	    # For each phone label
	    my $plbl;
	    for( $plbl = 0; $plbl < $numphnlbls; $plbl++ )
	    {
		
		# Left phone label context
		for( $cxt = $letidx - $numlblleftcxt; $cxt < $letidx; $cxt++ )
		{

		    print DATA "," if !$firstval;

		    if( $cxt < 0 )
		    {
			print DATA "no";
		    }
		    else
		    {
			my( $phn ) = $wdphones[$cxt];
			my( $phnlbl ) = $phnlbls{$phn}[$plbl];
			print DATA $phnlbl;
		    }
		    $firstval = 0;
		}

		# Right phone label context
		for( $cxt = $letidx + 1; $cxt <= $letidx + $numlblrightcxt; $cxt++ )
		{

		    print DATA "," if !$firstval;

		    if( $cxt > $#letters )
		    {
			print DATA "no";
		    }
		    else
		    {
			my( $phn ) = $wdphones[$cxt];
			my( $phnlbl ) = $phnlbls{$phn}[$plbl];
			print DATA $phnlbl;
		    }
		    $firstval = 0;
		}
	    }
	}
	
	# Output the phone corresponding to context attributes
	print DATA ",${wdphones[$letidx]}.\n";
	close( DATA );
    }
}


#---------------------------------------------------------------
sub output_names_files
{
    my( $workdir ) = shift @_;

    foreach $letter ( sort keys( %lethash ) )
    {
	my $fn;

	
	# Open the .names file for the letter
	$fn = sub_letter( $letter );
	$fn = "${workdir}/${fn}.names";
	open( LET, ">$fn" ) or die "ttp_dict_tree_data: Could not open $fn\n";


	# Print all pseudo-phones mapped to the letter
	my(@letphones) = sort keys( %{$ltpmap{$letter}} );
	print LET join(",", @letphones), ".\n";


	# Print possible discrete values for each attribute

	my( @letvalues ) = sort keys( %lethash );

	foreach $letv (@letvalues)
	{
	    $letv = sub_letter( $letv );
	}
	unshift( @letvalues, "no" );


        # Letter left context
	for( $cxt = $numleftcxt; $cxt > 0; $cxt-- )
	{
	    print LET "L${cxt}: " , join(",", @letvalues) , ".\n";
	}

        # Letter right context
	for( $cxt = 1; $cxt <= $numrightcxt; $cxt++ )
	{
	    print LET "R${cxt}: " , join(",", @letvalues) , ".\n";
	}

	if( $dophnlbl )
	{
	    # For each phone label
	    for( $plbl = 1; $plbl <= $numphnlbls; $plbl++ )
	    {
		
		# All values that can be assigned to this label
		@labelvals = sort keys( %{$lblvals[$plbl - 1]} );
		unshift( @labelvals, "no" );


		# Left phone class label context
		for( $cxt = $numlblleftcxt; $cxt > 0; $cxt-- )
		{
		    print LET "P${plbl}L$cxt: ", join(",", @labelvals) , ".\n";
		}
	
		# Right phone class label context
		for( $cxt = 1; $cxt <= $numlblrightcxt; $cxt++ )
		{
		    print LET "P${plbl}R$cxt: " , join(",", @labelvals) , "\n";
		}
	    }
	}

	close( LET );
    }
}





#---------------------------------------------------------------
# sub letter
#
# substitute for punctuation letters for TIesr.
#
sub sub_letter
{
    my( $letter ) = shift @_;

    $sletter = $letter;
    $sletter = "dot" if $letter eq ".";
    $sletter = "dash" if $letter eq "-";
    $sletter = "up" if $letter eq "'";
    $sletter = "down" if $letter eq "_";

    return $sletter;
}
