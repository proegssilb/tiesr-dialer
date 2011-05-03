#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # htk_mmf_to_tiesr.pl
 #
 # Convert HTK model data to TIesr compatible binary file data.
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
htk_mmf_to_tiesr [-Help] -H mmffile [-H mmffile ...]  [-l listfile]
  [ -r regexp ] [-g genders] [-f features] [-bmean] [-bvar] [-le]
  [-sb] [-notrees] [-cw clusterwgt] -bwgt -d outdir";


$DOCUMENTATION = " 

This perl script is used to interface with ASCII HTK MMF model files
containing HMM data, and output the MMF file data in a set of binary
files compatible with TIesrFlex.  Options are provided to allow
flexible output for a variety of model types, including the ability
to output either triphone or monophone data.  The MMF file must meet
certain formatting criteria to be used in this script.  The
requirements are:

- All HMMs must be represented with ~h macro names.  They may all be
  gender-independent (GI) or all gender-dependent (GD), but not a
  mix. Genders are marked by gender tags appended to monophone names
  which must make a monophone name unique for a gender, preferably
  ':*', such as iy:m. Triphone names must be of the form
  *-<monophone>[<gendertag>]+* where * indicates leading and trailing
  monophone context, such aas p-er:m+l.  Note that there may be more
  than two 'genders', such as children, etc.

- There must be a silence model, denoted by the model name 'sil',
  which is gender-independent.

- All data in the active HMM set (see definition of active HMM set
  below) with the exception of the sil model must correspond to either
  triphones or monophones.  They can not be mixed.

- All triphone HMMs must have transition matrices defined in macros.
  Monophone HMMs may choose not to use macros for transition matrices,
  but then the transition matrices will automatically be given the
  macro name HMM_<phone>_TRANSP. The transition matrix macros may all
  be GD or GI, but not a mix of both.  GD HMMs may use either GI or GD
  transition macros, but GI HMMs may only use GI transition macros. A
  macro name must be of the form *<phone>[<gendertag>][_TRANSP], for
  example, T_aa or T_aa:m, or HMM_aa:m_TRANSP.  (This is not a
  requirement for the silence model.)  The transition matrix used by a
  triphone must be tied to the center monophone of the triphone. For
  example, the triphone *-aa:m+* uses the transition matrix macro T_aa
  if GI transitions, or T_aa:m if GD.

- All triphone HMM state acoustic distributions must be defined by ~s
  macros, which are the result of HTK decision tree clustering.
  Otherwise, it is not possible for TIesrFlex to synthesize the
  triphone using decision trees alone. (The binary data that TIesrFlex
  uses does not include the triphone HMM information from the MMF file
  since that would take too much space.  Instead, TIesrFlex
  synthesizes each needed triphone from decision trees.)  

- Gender-dependent state macro tags in triphones must have the gender
  tag in the state macro name, for example aa_s2_1:m or aa:m_s2_1.

The user specifies MMF files using the -H option.  Multiple MMF files
may be specified and loaded, but they must be consistent, such as
having the same feature size, etc.

The user may select a set of HMMs to be considered as the active HMM
set, from which the binary data is output.  The -l option is used to
specify a list of HMMs that are considered part of the active HMM
model set.  The list file contains one or more hmm names per text line
separated by spaces.  The -r option specifies a regular expression
that can be used to select HMMs in the active HMM set by regular
expression match.  Additional HMM names to be added to the HMM set can
be specified on the command line after options.

The -g option is used to specify genders that will be output.  This
should be a string that specifies the individual gender tags on the
models, separated by commas, for example \":m,:f\".  It is assumed that
these tags are appended to the center monophone of each triphone name,
such as p-er:m+l.  Note that if gender tagging is specified, then you
must ensure that the active HMM set selected by -l and/or -r includes
all of the gender-specific models desired.  If no gender tags are
specified, it is assumed that the HMMs are gender-independent.

The -f option specifies how many features each for static, dynamic,
and acceleration (if contained in the MMF) will be output.  It should
be an integer.

The -bmean option causes byte mean vectors to be output. Output byte
mean vectors must not be CMN, so if the input vectors are CMN, and
the -sb option is selected, the conversion will fail with an error
message. 

The -bvar option causes byte inverse variance vectors to be output.

The -le option causes output in little endian format. 

The -d option specifies the output directory, which must be specified.
If it does not exist, an attempt will be made to create it. 

The -sb option specifies to skip the bias addition on static features.
This is present since bias was not originally added to the static
features until flexphone added it during conversion of 16-bit features
to 8-bit features.

The -notrees option states that the set of models is not a triphone
model set that needs acoustic decision trees.  This will cause this
tool to output all configuration file information, where the tree
information is dummy placeholder data.

The -cw option will cluster the Gaussian weight vectors of the HMM
state pdfs, and create a VQ clustered weights file,
fxweight.cb. Separate VQ clusters are formed for each set of states
containing a specific number of Gaussian mixture components. The
clusterwgt parameter defines how clustering will be performed. If 
0 < clusterwgt < 1, then clusterwgt is the fraction of original
weight vectors that should be in the VQ clusters. For example,
if 1000 HMM states contain two mixture components and clusterwgt is
set to .01, then the 1000 weight vectors will be clustered to 
10 VQ vectors. If clusterwgt is >= 1, then it should be an integer,
and it defines the requested number of clustered VQ vectors. The
number of VQ vectors may be less than the requested number if the
original number of weight vectors is less than the requested number.
Clustering weight vectors allows significant compression of the pdf
codebook file fxclust.cb by encoding each vector as a VQ index.
The -bwgt parameter will pack the pdf clustered weight index
and number of gaussians in the pdf into a byte format. This will
limit the HMM state to a  maximum of 256 clustered weight vectors, 
and 256 mixture components in a pdf.
";

#----------------------------------------------------------------
use Getopt::Long;

# This tool uses the HTKdata module to parse the MMF file
use HTKdata;

use Algorithm::Cluster;


# Global variables used by the script

