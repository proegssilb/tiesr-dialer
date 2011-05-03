#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # htk_model_compare.pl
 #
 # Compare acoustic match of models.
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
htk_model_compare [-Help] -H mmf_file [-l model_list] [-n] [-w workdir] [ -s] -o out_file [model ...]";


$DOCUMENTATION = " 

This perl script is used to compare the acoustic match of a list of
models in a set of HTK MMF files.  This is done by successively using
each model in the list as the recognition model, and using the mean
vector data of each model as the input data to recognize.  A
list of likelihood scores is output to show comparison of each
recognition model with all model's state feature vectors.

Since model states may contain multiple Gaussian components, there are
two methods by which the model state mean vectors are used to generate
features.  Either the vector with the highest weight is used
(default), or the -n option can be used to form a mean vector by
weighting all of the means by their weights.

The user must specify the HTK MMF files in multiple -H options.
Further, the user must supply a list of HMMs to compare.  This listing
may be in a list file specified by the -l option, or on the command
line.  Usually one method or the other should be used.  If both -l and
command line model names are supplied, the command line names will
follow at the end of the list model names. The order of the models
input in the list or/and command line is the order of the likelihood
scoring of models that will be output.

The user should supply a unique working directory to use for this
script, since the script generates quite a few working files.  The
default is to use the present working directory.  The output file is
not relative to the specified working directory.  Upon successful
creation of the scoring list, all working directory files will be
removed.

-Help : Output this help info.
-H : HTK MMF file.  This option may be repeated.
-l : File list of models to compare
-n : Normalize vectors by a sum of weighted means of state Gaussian mean components.
-o : Output file
-w : Working directory, default none.
-s : use only HMM static parameters as input data. 
";

#----------------------------------------------------------------

use Getopt::Long;

use HTKdata;

# Global variables used in the script

# configuration file 

GetOptions( 'H=s@' => \@mmffiles,
            'Help' => \$outdoc,
            'l=s' => \$modlist,
            'n'   => \$normalize,
            'o=s' => \$outfile,
            'w=s' => \$workdir,
            's' => \$staticfeats
           );


if( $outdoc )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}


if( !defined $mmffiles[0] or ! defined $outfile )
{
    print "$USAGE\n";
    exit 1;
}

if( defined $workdir )
{
    if( ! -d $workdir )
    {
	mkdir $workdir, 0770 or die "Could not make working directory\n";
    }

    $workdir .= "/" if( $workdir !~ /\/$/ );
}
else
{
    $workdir="";
}

$CONFIGFILE = "${workdir}HTKCONFIG$$.config";
$SCRIPTFILE = "${workdir}HTKSCRIPT$$.scp";
$RECFILE = "${workdir}HTKREC$$.mlf";
$GRAMFILE = "${workdir}HTKGRAM$$.grm";
$NETFILE = "${workdir}HTKNET$$.net";
$HMMFILE = "${workdir}HTKHMMS$$.lst";
$DICTFILE = "${workdir}HTKDICT$$.dict";


# make a list of hmms to compare
@hmmnames = ();

if( defined $modlist )
{
    open( LST, "$modlist") or die "Invalid list of models $modlist\n";

    while( $lin = <LST> )
    {
        chomp($lin);
        push @hmmnames,  split( " ", $lin );
    }

    close( LST );
}

# Add command line names to the list
if( @ARGV && length(@ARGV) > 0 )
{
    push @hmmnames, @ARGV;
}


# Make a listing file of the names
open LST, ">$HMMFILE" or die "Failed to open hmm listing output\n";
foreach $name (@hmmnames)
{
    print LST "$name\n";
}
close LST;

# Make a dictionary, which is just the hmm names
open DICT, ">$DICTFILE" or die "failed to open dict output\n";
foreach $name ( sort @hmmnames )
{
    print DICT "$name [$name] $name\n";
}
close DICT;


# Load in all MMF file data.  Create a new instance of 
# an HTKdata object. $hmms is a reference to the object.
$hmms = HTKdata->new();

foreach $filename (@mmffiles)
{
    $hmms->parsemmf($filename, undef, @hmmnames );
}


die "Can not support multiple streams now\n" if $hmms->{STREAMINFO}{SIZE} != 1;



# If there is a list, then check to ensure that all
# hmms in the list are loaded
if( @hmmnames )
{
    $hmms->checklist( @hmmnames );
}


# Mark the active data corresponding to the active models. 
$mexpand = "";
$hmms->markusagehmm( $mexpand );




# Create a script file and all model data test files from the hmms
 make_test_files();


