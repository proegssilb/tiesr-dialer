#!/usr/bin/perl
#-------------------------------------------------------------

 #
 # sample_to_htk.pl
 #
 # Convert sample data files to TIesr-compatible cepstral vector files.
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
sample_to_htk [-Help] [ -s samplerate -be -p preemp -c numcepstra 
              -f numfilters -l floor -w winsize -t frameshift
              -o outdir ] sample_data_file";


$DOCUMENTATION = "

This script converts a binary sampled audio data file into a set of cepstral
vectors and puts the output in a parameter file that is compatible with HTK.
HTK parameter files contain a header, and the cepstral data is in big-endian
format. The binary sampled data file must be single channel with two-bytes per
sample. The cepstral parameters are compatible with TIesr. The user may
specify the parameters for the conversion. If they are not specified, defaults
will apply. The options are as follows:

-s samplerate  : Integer sample rate of the binary audio file. Default 8000/sec
-be            : Set if binary sampled data is in big-endian format
-p preemp      : Preemphasis floating point value. Default 0.96
-c numcepstra  : Number of cepstra. Default 10
-f numfilters  : Number of mel-spaced filters. Default 20
-l floor       : Floor on power spectral density
-w winsize     : Hamming sample window size. Must by power of 2. Default 256
-t frameshift  : Sample shift between frames. Default 160 (20msec @ 8000 samp/sec)
-o outdir      : Directory where output parameter files placed. Default current dir

The resulting files will have the same file name as the original file, but
will have an appended '.htk' file type.";

#----------------------------------------------------------------
use Getopt::Long;
use File::Basename;

# Use CPAN Math::FFT module
use lib "lib64/perl5/site_perl/5.8.5/x86_64-linux-thread-multi";
use Math::FFT;


GetOptions( "Help" => \$help,
	    "s=i" => \$samplerate,
	    "be" => \$bigendian,
	    "p=f" => \$preemp,
	    "c=i" => \$numcepstra,
	    "f=i" => \$numfilters,
	    "l=f" => \$psdfloor,
            "w=i" => \$winsize,
            "t=i" => \$frameshift,
	    "o=s" => \$outdir );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}

$samplerate = 8000 unless defined $samplerate;
$preemp = 0.96 unless defined $preemp;
$numcepstra = 10 unless defined $numcepstra;
$numfilters = 20 unless defined $numfilters;
$psdfloor = 0.0 unless defined $psdfloor;
$winsize = 256 unless defined $winsize;
$frameshift = 160 unless defined $frameshift;
$outdir = "." unless defined $outdir;

# Check for reasonable input
if( $numcepstra > $numfilters )
{
    die "sample_to_htk: Invalid number of cepstra/filters\n";
}

if( $frameshift > $winsize )
{
    die "sample_to_htk: Invalid window/shift size\n";
}

if( 2**( int( log($winsize)/log(2) ) ) != $winsize )
{
    die "sample_to_htk: window size not power of 2\n";
}



if( ! -d $outdir )
{
    mkdir $outdir, 0770 or die "sample_to_htk: Could not make $outdir\n";
}


# Data preparation

# Generate Hamming window
@hwindow = hamming_window( $winsize );

# Generate mel filters
$rfilters = mel_filters( $samplerate, $numfilters, $winsize );

# Setup for fft
@frame = (0) x $winsize;
$fft = Math::FFT->new(\@frame);

# Setup for dct
$rdct = dct_transform( $numcepstra, $numfilters );