# Scaling for mfcc mean and variance, up to 16 dimensions.  
@MU_SCALE_STATIC = ( 16,4,4,2,2,2,1,1,1,1,1,1,1,1,1,1 );
@MU_SCALE_DYNAMIC = ( 4,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1 );
@MU_SCALE_ACC = ( 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 );
@MU_BIAS = ( 14951, -2789, -811, 1563, -3013, -1346, 973, -884, 355, -496, 
	     0, 0, 0, 0, 0, 0 );



# read mmf files and a file containing a list of hmms to load
GetOptions('Help' => \$outdoc,
	   'H=s@' => \@mmffil,
	   'l=s' => \$lstfil,
	   'r=s' => \$regexp,
	   'g=s' => \$genders,
	   'f=s' => \$features,
	   'bmean'   => \$bytemean,
	   'bvar'   => \$bytevar,
	   'le'  => \$littleendian,
	   'sb'  => \$skipbias,
	   'notrees' => \$notrees,
	   'd=s' => \$outdir,
	   'cw=f' => \$clusterwgt,
	   'bwgt' => \$bytewgt );


# Output only the documentation if requested

if( $outdoc )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}


# The list of MMF files must not be empty, and 
# the output directory must be specified

if( !defined $mmffil[0] or
    ! defined $outdir )
{
    print "$USAGE\n";
    exit 1;
v}


# Create outdir if it does not exist

if( ! -d $outdir )
{
    mkdir $outdir, 0770 or die "Could not make $outdir\n";
}


# Initialize regular expression if needed

( defined $regexp ) or $regexp = "";


# make a list of hmms to load from the mmf file

@hmmnames = ();

if( defined $lstfil )
{
    open( LST, "$lstfil") or die "Invalid list of hmms $lstfil\n";

    while( $lin = <LST> )
    {
	chomp($lin);
	push @hmmnames,  split( " ", $lin );
    }

    close( LST );
}


# add command line names to hmmname list

if( @ARGV && length(@ARGV) > 0 )
{
    push @hmmnames, @ARGV;
}


# Load in all MMF file data.  Create a new instance of 
# an HTKdata object. $hmms is a reference to the object.

print "Parsing MMF files\n";

$hmms = HTKdata->new();

foreach $filename (@mmffil)
{
    $hmms->parsemmf($filename, $regexp, @hmmnames );
}


# if there is a list, then check to ensure that all
# hmms in the list are loaded

if( @hmmnames )
{
    $hmms->checklist( @hmmnames );
}


# Mark the active data corresponding to the active models.  No macros
# will be marked to be expanded into their own items.  We want macros
# to be shared maximally for TIesrFlex.

$mexpand = "";
$hmms->markusagehmm( $mexpand );


# Check for conflicts using -sb option
die "Output CMN byte mean not allowed, invalid use of -sb option\n" 
    if ( $skipbias and $bytemean and $hmms->{PARMKIND} =~ /_Z/ );

die "Non-CMN input means require -sb option\n"
    if ( ! $skipbias and  $hmms->{PARMKIND} !~ /_Z/ );


# Generate an ordered list of monophones in the active hmm set,
# checking that there are the same number for each gender tag, if
# gender tags are defined.
print "Generating monophone list\n";

@monophones = monophone_list( $hmms, $genders, $outdir );


# Generate the fixed point PDF (cluster) codebook and the mean,
# variance, and gconst codebooks.  This will also output a file
# containing a list of all of the cluster macros so the order of
# PDFs in the codebook is known.  This will be needed in coding
# the acoustic decision trees files.
print "Creating cluster, mean, variance, and gconst codebooks\n";

@pdfs = create_pdf_cb( $hmms, $genders, $outdir, 
		       $features,
		       $bytemean, $bytevar,
		       $littleendian,
		       $skipbias, $clusterwgt, $bytewgt );

# Generate the fixed point transition matrix codebook and offset files
# in monophone order.  The transition codebooks are in monophone order
# according to the above generated list.  There may be
# gender-dependent transition matrices or gender independent matrices.
# If there are gender-dependent matrices then a check is made to
# ensure that transition matrices for all genders and monophones are
# covered.  This will also output a file containing a list of all
# ordered transition macros so you can see their sequence of
# organization in the codebook.  The last transition matrix is for the
# "sil" model.
print "Creating transition codebook\n";


@transitions = create_trans_cb( $hmms, $genders, $outdir,
				$littleendian,
				@monophones );



# Generate the specific file for the 'sil' HMM model.
print "Generating silence hmm information file\n";

create_sil_hmm( $hmms, $outdir, $littleendian );


# Output dummy tree configuration information if requested when data is
# monophones, or if needed since pdf weights are clustered and this info
# must be in the configuration file.
if( $notrees or $clusterwgt )
{
    print "Caution: Dummy tree information written to configuration file\n";
    $ptype = $littleendian ? "V" : "N";
    open(CFG, ">>$outdir/fxconfig.bin" ) or die "Can not open fxconfig.bin\n";
    print CFG pack $ptype, 0;

    print CFG pack $ptype, 1 if ( $clusterwgt );
    print CFG pack $ptype, $bytewgt ? 1 : 0 if ( $clusterwgt );

    close( CFG );
}


#----------------------------------------------------------------
# monophone_list

# Create a list of gender-independent monophones.  If genders are 
# specified, ensure that triphones cover all monophones for all
# genders