# Create an MMF copy and remove deltas if needed for static feature only analysis
if( $staticfeats && $hmms->{PARMKIND} =~ /_D/ )
{
    @mmffiles = delete_deltas( @mmffiles );
}


# Make a list of MMF files for HVite command line
foreach $mmf (@mmffiles )
{
    $mmfs .= "-H $mmf ";
}

# Prepare configuration file for comparison testing
$config = $hmms->{PARMKIND};
$config =~ s/_D// if $staticfeats;
open( CFG, ">$CONFIGFILE" ) or die "Could not open config file\n";
print CFG "TARGETKIND = $config\n";
print CFG "NATURALREADORDER = T\n";
close(CFG);


# Loop over models, recognizing model "data"
$firstmodel = 1;
foreach $model (@hmmnames )
{
    # Create the supervision grammar, which is the model name
    open  GRM, ">$GRAMFILE";
    print GRM "( $model )\n";
    close GRM;
    
    
    `HParse $GRAMFILE $NETFILE`;

    `HVite -i $RECFILE -o NT -w $NETFILE -S $SCRIPTFILE -C $CONFIGFILE $mmfs $DICTFILE $HMMFILE`;

    # Output the scores for the model from the recognized mlf file
    open REC, "<$RECFILE" or die "Recognition file for $model not generated\n";

    # Open the output file
    if( $firstmodel )
    {
	open OUT, ">$outfile" or die "Could not open $outfile\n";
	$firstmodel = 0;
    }
    else
    {
	open OUT, ">>$outfile" or die "Could not open $outfile\n";
    }
    
    # Keep track of model that should be recognized next
    $recidx = 0;

    # Find all recognized models
    while( $line = <REC> )
    {
	chomp $line;
	if( $line =~ /MODEL_(.*)$$/ )
	{
	    $testmodel = $1;

	    # If some models skipped, they could not be recognized, set
	    # default score to -100.0
	    while( $testmodel ne $hmmnames[$recidx] )
	    {
		print OUT "${hmmnames[$recidx]} rec as $model score: -100.0\n";
		$recidx++;
	    }

	    # output recognized model score
	    $line = <REC>;
	    chomp $line;
	    $line =~ /${model}\s+([\S]+)/;
	    print OUT "$testmodel rec as $model score: $1\n";
	    $recidx++;
	}
    }
	

    # Set default score for any models not recognized by the end of the MLF file
    while( $recidx <= $#hmmname )
    {
	print OUT "${hmmnames[$recidx]} rec as $model score: -100.0\n";
	$recidx++;
	
    }


    close REC;
    close OUT;
}

# Remove all temp files 
unlink $CONFIGFILE;
unlink $SCRIPTFILE;
unlink $RECFILE;
unlink $GRAMFILE;
unlink $NETFILE;
unlink $HMMFILE;
unlink $DICTFILE;
unlink <${workdir}MODEL*.data>;
unlink <${workdir}HTK*$$.*>;

#----------------------------------------------------------------
# make_test_files
#
# This sub creates test files for each model to be compared. It puts
# the name of all the files in a scriptfile.
#--------------------------------
sub make_test_files
{
    open( SCP, ">$SCRIPTFILE" ) or die "Failed to open script file\n";
    foreach $model (@hmmnames)
    {

	my $rhmm = $hmms->{HMM}{$model};
	
	my $datafile = "${workdir}MODEL_${model}$$.data";
        

        make_file( $rhmm, $datafile ) or die "Failed to make data for $model\n";

	print SCP "$datafile\n";
    }

    close( SCP );
}


#----------------------------------------------------------------
#  make file
#
#  This sub makes an HTK compatible data file from the state Gaussian vectors
#  in a model
#--------------------------------
sub make_file
{
    my $rhmm = shift;
    my $datafile = shift;

    open( DATA, ">$datafile" ) or die "Could not open data file\n";


    # Output the number of vectors in the data file
    my $nstates = $rhmm->{NUMSTATES};
    $nstates -= 2;
    print DATA pack "i", $nstates;

    #Output the sampPeriod, which is dummy
    print DATA pack "i", 100;

    # Output the number of bytes per sample - each sample is a 4 byte float
    if( $staticfeats and $hmms->{PARMKIND} =~ /_D/ )
    {
	print DATA pack "s", 2*$hmms->{STREAMINFO}{VECTOR}[0];
    }
    else
    {
	print DATA pack "s", 4*$hmms->{STREAMINFO}{VECTOR}[0];
    }

    # Output the parameter kind code, which matches the input parameter code
    my $pkind = $hmms->{PARMKIND};
    $pkind =~ s/_D// if $staticfeats;

    my $kindCode = get_parmkind_code( $pkind );
    print DATA pack "S", $kindCode;

    # Fetch all vectors for the model, and output the vector
    my @vec;
    for( $state = 0; $state < $nstates; $state++ )
    {    
	@vec = get_vector( $rhmm, $state, $normalize );
	
	@vec = @vec[0 .. scalar(@vec)/2 - 1] if $staticfeats and $hmms->{PARMKIND} =~ /_D/;
	print DATA pack "f*", @vec;
    }

    close( DATA );
}


#----------------------------------------------------------------
#   get_parmkind_code
#

#   This sub returns the integer value of the code for a parmKind text
#   string, such as USER_D_Z.
#--------------------------------
sub get_parmkind_code
{
    my $kindString = shift;
    my $kindCode = 0;

    # Type array and hash
    my @types = qw( WAVEFORM LPC LPCREFC LPCCEPSTRA LPDELCEP IREFC MFCC FBANK MELSPEC USER DISCRETE );
    my %typehash;
    @typehash{@types} = ( 0 .. $#types );

    # Qualifier array and hash
    my @qualifiers = qw( E N D A C Z K O );
    my %qualhash;
    @qualhash{@qualifiers} = ( 0100, 0200, 0400, 01000, 02000, 04000, 010000, 020000 );

    # Determine the code for the type
    $kindString =~ /^([a-zA-Z]+)_?/;
    $kindCode = $typehash{$1};

    # Set any qualifiers in the code
    foreach $qual (@qualifiers)
    {
	if( $kindString =~ /^[a-zA-Z]+.*_${qual}/ )
	{
	    $kindCode |= $qualhash{$qual};
	}
    }

    return $kindCode;
}



#----------------------------------------------------------------
# get_vector
#
# This sub gets a vector that represents the data from a state.
# The vector is either the weighted sum of the Gaussian components,
# or the maximum magnitude Gaussian component.
#--------------------------------
sub get_vector
{
    my $rhmm = shift;
    my $state = shift;
    my $normalize = shift;


    # HMM state macro name
    my $stmacro = $rhmm->{STATE}[$state];
    my $rstate = $hmms->{STATE}{$stmacro};

    # Number of mixtures for this state (only one stream allowed now)
    my $nummix = $rstate->{NUMMIXES}[0];


    my $mix;
    my @vec;
    my $maxprob = 0;

    # Loop over all mixtures picking max or making weighted mean vector
    for( $mix = 0; $mix < $nummix; $mix++ )
    {
	my $prob = $rstate->{STREAMMIX}[0][$mix]{PROB};
	my $mixname = $rstate->{STREAMMIX}[0][$mix]{MIXPDF};
	my $mnname = $hmms->{MIXPDF}{$mixname}{MEAN};
	my $rmean = $hmms->{MEAN}{$mnname}{VECTOR};
	my $vsize = $hmms->{MEAN}{$mnname}{SIZE};

	my $el;
	for( $el = 0; $el < $vsize; $el++ )
	{
	    if( $normalize )
	    {
		$vec[$el] += $prob * ${$rmean}[$el];
	    }
	    else
	    {
		if( $mix == 0 )
		{
		    $vec[$el] = ${$rmean}[$el];
		}
		elsif( $prob > $lastprob )
		{
		    $vec[$el] = ${$rmean}[$el];
		}

	    }
	}

	$lastprob = $prob;
    }

    return @vec;
}


#------------------------------------------------------------
# delete_deltas

# Delete deltas from HMM files if the comparison will only 
# compare static features.
#----------------------------
sub delete_deltas
{
    my @mmffiles = @_;


    # Create list of MMF files for removing deltas
    my $mmf;
    my ( $mmf, $mmfs );
    foreach $mmf (@mmffiles )
    {
	$mmfs .= "-H $mmf ";
    }

    # New parameter kind omitting deltas
    my $skind = $hmms->{PARMKIND};
    $skind =~ s/_D//;


    my $vecsize = $hmms->{STREAMINFO}{VECTOR}[0] / 2;

    # HHEd command to remove deltas
    my $HHEDFILE = $workdir . "HTKHED$$.hed";
    open( HHED, ">$HHEDFILE" ) or die "$0 : Could not open $HHEDFILE\n";
    print HHED "SW 1 ${vecsize}\n";
    print HHED "SK ${skind}\n";
    close( HHED );

    # Create new MMF file with deltas removed
    my $MMFFILE = $workdir . "HTKMMF$$.mmf";
    `HHEd -w $MMFFILE $mmfs $HHEDFILE $HMMFILE`;

    return ($MMFFILE);
}
    