# Process each of the binary sampled data files
foreach $rawfile ( @ARGV )
{
    # Find the name of the output file
    ($basename, $path, $suffix) = fileparse( $rawfile, '\.[^.]*$' );
    $outfile = $outdir. "/" . $basename . ".htk";


    # Open the raw data file
    open( RAW, "<$rawfile" ) or die "sample_to_htk: Could not open $rawfile\n";

    # Initialize cepstral vector array
    @cepstra = ();

    # Set up to input the 2-byte samples as little or big endian
    $template = $bigendian  ?  "n*"  :  "v*";

    # Obtain the first window of samples from the file
    $numbytes = read RAW, $instring, 2*$winsize;
    die "sample_to_htk: $fil too small\n" if $numbytes != 2*$winsize;
    $validframe = 1;

    # Convert to signed integer
    @inbuf = unpack $template, $instring;
    @inbuf = unpack "s*", ( pack "S*", @inbuf );

    # Preemphasis if requested
    $lastsample = 0;
    if( $preemp > 0 )
    {
	( $lastsample, @inbuf ) = preemphasis( $preemp, $lastsample, @inbuf );
    }


    # Loop over all frames
    while( $validframe )
    {
	
	# Window the data
	@frame = window( \@inbuf, \@hwindow );
	    
	# Calculate FFT of the data
	$fft = $fft->clone(\@frame);
	$coeff = $fft->rdft();

	# TIesr power spectrum
	@dftpwr = pwrspec( $coeff, $psdfloor );


	# Determine the mel-spaced filter powers
	@melspec = melfilter( \@dftpwr, $numfilters,  $rfilters );

	# Convert from mel spectrum to cepstrum vector by DFT
	@cepstrum = transform( \@melspec, $rdct );

	push @cepstra, @cepstrum;


	# Get next frameshift samples of data and preemphasize
	$numbytes = read RAW, $instring, 2*$frameshift;

	if( $numbytes < 2*$frameshift )
	{
	    $validframe = 0;
	}
	else
	{
	    # Obtain samples in signed integer
	    @subbuf = unpack $template, $instring;
	    @subbuf = unpack "s*", ( pack "S*", @subbuf );
	    
	    if( $preemp > 0 )
	    {
		( $lastsample, @subbuf ) = preemphasis( $preemp, $lastsample, @subbuf );
	    }

	    # Updated frame with new samples
	    splice( @inbuf, 0, $frameshift );
	    push @inbuf, @subbuf;
	}
    }


    # Done with the input raw data file
    close( RAW );


    # Open the HTK parameter file
    open( CEP, ">$outfile" ) or die "sample_to_htk: Could not open $outfile\n";


    # Write the HTK header in the file
    # Number of cepstral vectors
    print CEP  pack "N", int(  scalar(@cepstra)/$numcepstra );
    
    # cepstral vector sample rate
    print CEP  pack "N", int( 10000000.0*$frameshift/$samplerate );
    
    # Number of bytes per vector
    print CEP  pack "n", 4 * $numcepstra;

    # Parameter kind is USER with no other qualifiers
    print CEP  pack "n", 9;


    # Determine native endian type
    $is_le = ( (unpack "S", ( pack "v", 0x0001 ) ) == 0x0001 );

    # Write the float cepstral data as big-endian. This may require byte swapping.
    # This assumes that your machine uses IEEE floating point format.
    foreach $cep ( @cepstra )
    {
	if( !is_le )
	{
	    print CEP (pack "f", $cep);
	}
	else
	{
	    # Must convert float 4 byte value to big endian
	    @bcep = unpack "C*", pack "f", $cep;
	    print CEP ( pack "C*", reverse(@bcep) );
	}
    }
    
    close( CEP );
}


#----------------------------------------------------------------------------
# hamming_window

# Define a Hamming window
#------------
sub hamming_window
{
    my $winsize = shift @_;

    my $pi = atan2(1,1)*4;

    my @hwin;
    for( my $el = 0; $el < $winsize; $el++ )
    {
	$hwin[$el] = 0.54 - 0.46*cos( 2*$pi*$el/($winsize-1) );
    }

    return @hwin;
}


#-----------------------------------------------------------------------------
# mel_filters