sub monophone_list
{
    use strict;

    my($mmfref) = shift;
    my($genders)  = shift;
    my($outdir) = shift;


    # If a gender string is defined, then determine genders needed
    my(@genderlist);
    my($genderre);
    if( $genders )
    {
	@genderlist = sort( split /\s+|\s*,\s*/ , $genders );
	$genderre = join '|', @genderlist ;
    }

    # Loop over all HMMs, determining phone names from triphone
    # names, and adding their phonenames to the phone hash.
    # Create a GI monophone list too.
    # Ignore silence HMM.
    my(%phonehash) = ();
    my(%monohash) = ();
    foreach my $hmmname ( keys %{$mmfref->{HMM}}  )
    {
	my($phonename);

	# Extract center phone name from triphones
	if ( $hmmname =~ /^\S+-(\S+)\+\S+$/ )
	{
	    $phonename = $1;
	}

	# ignore sp and sil models
	elsif ( $hmmname =~ /^sil|^sp/ )
	{
	    next;
	}
	else
	{
	    $phonename = $hmmname;
	}

	$phonehash{$phonename} = 0;
	    
	if( $genders )
	{
	    # Get monophone name from GD phone name
	    $phonename =~ /^(.+)($genderre)$/ 
		or die "Missing gender tag in HMM $hmmname\n";
	    $monohash{$1} = 0;
	}
	else
	{
	    # phone name is GI
	    $monohash{$phonename} = 0;
	}
    }

    # Check that all GD monophones are represented if necessary
    if( $genders )
    {
	foreach my $mononame ( keys %monohash )
	{
	    foreach my $gender (@genderlist)
	    {
		die "Gender dependent HMMs missing for ${mononame}${gender}\n"
		    unless exists $phonehash{ "${mononame}${gender}" };
	    }
	}
    }

    # Output the monophone list
    my($outfile) = "${outdir}/monophone.list";
    open( OUT, ">$outfile" ) or die "Can not open $outfile\n";
    foreach my $mononame ( sort keys %monohash )
    {
	print OUT "$mononame\n";
    }

    close(OUT);
    return sort keys %monohash;
}



#----------------------------------------------------------------
# create_trans_cb

# Create the transition codebook, offset, and list file.  The
# transition matrices will be output in the order specified by the
# monophone list.  If genders are specified, and gender-specific
# transition matrices exist, this function will check that there are
# gender-specific transition matrices for all monophones, and that
# they are output in proper order, with all transition matrices for
# each gender output together as a set.  This is required in order
# to look up the GD transitions easily within TIesrFlex.

sub create_trans_cb
{
    use strict;

    # Arguments to the subroutine
    my( $mmfref ) = shift;
    my( $genders ) = shift;
    my( $outdir ) = shift;
    my( $littleendian ) = shift;
    my( @monophones) = @_;


    # Transition names in triphones
    my( %tranhash ) = ();

    my $ptype = $littleendian ? "V" : "N";

    # Regular expression defining monophone context
    my( $monore ) = join '|', @monophones;


    # If a gender string is defined, then determine genders needed
    my(@genderlist);
    my($genderre);
    if( $genders )
    {
	@genderlist = sort split( /\s+|\s*,\s*/ , $genders);
	$genderre = join '|', @genderlist ;
    }
    else
    {
	$genderre = "";
    }
    

    # Loop over all triphones, gathering transition macro names.
    # Check for type of transitions - GD or GI monophone.
    # Guarantee no mix of both.
    my($gdtrans) = 0;
    my($gitrans) = 0;
    foreach my $hmmname ( keys %{$mmfref->{HMM}}  )
    {
	my($tranname);

	# Only consider (tri)phones but not sil or sp
	if ( ( $hmmname =~ /^\S+-(\S+)\+\S+$/ ) or
	     ( $hmmname !~ /^(sil|sp)($genderre)?$/ ) )
	{
	    $tranname = $mmfref->{HMM}{$hmmname}{TRANSP};

	    if( $tranname =~ /.*?($monore)(?:_TRANSP)?$/ )
	    {
		# This is a GI transition
		$gitrans = 1;

		die "Duplicate GI transition for $tranname\n"
		    if( exists $tranhash{$1} and $tranname ne $tranhash{$1} );

		$tranhash{$1} = $tranname;
	    }
	    elsif( $genders && $tranname =~ /.*?($monore)($genderre)(?:_TRANSP)?$/ )
	    {
		# This is a GD transition
		$gdtrans = 1;
		die "Duplicate GD transition $tranname\n"
		    if( exists $tranhash{ "$1$2" } and  
			$tranname ne $tranhash{"$1$2"} );
		$tranhash{ "$1$2" } = $tranname;
	    }
	    else
	    {
		die "Invalid transition name $tranname\n";
	    }

	    die "Mixed GD/GI transition macro $tranname\n"
		if( $gdtrans and $gitrans );
	}
    }
    

    # Guarantee all GI or GD transitions are covered for all monophones
    foreach my $monophone (@monophones)
    {
	if( $gdtrans )
	{
	    foreach my $gender (@genderlist )
	    {
		die "Missing transition for $monophone$gender\n" 
		    unless exists $tranhash{ "$monophone$gender" };
	    }
	}
	else
	{
	    die "Missing transition for $monophone\n" 
		unless exists $tranhash{ $monophone };
	}
    }


    # Output the codebook, offset and list files

    # Open transition codebook, offset, and list files
    open(CB, ">${outdir}/fxtran.cb" ) or die "Can not open fxtran.cb\n";
    open(OFF, ">${outdir}/fxtran.off" ) or die "Can not open fxtran.off\n";
    open(LIST, ">${outdir}/fxtran.list" ) or die "Can not open fxtran.list\n";


    # Dummy sizes to start
    print CB  pack $ptype, 0;
    print OFF  pack $ptype, 0;


    # Offset of transition matrix data in 16-bit words.  
    # Number of transition matrices.
    my $offset = 0;
    my $ntrans = 0;
    my @tranlist = ();
    my $size;
    my $tranname;

    my @gdrset = $gdtrans ?  @genderlist  :  ("");

    foreach my $gender (@gdrset)
    {
	foreach my $monophone (@monophones)
	{
	    $tranname = $tranhash{"$monophone$gender"};
	    $size = output_trans( $mmfref, 
				  $tranname,
				  $littleendian,
				  *CB );

	    print LIST "$tranname\n";
	    push @tranlist, $tranname;

	    print OFF pack $ptype, $offset;
	    $offset += $size;

	    $mmfref->{TRANSP}{$tranname}{INDEX} = $ntrans++;
	}    
    }


    # Output the transition matrix for the sil model as the
    # last transition information

    $tranname = $mmfref->{HMM}{"sil"}{TRANSP};
    die "Missing sil HMM\n" unless $tranname;
    $size = output_trans( $mmfref, 
			  $tranname,
			  $littleendian,
			  *CB );

    print LIST "$tranname\n";
    push @tranlist, $tranname;
    
    print OFF pack $ptype, $offset;
    $offset += $size;

    $mmfref->{TRANSP}{$tranname}{INDEX} = $ntrans++;


    # Output codebook size
    seek CB, 0, 0;
    print CB pack $ptype, $offset;
    close(CB);


    # Output offsets size
    seek OFF, 0, 0;
    print OFF pack $ptype, $ntrans;
    close(OFF);

    close(LIST);


    # Output further part of the configuration file, indicating whether or not
    # transition information sd GD or GI, that is, whether or not there are 
    # multiple transition matrix sets, one for each "gender".
    open(CFG, ">>$outdir/fxconfig.bin" ) or die "Can not open fxconfig.bin\n";
    print CFG pack $ptype, ($gdtrans ? 1 : 0 );
    close( CFG );

    return @tranlist;
}

