#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # tiesr_model_convert.pl
 #
 # Convert and display contents of TIesr model set.
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
tiesr_model_convert [-Help] [-m modeldir] [-o outdir] [-l] [-bmean] [-bvar]
      [-d] [-hmm] [-pdf] [-trans] [-mean] [-var] [-gconst] [-cluster] [-centroid]";


$DOCUMENTATION = " 

This perl script converts a tiesr fixed point model set to big or
little endian.  It also provides for output of the information about the model
set to stdout so that the model contents can be examined if the -d option is
provided.  Models are assumed to hold static and delta cepstral features. When
-d is specified, additional details of the model set can be requested using the
following options:

-hmm for hmm transition and state pdf index info
-pdf for pdf gaussian weight, mean index and var index info
-trans for transition matrix info
-mean for mean vector output
-var for var vector output
-gconst for gconst data output
-cluster for cluster indices of all Gaussian mean vectors
-centroid for cluster centroid vectors

For model output, the -l option requests little endian output. The -bmean option
requests byte mean output, and the -bvar requests byte variance output.

All numeric display data is in 16 bit short format.  If the model means and/or
variances are stored in bytes, then unpacking has been done to display
the values in the proper short format.
";

#----------------------------------------------------------------

use Getopt::Long;

use File::Basename;

use TIesrModel();


# read options
GetOptions('Help' => \$outdoc,
	   'l' => \$LITTLEENDIAN,
	   'bmean' => \$bytemean,
	   'bvar' => \$bytevar,
	   'd' => \$showdata,
	   'hmm' => \$showhmm,
	   'pdf' => \$showpdf,
	   'trans' => \$showtrans,
	   'mean' => \$showmean,
	   'var' => \$showvar,
	   'gconst' => \$showgconst,
	   'centroid' => \$showcentroid,
	   'cluster' => \$showcluster,
	   'm=s' => \$modeldir,
	   'o=s' => \$outdir,
	   'h=s' => \$hmmlist );


# Output only the documentation if requested

if( $outdoc )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}

( -d $modeldir ) or die "Invalid model directory $modeldir\n";



# Create a new model set object
$models = TIesrModel->new();

# Parse the models
$models->parse( $modeldir, $hmmlist );

show_data() if $showdata;

if( $outdir )
{
    $models->output( $outdir, $bytemean, $bytevar, $LITTLEENDIAN );
}


#----------------------------------------------------------------
#  show_data

#  show requested data from models
#----------------------------------------------------------------
sub show_data
{
    print "Model data for $modeldir\n\n";

    printf "Number of words in grammar: %d\n", scalar( @{$models->{WORDS}} );
    print "Number of HMMs: $models->{NUMHMMS}\n";
    printf "Number of PDFs: %d\n", scalar( @{$models->{PDF}} );
    printf "Number of Transition matrices: %d\n", scalar( @{$models->{TRANS}} );
    print "Number of mean vectors: $models->{NUMMEANS}\n";

    print "Number of clusters: $models->{NUMCLUSTERS}\n" 
	if exists $models->{NUMCLUSTERS};

    print "Number of variance vectors: $models->{NUMVARS}\n";
    printf "Feature dimension (static+delta): %d\n", 2*$models->{DIM};


    if( $showhmm )
    {
	print "\n\nHMM information:\n";
	$index = 0;
	printf "%5s   %15s   %5s  %5s\n", "Num", "Name", "Trans", "PDFs";
	foreach $hmm ( @{$models->{HMM}} )
	{
	    printf "%5d  (%15s)  %5d", $index, $hmm->{NAME}, $hmm->{TRANS};

	    foreach $pdfidx ( @{$hmm->{PDF}} )
	    {
		printf "  %5d", $pdfidx;
	    }
	    print "\n";
	    $index++;
	}
    }


    if( $showpdf )
    {
	print "\n\nPDF information:\n";
	$index = 0;
	printf "%5s  %9s  %6s  %5s  %5s  \n", "Num", "Gaussians", "Wgt", "Mean", "Var";
	foreach $pdf ( @{$models->{PDF}} )
	{
	    printf "%5d  %9d  ", $index, $pdf->{NUMGAUSS};
	    $ngauss = 0;
	    foreach $gauss ( @{$pdf->{GAUSS}} )
	    {
		print "                  " if $ngauss != 0;
		printf "%6d  %5d  %5d\n", $gauss->{WGT}, $gauss->{MEAN}, $gauss->{VAR};
		$ngauss++;
	    }
	    $index++;
	}
    }


    if( $showtrans )
    {
	print "\n\nTRANS information:\n";
	$index = 0;

	printf "%5s  %6s  %6s\n", "Num", "States", "Entry";
	foreach $trans ( @{$models->{TRANS}} )
	{
	    $nstates = $trans->{NUMSTATES};
	    printf "%5d  %6d", $index, $nstates;

	    # Entry probabilities
	    foreach $lprob ( @{$trans->{ENTRY}} )
	    {
		printf "  %6d", $lprob;
	    }
	    print "\n";

	    #Transition probabilities from all emitting states
	    for $state (0 .. $nstates-2)
	    {
		printf "  Trn from state %2d:", $state;
		$lprobs = $trans->{TRANS}[$state];
		foreach $lprob ( @{$lprobs} )
		{
		    printf "  %6d", $lprob;
		}
		print "\n";
	    }
	    print "\n";
	    $index++;
	}
    }

    if( $showmean )
    {
	print "\n\nMean vectors:\n";
	printf "%6s\n", "Num";
	
	$index = 0;
	foreach $vec (@{$models->{MEANS}} )
	{
	    printf "%5d:", $index;
	    foreach $val ( @{$vec} )
	    {
		printf "  %6d", $val;
	    }
	    print "\n";
	    $index++;
	}
    }
    

    if( $showvar )
    {
	print "\n\nVariance vectors:\n";
	printf "%6s\n", "Num";
	
	$index = 0;
	foreach $vec (@{$models->{VARS}} )
	{
	    printf "%5d:", $index;
	    foreach $val ( @{$vec} )
	    {
		printf "  %6d", $val;
	    }
	    print "\n";
	    $index++;
	}
    }


    if( $showgconst )
    {
	print "\n\nGconst values:\n";

	$index = 0;
	foreach $gconst (@{$models->{GCONST}} )
	{
	    printf "  %5d", $gconst;
	    print "\n" if( $index%10 == 9 );
	    $index++;
	}
	print "\n";
    }


    if( $showcluster )
    {
	print "\n\nCluster indices:\n";
	printf "%7s", "Num";

	$index = 0;
	foreach $class ( @{ $models->{CLUSTER} } )
	{
	    printf "\n%6d:", $index if( $index%10 == 0 );
	    printf "%6d", $class;
	    $index++;
	}
	print "\n"
    }


    if( $showcentroid )
    {
	print "\n\nCluster centroid vectors:\n";
	printf "%6s\n", "Num";
	
	$index = 0;
	foreach $vec (@{ $models->{CENTROID} } )
	{
	    printf "%5d:", $index;
	    foreach $val ( @{$vec} )
	    {
		printf "  %6d", $val;
	    }
	    print "\n";
	    $index++;
	}
    }
}