# Define the mel filters for the given samplerate, number of filters and window
# size. This function returns a reference to a structure containing the frequency
# samples and weights for each of the mel filters.
#-------------------
sub mel_filters
{
    my ($samplerate, $numfilters, $winsize) = @_;

    
    # Frequency of each fft point
    my $idx;
    my @frq;
    my $frqbin = $samplerate/$winsize;
    for( $idx = 0; $idx <= ($winsize/2); $idx++ )
    {
	$frq[$idx] = $idx*$frqbin;
    }


    # Mel filterbank center frequencies and 0 Hz and half sample Hz
    my @melfrq;
    my $numlower = int(  $numfilters/2 + 2 );

    # Lower half of mel center frequencies are linear to 1kHz
    for( $idx = 0; $idx <= $numlower-1; $idx++ )
    {
	$melfrq[$idx] = 1000*$idx/($numlower-1);
    }

    # Higher half of mel center frequencies are logrithmic to half sample rate
    my $numupper = $numfilters + 2 - $numlower;
    my $logconst = 10**( ( log($samplerate/2/1000)/log(10) )/$numupper );
    for( $idx = $numlower; $idx < $numfilters + 2; $idx++ )
    {
	$melfrq[$idx] = 1000 * $logconst**( $idx - $numlower + 1 );
    }
    

    # Define filter bank for each mel frequencies. There are $numfilters 
    # filters defined in the structure, indexed as 1..$numfilters

    $filt = 1;
    for( $idx = 0; $idx <= $#frq; $idx++ )
    {
	# Start defining next filter if frequency above prior mel filter midpoint
 	$filt++ if( $frq[$idx] >= $melfrq[$filt] );

	# Filter gain at frequency given by $frq[$idx]
	$gain = ( $frq[$idx] - $melfrq[$filt-1] ) / ( $melfrq[$filt] - $melfrq[$filt-1] );

	
	# Increasing gain component of present filter
	if(  $filt <= $numfilters )
	{
	    push @{$rmelfilt->[$filt]{FILTIDX}}, $idx;
	    push @{$rmelfilt->[$filt]{FILTGAIN}}, $gain*$gain;
	}

	# Decreasing gain part of prior filter
	if( $filt > 1 && $filt <= $numfilters + 1 )
	{
	    push @{$rmelfilt->[$filt-1]{FILTIDX}}, $idx;
	    push @{$rmelfilt->[$filt-1]{FILTGAIN}}, (1 - $gain)*(1 - $gain);
	}
    }

    return $rmelfilt;
}


#----------------------------------------------------------------------------
# preemphasis

# Perform simple preemphasis of a block of data and also return the last
# sample of the data for future preemphasis of following blocks.
#----------------------
sub preemphasis
{

    my $pconst = shift @_;
    my $lastsample = shift @_;
    my @block = @_;

    # preemphasize first sample
    my @pblock;
    $pblock[0] = $block[0] - $pconst*$lastsample;

    # Preemphasize all remaining samples of the block
    for( my $idx=1; $idx <=$#block; $idx++ )
    {
	$pblock[$idx] = $block[$idx] - $pconst*$block[$idx-1];
    }

    return ( $block[$#block], @pblock );
}


#-----------------------------------------------------------------------
# window

# Apply window to a list of samples and return new list
#----------------------
sub window
{
    my $rdata = shift @_;
    my $rwindow = shift @_;

    my @windata;

    my $idx;
    for( $idx = 0; $idx <= $#{$rdata}; $idx++ )
    {
	$windata[$idx] = $$rwindow[$idx] * $$rdata[$idx];
    }

    return @windata;
}


#----------------------------------------------------------------------
# pwrspec

# Return power spectrum defined as square of magnitudes of fft coefficients
# floored by a minimum value. TIesr defines 'power' as simple amplitude 
# squared of fft coefficients.
#------------------------
sub pwrspec
{
    # Reference to FFT coefficient list
    my $rcoeff = shift @_;

    # Floor value
    my $floor = shift @_;

    # Number of coefficients in list, including real and imaginary parts
    my $numcoeff = scalar @{$rcoeff};

    my $idx;
    my $dftpwr;
   
    # Loop over all positive frequency FFT coefficients
    for($idx = 0; $idx < $numcoeff; $idx += 2 )
    {
	my $frqidx = int( $idx/2 );
	if( $frqidx == 0 )
	{
	    # Zero and half sample powers
	    $dftpwr[$frqidx] = $$rcoeff[$idx] * $$rcoeff[$idx];
	    $dftpwr[$frqidx] = $dftpwr[$frqidx] < $floor ?  $floor : $dftpwr[$frqidx];
	    
	    $frqidx = int( $numcoeff/2 );
	    $dftpwr[$frqidx] = $$rcoeff[$idx+1] * $$rcoeff[$idx+1];
	    $dftpwr[$frqidx] = $dftpwr[$frqidx] < $floor ?  $floor : $dftpwr[$frqidx];

	}
	else
	{
	    # All other frequencies
	    $dftpwr[$frqidx] = $$rcoeff[$idx] * $$rcoeff[$idx] +
		$$rcoeff[$idx+1] * $$rcoeff[$idx+1];

	    $dftpwr[$frqidx] = $dftpwr[$frqidx] < $floor ?  $floor : $dftpwr[$frqidx];
	}
    }
    return @dftpwr;
}


#----------------------------------------------------------------------------------
# melfilter

# Calculate 'power' in each mel filter
#-------------------------------
sub melfilter
{
    my $rdftpwr = shift @_;
    my $numfilters = shift @_;
    my $rfilters = shift @_;

    my @melspec;
    # Loop over all filters, gathering the power in each 
    my $filter;
    my $idx;
    for( $filter = 1; $filter <= $numfilters; $filter++ )
    {
	$melspec[$filter-1] = 0;

	# Frequency indices and filter gains for this filter
	my @indices = @{$rfilters->[$filter]{FILTIDX}};
	my @gains = @{$rfilters->[$filter]{FILTGAIN}};

	for( $idx = 0; $idx <= $#indices; $idx++ )
	{
	    my $frqidx = $indices[$idx];
	    my $gain = $gains[$idx];

	    $melspec[$filter-1] +=  $gain * $rdftpwr->[$frqidx];
	}

	$melspec[$filter-1] = log( $melspec[$filter-1] ) / log(10);
    }

    return @melspec;
}


#-------------------------------------------------------------------------------------
# dct_transform

# Define the dct transform for the TIesr cepstral processing
#-----------------------------
sub dct_transform
{
    my $numcepstra = shift @_;
    my $numfilters = shift @_;

    my (  $cidx, $fidx );
    my $rdct;
    my $pi = atan2(1,1)*4;
    for( $cidx = 0; $cidx < $numcepstra; $cidx++ )
    {
	for( $fidx = 0; $fidx < $numfilters; $fidx++  )
	{
	    $rdct->[$cidx][$fidx] = cos( $pi*$cidx*($fidx+0.5)/$numfilters );
	}
    }

    return $rdct;
}


#----------------------------------------------------------------------------
# transform

# transform the mel spectrum  vector by the dct matrix.
#-------------------
sub transform
{
    my $rdata = shift @_;
    my $rdct = shift @_;

    # Check that dimensions of matrix-vector multiply aggree
    if( scalar @$rdata != scalar @{$rdct->[0]} ) 
    {
	die "sample_to_htk: Transform size mismatch\n";
    }

    my ( $cidx, $fidx );
    my @cepstrum;


    # Perform simple matrix-vector multiply. Should use fast DCT
    # if one is available.
    for( $cidx = 0; $cidx <= $#{$rdct}; $cidx++ )
    {
	my $cep = 0;
	for( $fidx = 0; $fidx <= $#{$rdct->[0]}; $fidx++ )
	{
	    $cep += $rdct->[$cidx][$fidx] * $$rdata[$fidx];
	}
	push @cepstrum, $cep;
    }

    return @cepstrum;
}