#----------------------------------------------------------------
# output_trans

# This is a helper function that outputs information about one HTK
# transition structure to the codebook file.  Probabilities are 
# output as logs in Q6.

sub output_trans
{
    use strict;

    # Arguments
    my $mmfref = shift;
    my $tranname = shift;
    my $littleendian = shift;
    local(*CB) = @_;

    # Number of output shorts and type of output
    my $size = 0;
    my $ptype = $littleendian ? "v" : "n";


    # Access the transition information
    my $tranref = $mmfref->{TRANSP}{$tranname};
    my $tranprob = $tranref->{MATRIX};

    # Number of GMHMM states ( # HTK states - 1 )
    print CB pack $ptype, $tranref->{SIZE} - 1;
    $size++;

    # Entry probabilities in Ln
    foreach my $dst ( 1 .. $tranref->{SIZE}-2 )
    {
	my $logprob;
	$logprob = $tranprob->[0][$dst];
	$logprob = $logprob < 1E-5 ? -1E5 : log( $logprob );
	$logprob = qtruncate( $logprob, 6, 16 );

	print CB pack $ptype, $logprob;
	$size++;
    }

    # Transition probabilities, emitting states to emitting plus ending state
    # In Ln
    foreach my $src ( 1 .. $tranref->{SIZE}-2 )
    {
	foreach my $dst ( 1 .. $tranref->{SIZE}-1 )
	{
	    my $logprob;
	    $logprob = $tranprob->[$src][$dst];
	    $logprob = $logprob < 1E-5 ? -1E5 : log($logprob);
	    $logprob = qtruncate( $logprob, 6, 16);

	    print CB pack $ptype, $logprob;
	    $size++;
	}
    }

    return $size;
}


#----------------------------------------------------------------
# qtruncate

# Helper function that converts a number or an array of numbers
# specified by a reference, to given signed fixed Q point numbers and
# truncates to a number of bits.  The number of bits must be greater
# than one.

sub qtruncate
{
    use strict;

    #Arguments
    # $data is either a number or a reference to an array of numbers
    my $data = shift;
    my $qpoint = shift;
    my $bits = shift;


    # Min and max values for signed binary truncation
    my $min = -( 2**($bits-1) );
    my $max = -$min - 1;

    my $qmult = 2**$qpoint;


    my $number;
    my @anumbers;
    if( ref($data) eq "ARRAY" )
    {
	@anumbers= @{$data};
    }
    else
    {
	$anumbers[0] = $data;
    }

    foreach $number (@anumbers)
    {	    
	$number *= $qmult;

	# Round number to nearest integer
	$number +=  ( $number > 0 )  ?  0.5  : -0.5;
	$number = int  $number;

	$number = $min if $number < $min;
	$number = $max if $number > $max;
    }
	
    return wantarray ? @anumbers : $anumbers[0];
}


#----------------------------------------------------------------
# create_pdf_cb

# This subroutine creates the fixed point PDF (cluster) codebook and
# the mean, variance, and gconst codebooks.  This will also output a
# file containing a list of all of the cluster macros so you can
# observe their sequence in the organization of the codebook.

sub create_pdf_cb
{
    use strict;

    # Input arguments
    my $mmfref = shift;
    my $genders  = shift;
    my $outdir = shift;
    my $features = shift;
    my $bytemean = shift;
    my $bytevar = shift;
    my $littleendian = shift;
    my $skipbias = shift;
    my $clusterwgt = shift;
    my $bytewgt = shift;

    # Pack types for printing long and short
    my $plong = $littleendian ? "V" : "N";
    my $pshort = $littleendian ? "v" : "n";

    # PDF names used in triphones
    my( %pdfhash ) = ();

    # This holds the mean and variance vector components
    my( @meanvectors ) = ();
    my( @varvectors ) = ();
    
    # Determine the output features desired
    die "Can only handle one stream\n"
	unless $mmfref->{STREAMINFO}{SIZE} == 1;

    my $vecsize = $mmfref->{STREAMINFO}{VECTOR}[0];
    

    # Determine if mean vectors are CMN
    my $cmnmeans = ( $mmfref->{PARMKIND} =~ /_Z/ );
    
    # Output mean vectors are cmn if input is cmn and 
    # no bias is added
    my $cmnoutput = ( $cmnmeans and $skipbias );

    my $subvectors = 1;
    my $havedelta = 0;
    my $haveacc = 0;
    if( $mmfref->{PARMKIND} =~ /_D/ )
    {
	$subvectors++;
	$havedelta = 1;
    }
    if( $mmfref->{PARMKIND} =~ /_A/ )
    {
	$subvectors++;
	$haveacc = 1;
    }

    my $featsize = $vecsize/$subvectors;

    $features = $featsize unless defined $features;

    die "Feature dimension requested too large\n"
	unless $features <= $featsize;

    my @featselect = ();
    push @featselect, (0 .. $features-1);
    push @featselect, ( $featsize .. $featsize+$features-1) if $subvectors >= 2;
    push @featselect, ( 2*$featsize .. 2*$featsize+$features-1) if $subvectors >= 3;


    # If a gender string is defined, then determine genders needed
    my(@genderlist);
    my($genderre);
    if( $genders )
    {
	@genderlist = sort split( /\s+|\s*,\s*/ , $genders);
	$genderre = join '|', @genderlist ;
    }
    else
    {
	$genderre = "";
    }

    # Find all (tri)phone PDFs used in model set and make an ordered
    # list of pdf names
    my @pdflist = ();
    foreach my $hmmname ( sort( keys %{$mmfref->{HMM}} )  )
    {
	# Only consider (tri)phones but not sil or sp
	if ( ($hmmname =~ /^\S+-(\S+)\+\S+$/) or
	     ($hmmname !~ /^(sil|sp)($genderre)?$/) )
	{
	    my $hmmref = $mmfref->{HMM}{$hmmname};
	    my $nstates = $hmmref->{NUMSTATES};
	    foreach my $state (0 .. $nstates-3)
	    {
		my $pdfname = $hmmref->{STATE}[$state];

		# If genders specified, check state macro names are GD
		if( $genderre )
		{
		    warn "No gender in state macro name $pdfname\n"
			unless $pdfname =~ /($genderre)/;
		}

		# List of pdf names in state order by (tri)phone name
		push @pdflist, $pdfname unless exists $pdfhash{$pdfname};

		# Add pdfname to hash
		$pdfhash{$pdfname} = 0;
	    }
	}
    }


    # Add silence PDF state macro tag names at end of the pdflist
    {
	my $hmmref = $mmfref->{HMM}{"sil"};
	my $nstates = $hmmref->{NUMSTATES};
	foreach my $state (0 .. $nstates-3)
	{
	    my $pdfname = $hmmref->{STATE}[$state];
	    push @pdflist, $pdfname unless exists $pdfhash{$pdfname};
	    $pdfhash{$pdfname} = 0;
	}
    }
    

    # If weight clustering requested, cluster the weights of each pdf
    # and return an array of the weight index of each pdf. This is stored 
    # in %pdfhash.
    if( defined $clusterwgt )
    {
	my @pdfclusteridx;
	@pdfclusteridx = cluster_weights( $mmfref, \@pdflist, $clusterwgt,
					  $outdir, $littleendian, $bytewgt );
	@pdfhash{@pdflist} = @pdfclusteridx;
    }


    # Open the pdf cluster and offset files, and cluster list file
    open(CCB, ">${outdir}/fxclust.cb" ) or die "Can not open fxclust.cb\n";
    open(OFF, ">${outdir}/fxclust.off" ) or die "Can not open fxclust.off\n";
    open(LIST, ">${outdir}/fxclust.list" ) or die "Can not open fxclust.list\n";

    # Dummy sizes to start
    print CCB  pack $plong, 0;
    print OFF  pack $plong, 0;

    my $offset = 0;
    my $npdf = 0;
    my $nmean = 0;
    my $nvar = 0;


    my $pdfsize;
    # Output the data for each state
    foreach my $state (@pdflist)
    {
	print LIST "$state\n";

	$pdfsize = 0;

	# reference to state hash information for this state
	my $stateref = $mmfref->{STATE}{$state};

	# Assign index for this pdf
	$stateref->{INDEX} = $npdf++;

	# Number of Gaussian mixture components for this state PDF
	my $nmixes = $stateref->{NUMMIXES}[0];  
	
	# Output pdf num mixture and weight index if clustering weights
	if( $clusterwgt )
	{
	    if( $bytewgt )
	    {
		die "Too many mixtures in pdf $state\n" if $nmixes > 255;
		die "Too many clusters in pdf $state\n" if $pdfhash{$state} > 255;
		my $clustandmix = ( $pdfhash{$state} << 8 ) + ( $nmixes & 0xff );
		print CCB pack $pshort, $clustandmix;
		$pdfsize++;
	    }
	    else
	    {
		print CCB pack $pshort, $nmixes;
		print CCB pack $pshort, $pdfhash{$state};
		$pdfsize += 2;
	    }
	}

	else
	{
	    print CCB pack $pshort, $nmixes;
	    $pdfsize++;
	}

	# Sort mixes by probability
	my @mixprob = ();
	foreach my $mix (0 .. $nmixes-1)
	{
	    $mixprob[$mix] = $stateref->{STREAMMIX}[0][$mix]{PROB};
	}
	my @sortedmixes = sort { $mixprob[$a] <=> $mixprob[$b] } (0 .. $nmixes-1);
	    
	
	# Output information for each mixture component of this PDF by
	# probability from lower to higher. This is required by 
	# clustered weights.
	foreach my $mix ( @sortedmixes )
	{
	    if( ! $clusterwgt )
	    {
		# Output log probability of mixture component
		my $logprob = $mixprob[$mix];
		$logprob = $logprob < 1E-5 ? -1E5 : log( $logprob );
		$logprob = qtruncate( $logprob, 6, 16 );
		print CCB pack $pshort, $logprob;
		$pdfsize++;
	    }

	    # Mixture component macro tag name and reference
	    my $mixname = $stateref->{STREAMMIX}[0][$mix]{MIXPDF};
	    my $mixref = $mmfref->{MIXPDF}{$mixname};

	    # Output index of mean component
	    # Assign index and save mean vector if no index for this mean yet
	    my $meanname = $mixref->{MEAN};
	    my $meanref = $mmfref->{MEAN}{$meanname};
	    if( ! exists $meanref->{INDEX} )
	    {
		$meanref->{INDEX} = $nmean++;
		push @meanvectors, @{ $meanref->{VECTOR} }[@featselect];
	    }
	    print CCB pack $pshort, $meanref->{INDEX};
	    $pdfsize++;


	    # Output index of variance component
	    my $varname = $mixref->{VARIANCE};
	    my $varref = $mmfref->{VARIANCE}{$varname};
	    if( ! exists $varref->{INDEX} )
	    {
		die "Can only handle variance vectors\n"
		    unless $varref->{TYPE} eq "v";

		$varref->{INDEX} = $nvar++;
		push @varvectors, @{ $varref->{VECTOR} }[@featselect];
	    }
	    print CCB pack $pshort, $varref->{INDEX};
	    $pdfsize++;
	}

	# Offset is now dynamically calculated in TIesrFlex API
	# print OFF pack $plong, $offset;
	$offset += $pdfsize;
    }


    seek CCB, 0 , 0;
    print CCB pack $plong, $offset;
    close(CCB);

    seek OFF, 0, 0;
    print OFF pack $plong, $npdf;
    close(OFF);


    # Output mean codebook
    my @meanshift = output_mean( \@meanvectors, $features, 
				 $havedelta, $haveacc,
				 $nmean, $bytemean, 
				 $littleendian, $skipbias, $outdir );


    # Output variance codebook and gconst codebook
    my @varshift = output_var( \@varvectors, $features, 
			       $havedelta, $haveacc,
			       $nvar, $bytevar, 
			       $littleendian, $outdir );

    
    # Output scales if byte features output
    if( $bytemean || $bytevar )
    {
	open(SCA, ">$outdir/scale.bin" ) or die "Can not open scale.bin\n";
	print SCA pack "${pshort}*", @meanshift;
	print SCA pack "${pshort}*", @varshift;
	close(SCA);
    }



    # Output first portion of the configuration file
    open(CFG, ">$outdir/fxconfig.bin" ) or die "Can not open fxconfig.bin\n";
    # mfcc vector size in number of elements
    print CFG pack $plong, $features*$subvectors;
    # size of static portion of mfcc vector
    print CFG pack $plong, $features;
    # boolean flag indicating if feature means output as bytes
    print CFG pack $plong, ( $bytemean ?  1 : 0 );
    # boolean flag indicating if feature vars output as bytes
    print CFG pack $plong, ( $bytevar ?  1 : 0 );
    # boolean flag indicating if static mfcc vector is mean normalized
    print CFG pack $plong, ( $cmnoutput ?  1 : 0 );
    # boolean flag indicating output data is in little endian format
    print CFG pack $plong, ( $littleendian ?  1 : 0 );
    # number of "gender" hmm model sets 
    print CFG pack $plong, ( $genders ?  scalar(@genderlist) : 1 );
    close( CFG );
    
    return @pdflist;
}


#----------------------------------------------------------------
# output_mean

# This subroutine outputs the mean vectors to the mean codebook

sub output_mean
{
    use strict;

    # Arguments
    my $meansref = shift;
    my $features = shift;
    my $havedelta = shift;
    my $haveacc = shift;
    my $nmean = shift;
    my $bytefeatures = shift;
    my $littleendian = shift;
    my $skipbias = shift;
    my $outdir = shift;


    die "Vector size exceeds scale size\n"
	if $features > scalar( @main::MU_SCALE_STATIC );

    # Output element type
    my $plong = $littleendian ? "V" : "N";
    my $pshort =  $littleendian ? "v" : "n";
    my $ptype = $bytefeatures ? "c" : $pshort;


    # Open the output file, and initialize number of mean vectors
    open( MCB, ">$outdir/fxmean.cb") or die "Can not open mean CB\n";
    print MCB pack $plong, $nmean;

    # Feature configuration determining vector size and delta/acc available
    my $numfeats = $features;
    my $delta = 0;
    my $acc = 0;
    if( $havedelta )
    {
	$delta = $features;
	$numfeats += $features;
    }
    if( $haveacc )
    {
	$acc = $delta + $features;
	$numfeats += $features;
    }

    # Initialize maximum absolute values of each feature
    # and shift value for use if byte features
    my @absmax = (0) x $numfeats;
    my @max_shift = (0) x $numfeats;
    
    # Process each mean vector
    foreach my $mindex (0 .. $nmean-1)
    {
	my $startix = $mindex*$numfeats;
	my $endix = $startix + $numfeats -1;
	my @meanvec =  @{$meansref}[$startix .. $endix];

	# Scale the mean vector according to global values of scaling
	for( my $feat = 0; $feat < $features; $feat++ )
	{
	    $meanvec[$feat] /= $main::MU_SCALE_STATIC[$feat];

	    $meanvec[$delta+$feat] /= $main::MU_SCALE_DYNAMIC[$feat] 
		if $havedelta;

	    $meanvec[$acc+$feat] /= $main::MU_SCALE_ACC[$feat] 
		if $haveacc;
	}

	# Convert to fixed point shorts
	@meanvec = qtruncate( \@meanvec, 11, 16 );


	# Add bias to static vector elements, and limit
	unless( $skipbias )
	{
	    for( my $feat=0; $feat < $features; $feat++ )
	    {
		$meanvec[$feat] += $main::MU_BIAS[$feat];
	    }
	    @meanvec = qtruncate( \@meanvec, 0, 16);
	}


	# If doing byte features, then find maximum magnitude features
	if( $bytefeatures )
	{
	    # Find maximum absolute value of each feature
	    for( my $el=0; $el < $numfeats; $el++ )
	    {
		my $absval = abs( $meanvec[$el] );
		$absmax[$el]  = $absval if $absval > $absmax[$el];
	    }
	}
	    
	# Put scaled fixed point vector back into meansref
	@{$meansref}[$startix .. $endix] = @meanvec;
    }
    
    # Output means immediately if conversion to byte not necessary
    if( ! $bytefeatures )
    {
	print MCB pack "${ptype}*", @{$meansref};
	close( MCB );
	return @max_shift;
    }

    # Conversion of mean vector to bytes needed 
    

    # Scaling factors
    my $ix = 0;
    foreach my $amax (@absmax)
    {
	$max_shift[$ix] = 0;
	while( $amax < 0x4000 )
	{
	    $amax *= 2;
	    $max_shift[$ix]++;
	}
	$ix++;
    }

    foreach my $mindex (0 .. $nmean-1)
    {
	my $startix = $mindex*$numfeats;
	my $endix = $startix + $numfeats -1;
	my @meanvec =  @{$meansref}[$startix .. $endix];

	# Convert to byte elements
	@meanvec = tobyte( \@meanvec, \@max_shift );

	# Output to file
	print MCB pack "${ptype}*", @meanvec;
    }

    close(MCB);

    return @max_shift;
}
	

#----------------------------------------------------------------
# tobyte

# This subroutine implements converting a 16 bit integer value to
# a byte value for a scalar or array

sub tobyte
{
    use strict;

    # Arguments
    my $data = shift;
    my $max_shift = shift;
    
    my $shbyte = 2**8;

    my @data;
    my @sh;
    if( ref($data) eq "ARRAY" and ref($max_shift) eq "ARRAY" )
    {
	# If input is references to arrays, store array data locally
	@data = @{$data};
	@sh = @{$max_shift};
    }
    else
    {
	# Otherwise data is scalar, store as one element array
	$data[0] = $data;
	$sh[0] = $max_shift;
    }

    foreach my $ix (0 .. $#data)
    {
	my $sign = 0;
	
	# Work with positive numbers
	if( $data[$ix] < 0 )
	{
	    $data[$ix] = -$data[$ix];
	    $sign = 1;
	}
	
	# Shift data so that it is maximum representable in 8 bits
	$data[$ix] *= (2**$sh[$ix]);

	# Round top byte
	$data[$ix] += 0x80 if $data[$ix] < 0x7f00;

	# shift to form number represented in single byte
	$data[$ix]  = int ( $data[$ix]/$shbyte );

	$data[$ix] = -$data[$ix] if $sign;
    }

    return  wantarray ? @data : $data[0];
}


#----------------------------------------------------------------
# output_var

# This subroutine outputs the variance vectors to the variance codebook,
# and outputs the gconst codebook.

sub output_var
{
    use strict;

    # Arguments
    my $varsref = shift;
    my $features = shift;
    my $havedelta = shift;
    my $haveacc = shift;
    my $nvar = shift;
    my $bytefeatures = shift;
    my $littleendian = shift;
    my $outdir = shift;


    my $ln2pi = log( 2*3.1415926);


    die "Vector size exceeds scale size\n"
	if $features > scalar( @main::MU_SCALE_STATIC );

    # Output element type
    my $plong = $littleendian ? "V" : "N";
    my $pshort = $littleendian ? "v" : "n";
    my $ptype = $bytefeatures ? "c" : $pshort;


    # Open the codebook file, and initialize number of mean vectors
    open( VCB, ">$outdir/fxvar.cb") or die "Can not open fxvar.cb\n";
    print VCB pack $plong, $nvar;

    # Open gconst file, and initialize number of elements
    open( GCB, ">$outdir/fxgconst.cb") or die "Can not open fxgconst.cb\n";
    print GCB pack $plong, $nvar;


    # Feature configuration determining vector size and delta/acc available
    my $numfeats = $features;
    my $delta = 0;
    my $acc = 0;
    if( $havedelta )
    {
	$delta = $features;
	$numfeats += $features;
    }
    if( $haveacc )
    {
	$acc = $delta + $features;
	$numfeats += $features;
    }

    # For finding maximum values if byte features;
    my @absmax = (0) x $numfeats;
    my @max_shift = (0) x $numfeats;

    # Process each variance vector
    foreach my $vindex (0 .. $nvar-1)
    {
	my $startix = $vindex*$numfeats;
	my $endix = $startix + $numfeats -1;
	my @varvec =  @{$varsref}[$startix .. $endix];


	# Output gconst value prior to conversion and scaling
	my $gconst = $ln2pi*$numfeats;
	foreach my $var (@varvec)
	{
	    $gconst += log( $var );
	}
	$gconst = qtruncate( $gconst, 6, 16 );
	print GCB pack $pshort, $gconst;


	# Scale the variance vector into an inverse variance vector
	# according to global values of scaling
	for( my $feat = 0; $feat < $features; $feat++ )
	{
	    my $norm = $main::MU_SCALE_STATIC[$feat];
	    $varvec[$feat] = ($norm*$norm)/$varvec[$feat];

	    if( $havedelta )
	    {
		$norm = $main::MU_SCALE_DYNAMIC[$feat];
		$varvec[$delta+$feat] = ($norm*$norm)/$varvec[$delta+$feat];
	    }

	    if( $haveacc )
	    {
		$norm = $main::MU_SCALE_ACC[$feat];
		$varvec[$acc+$feat] = ($norm*$norm)/$varvec[$acc+$feat];
	    }
	}

	# Convert to fixed point Q9 shorts
	@varvec = qtruncate( \@varvec, 9, 16 );


	# If doing byte features, then 
	if( $bytefeatures )
	{
	    # Find maximum absolute value of each feature
	    for( my $el=0; $el < $numfeats; $el++ )
	    {
		my $absval = abs( $varvec[$el] );
		$absmax[$el]  = $absval if $absval > $absmax[$el];
	    }
	}
	    
	# Put scaled fixed point vector back into varsref
	@{$varsref}[$startix .. $endix] = @varvec;
    }
    

    # Finished with gconst codebook
    close( GCB );


    # Output inverse variances immediately if conversion to byte not necessary
    if( ! $bytefeatures )
    {
	print VCB pack "${ptype}*", @{$varsref};
	close( VCB );
	return @max_shift;
    }

    # Conversion of mean vector to bytes needed 
    

    # Scaling factors
    my $ix = 0;
    foreach my $amax (@absmax)
    {
	$max_shift[$ix] = 0;
	while( $amax < 0x4000 )
	{
	    $amax *= 2;
	    $max_shift[$ix]++;
	}
	$ix++;
    }


    foreach my $vindex (0 .. $nvar-1)
    {
	my $startix = $vindex*$numfeats;
	my $endix = $startix + $numfeats -1;
	my @varvec =  @{$varsref}[$startix .. $endix];

	# Convert to byte elements
	@varvec = tobyte( \@varvec, \@max_shift );

	# Output to file
	print VCB pack "${ptype}*", @varvec;
    }

    close(VCB);

    return @max_shift;
}


#----------------------------------------------------------------
# create_sil_hmm

# This subroutine simply outputs the information required for the 
# silence hmm.  This information has already been encoded in the
# parsed mmf structure information.

sub create_sil_hmm
{
    use strict;

    # Arguments
    my $mmfref = shift;
    my $outdir = shift;
    my $littleendian = shift;

    # Output pack type
    my $pshort = $littleendian ? "v" : "n";


    open( SIL, ">$outdir/fxsil.hmm" ) or die "Can not open fxsil.hmm\n";

    my $silref = $mmfref->{HMM}{"sil"};
    my $nstates = $silref->{NUMSTATES};

    # Output number of GMHMM states
    print SIL pack $pshort, $nstates-1;

    foreach my $state (0 .. $nstates-3)
    {
	my $statename = $silref->{STATE}[$state];
	my $stateref = $mmfref->{STATE}{$statename};
	print SIL pack $pshort, $stateref->{INDEX};
    }

    my $tranname = $silref->{TRANSP};
    my $tranref = $mmfref->{TRANSP}{$tranname};
    print SIL pack $pshort, $tranref->{INDEX};

    close(SIL);
}


#-----------------------------------------------------------------
# cluster_weights

# Cluster the mixture weights of each pdf. That way instead of storing
# all weights for all pdfs, only the weight vector index needs to be
# stored in fxclust.cb.
# ----------------------------------------------------------------------- 
sub cluster_weights
{
    use strict;

    my $mmfref = shift;
    my $pdfref = shift;
    my $clusterwgt = shift;
    my $outdir = shift;
    my $littleendian = shift;

    my %pdfwgtindex;
    my $state;
    my $pdfindex = 0;
    my @weightvecs;
    my @pdfindices;
    my $nummixes;
    my %numvectors;
    my $maxmixes = 0;
    my @pdfclusterindex;

    # Pack types for printing long and short
    my $plong = $littleendian ? "V" : "N";
    my $pshort = $littleendian ? "v" : "n";


    # Gather the weight vectors for each pdf into arrays of weights categorized
    # by number of mixtures in the pdfs
    foreach $state ( @{$pdfref} )
    {
	# Determine the number of mixtures in the pdf for
	# only the first stream. Not handling multiple streams.
	$nummixes = $mmfref->{STATE}{$state}{NUMMIXES}[0];
	$numvectors{$nummixes} = 0 if ! exists $numvectors{$nummixes};
	$maxmixes = $nummixes if $nummixes > $maxmixes;
	
	# Get the weight vector, sorted from lower to higher probability
	my @wgt = ();
	for( my $mix = 0; $mix < $nummixes; $mix++ )
	{
	    $wgt[$mix] =  $mmfref->{STATE}{$state}{STREAMMIX}[0][$mix]{PROB};
	}
	@wgt = sort { $a <=> $b } @wgt;
	
	# Store the weight vector categorized by mixture size,
	# and keep track of pdf indices used
	push @{$pdfindices[$nummixes]}, $pdfindex;
	$weightvecs[ $nummixes ]->[ $numvectors{$nummixes}++ ] = [ @wgt ] ;

	$pdfindex++;

    }

    # Open the weight cluster file
    open( WCB, ">${outdir}/fxweight.cb" ) or 
	die "Can not open ${outdir}/fxweight.cb\n";

    # Write dummy number of weight values, and vector size
    print WCB pack $plong, 0;
    print WCB pack $pshort, $maxmixes;
    
    # Loop to cluster the weight vectors categorized by mixture size
    my $numclusters = 0;
    foreach $nummixes ( sort {$a <=> $b } keys %numvectors )
    {
	# Special case for only one mixture, no clustering 
	# necessary
	if( $nummixes == 1 )
	{
	    # VQ weight vector for one mixture is all zeros
	    print WCB pack "${pshort}*", (0) x $maxmixes;
	    
	    # All pdf's with only one mixture use the first VQ weight vector
	    foreach my $index ( @{$pdfindices[$nummixes]} )
	    {
		$pdfclusterindex[$index] = 0;
	    }

	    $numclusters++;
	    next;
	}


	# Determine number of clusters requested
	my $numcl;
	if ( $clusterwgt >= 1 )
	{
	    $numcl = int $clusterwgt;
	    $numcl =  ( $numvectors{$nummixes} < $numcl ) ?
		$numvectors{$nummixes} : $numcl;
	}
	elsif ( $clusterwgt > 0 && $clusterwgt < 1 )
	{
	    $numcl = int ( $clusterwgt * $numvectors{$nummixes} + 0.5 );
	    $numcl = 1 if $numcl < 1;
	}
	else
	{
	    die "Invalid clusterwgt value\n";
	}


	# Cluster all weight vectors of size $nummixes
	my %params = ();
	%params = ( 'nclusters' => $numcl,
		    'data' => $weightvecs[$nummixes],
		    'mask' => '',
		    'weight' => '',
		    'transpose' => 0,
		    'npass' => 5,
		    'method' => 'a',
		    'dist' => 'e'
		    );
	

	my ($clusters, $error, $nfound) = Algorithm::Cluster::kcluster(%params);
	
	scalar( @{$clusters} ) == $numvectors{$nummixes} or
	    die "Cluster count mismatch for vector size $nummixes\n";

	# Map the cluster indices to the pdfs. Indices are offset by number
	# of already existing clusters
	for( my $index = 0; $index < $numvectors{$nummixes}; $index++ )
	{
	    $pdfindex = $pdfindices[$nummixes][$index];

	    $pdfclusterindex[$pdfindex] = $clusters->[$index] + $numclusters;
	}


	# Find and output the cluster centroids for this number of mixes
	%params = ();
	%params = ( 'data' => $weightvecs[$nummixes],
		    'mask' => '',
		    'clusterid' => $clusters,
		    'method' => 'a',
		    'transpose' => 0 );

	my ($vqvectors, $vqmask) = Algorithm::Cluster::clustercentroids( %params );

	# Normalize all vq centroids so they sum to one, and convert to 
	# integer log, and output the weight centroids in log probability form.
	# They should already be normalized, since the centroids are currently
	# found by arithmetic averaging.
	foreach my $vqref ( @{$vqvectors} )
	{
	    my $probsum = 0;
	    foreach my $prob ( @{$vqref} )
	    {
		$probsum += $prob;
	    }

	    foreach my $prb ( @{$vqref} )
	    {
		$prb /= $probsum;
		$prb =  ($prb < 1E-5) ?  -1E5 : log( $prb );
		$prb = qtruncate( $prb, 6, 16 );
	    }

	    # Output centroid vectors, padding to length of maxmixes
	    print WCB pack "${pshort}*", @{$vqref};
	    print WCB pack "${pshort}*", (0) x ($maxmixes - $nummixes) 
		if $maxmixes > $nummixes;
	}


	# Increment cluster count by number of clusters for this 
	# number of mixture components
	$numclusters += $numcl;
    }

    # Output number of cluster weight values
    seek WCB, 0, 0;
    print WCB pack $plong, $numclusters*$maxmixes;
    close( WCB );

    return @pdfclusterindex;
}
