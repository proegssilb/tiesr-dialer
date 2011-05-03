#================================================================
#  Module TIesrModel.pm

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/
#  ALL RIGHTS RESERVED


#  This module creates an object to encapsulate a TIESR fixed point
#  model set. The module creates the object and provides methods
#  to parse the models and expose the components of the models.  The
#  public interface to this object is given below.  Unlike many
#  modules, the output object structure is provided for use by
#  applications so that the models can be manipulated.
#  

#  The object has the following structure

#  $model->{NUMHMMS} = $number of HMMs in model set

#  $model->{DIM} = $feature dimension

#  $model->{WORDS} = \@array of WORDS represented by this HMM model set

#  $model->{HMMMAP} = \@array mapping each (possibly triphone) HMM index to a monophone index

#  Note: means, variances, and gconst are all represented in signed short,
#  without any conversion to floating point.

#  $model->{NUMMEANS} = $number of mean vectors
#  $model->{MEANS} = \@array of mean vector references consisting of array of feature means

#  $model->{NUMVARS} = $number of var vectors
#  $model->{VARS} = \@array of variance vectors consisting of array of variances

#  $model->{SCALES} = \@array of byte shift scaling if either mean or variance is in byte

#  $model->{GCONST} \@array of gconst values

#  $model->{TRANS} = \@array of transition structures holding transition info 

#  $model->{PDF} = \@array of pdf structures holding pdf info

#  $model->{HMM} = \@array of HMM structures holding HMM data

#  $model->{NUMCLUSTERS} = $number of gaussian clusters (if available)

#  $model->{CLUSTER} = \@array of cluster index for each gaussian mean vector

#  $model->{CENTROID} = \@array of references to cluster centroid vectors

#  The transition structure is as follows
#  $trn->{NUMSTATES} = $number of states in transition matrix
#  $trn->{ENTRY} = \@array of ln entry prob into state 0..NUMSTATES-1 (last non-emitting state excluded )
#  $trn->{TRANS} = \[$i][$j] ln prob of transition from emitting state $i to all states $j

#  The pdf structure is as follows
#  $pdf->{NUMGAUSS} = $number of Gaussians in pdf
#  $pdf->{GAUSS} = \@array of Gaussian info for each Gaussian of the pdf

#  The Gaussian info structure is as follows
#  $gauss->{WGT} = $weight in ln of Gaussian component
#  $gauss->{MEAN} = $mean vector index
#  $gauss->{VAR} = $var vector index

#  The HMM info structure is as follows
#  $hmm->{TRANS} = $index of transition matrix info for this HMM
#  $hmm->{PDF} = \@array of PDF indices for each emitting state
#  $hmm->{NAME} = $hmmname if a listing of HMMs provided during parse


#  The object has the following available public interface methods

#  Constructor for a reference to a tmf object.
#  $model = TIesrModel->new() 

#  Parse the TIesr model files into the object structure.  If an HMM
#  list file is provided then additional information will be available
#  in the structure for each HMM 
#  $model->parse( $modeldir [,$hmm_list_file] )

#  Output a model set.  Flags indicating whether to do $bytemean and
#  $bytevar output are required. If $little_endian is true output will
#  be in little endian format.
#  $model->output( $modeldir, $bytemean, $bytevar, $little_endian )

#----------------------------------------------------------------

#  Package header.  Now nothing exported.

package TIesrModel;

use strict qw( vars refs );

use Exporter ();
@TIesrModel::ISA = qw( Exporter );


#----------------------------------------------------------------
#  new

#  Construct a new TIesrModel hash data structure and return a reference
#  to it.  At this time the constructor does not provide for 
#  cloning by passing a reference to a blessed TIesrModel object.
#------------------------------------------------------------
sub new
{
    my $class = shift;
    my $this = {};
    return bless $this, $class;
}


#----------------------------------------------------------------

#  parse

#  This function parses the model data into a structure.

#----------------------------------------------------------------
sub parse
{
    my( $model,  $modeldir, $hmmfile ) = @_;

    # if configuration data exists get it or set defaults
    my $buf;
    my ($bytemean, $bytevar, $le);
    my $pks;
    if( open( FH, "${modeldir}/config.bin" ) )
    {
	read  FH, $buf, 6;
	($bytemean,$bytevar,$le) = unpack "n*", $buf;
	$pks =  $le ?  "v*"  : "n*";
	close(FH);
    }
    else
    {
	# if no config.bin, assume byte mean and var which was the latest
	# config prior to implementing config.bin.  Endian must be found
	# from dim.bin file
	$bytemean = 1;
	$bytevar = 1;
    }

    # Get dimension data.  Determine if big/little endian if needed
    open( FH, "${modeldir}/dim.bin") or die "Invalid dim.bin file\n";
    read FH, $buf, 2;
    if( defined $pks )
    {
	$model->{DIM} = unpack $pks, $buf;
    }
    else
    {
	my $dim = unpack "n", $buf;
	$pks =  ( $dim & 0xff00 )  ? "v*" : "n*";
	$model->{DIM} = unpack $pks, $buf;
    }
    close(FH);


    # Get words represented in grammar
    open( FH, "$modeldir/word.lis" ) or die "Could not open word.lis\n";
    my @words = <FH>;
    close(FH);
    chomp @words;
    $model->{WORDS} = \@words;


    # Get hmm to phone mapping array
    if( open( FH, "$modeldir/hmm2phone.bin" ) )
    {
	my $hmmidx = 0;
	while( read FH, $buf, 2 )
	{
	    my $phone = unpack $pks, $buf;
	    $phone -= 2**16 if $phone >= 2**15;
	    $model->{HMMMAP}[$hmmidx++] = $phone;
	}
	close(FH);
    }
    

    # Obtain scale information if byte mean or var
    if( $bytemean or $bytevar )
    {
	open( FH, "$modeldir/scale.bin" ) or die "Could not open scale.bin\n";
	read FH, $buf, $model->{DIM}*8;
	my @scales = unpack $pks, $buf;
	$model->{SCALES} = \@scales;
	close(FH);
    }

    # Get mean vectors
    open(FH, "$modeldir/mu.bin" ) or die "Could not open mu.bin\n";
    my $nummeans;
    read FH, $buf, 2;
    $nummeans = unpack $pks, $buf;
    $model->{NUMMEANS} = $nummeans;

    my $r_means = get_vectors( *FH, $nummeans, $model->{DIM}, $pks, $bytemean, $model->{SCALES} );
    $model->{MEANS} = $r_means;
    close(FH);

    # Get var vectors
    open(FH, "$modeldir/var.bin" ) or die "Could not open var.bin\n";
    my $numvars;
    read FH, $buf, 2;
    $numvars = unpack $pks, $buf;
    $model->{NUMVARS} = $numvars;

    # variance scales are last half of scales
    my @scales;
    if( $bytevar )
    {
	@scales = @{$model->{SCALES}};
	@scales = @scales[ 2*$model->{DIM} .. $#{$model->{SCALES}} ];
    }    
    my $r_vars = get_vectors( *FH, $numvars, $model->{DIM}, $pks, $bytevar, \@scales );

    $model->{VARS} = $r_vars;


    # Get GCONST values
    open(FH, "$modeldir/gconst.bin" ) or die "Could not open gconst.bin\n";
    my $numgconst;
    read FH, $buf, 2;
    $numgconst = unpack $pks, $buf;

    # Read all gconst values, ensuring they are in signed short
    read FH, $buf, 2*$numgconst;
    my @gconst = unpack $pks, $buf;
    my $gconst;
    foreach $gconst (@gconst)
    {
	$gconst  -= 2**16 if $gconst >= 2**15;
    } 
    $model->{GCONST} = \@gconst;
    close(FH);


    # Read in Transition matrix info
    get_trans( $modeldir, $model, $pks );


    # Read in Gaussian mixture info
    get_mixture( $modeldir, $model, $pks );

    # Read in Gaussian cluster info if it exists
    get_cluster( $modeldir, $model, $pks );


    # Read in hmm information
    get_hmm( $modeldir, $model, $pks );

    if( $hmmfile )
    {
	open( FH, $hmmfile ) or die "Could not open hmm list file\n";
	my @hmmnames = <FH>;
	close(FH);
	chomp( @hmmnames );

	die "HMM name list count mismatch\n" if ($#hmmnames != $#{$model->{HMM}} );

	my $index = 0;
	foreach my $name (@hmmnames)
	{
	    $model->{HMM}[$index++]{NAME} = $name;
	}
	close(FH);
    }

}


#----------------------------------------------------------------
# get_trans

# Read transition matrix information from file

#----------------------------------------------------------------
sub get_trans
{
    my( $modeldir, $model, $pks ) = @_;

    open(FH, "$modeldir/tran.bin" ) or die "Could not open tran.bin\n";
    my $buf;
    my $fsize;
    my $nstates;
    my $ntrans = 0;
    read FH, $buf, 2;
    $fsize = unpack $pks, $buf;

    while( read FH, $buf, 2 )
    {
	$nstates = unpack $pks, $buf;
	$model->{TRANS}[$ntrans]{NUMSTATES} = $nstates;

	my $r_entry = [];
	read FH, $buf, 2*($nstates-1);
	@$r_entry = unpack $pks, $buf;
	my $val;
	foreach $val (@$r_entry)
	{
	    $val -= 2**16  if $val >= 2**15;
	}
	$model->{TRANS}[$ntrans]{ENTRY} = $r_entry;

	# Obtain transitions from each emitting state to all states
	foreach my $state ( 0 .. $nstates-2)
	{

	    my $r_trans = [];
	    read FH, $buf, 2*($nstates);
	    @$r_trans = unpack $pks, $buf;
	    foreach $val (@$r_trans)
	    {
		$val -= 2**16  if $val >= 2**15;
	    }
	    $model->{TRANS}[$ntrans]{TRANS}[$state] = $r_trans;
	}

	$ntrans++;
    }
    close(FH);
}


#----------------------------------------------------------------
# get_mixture

# Read mixture information from file

#----------------------------------------------------------------
sub get_mixture
{
    my( $modeldir, $model, $pks ) = @_;

    open(FH, "$modeldir/mixture.bin" ) or die "Could not open tran.bin\n";
    my $buf;
    my $fsize;
    read FH, $buf, 2;
    $fsize = unpack $pks, $buf;

    my $nmix = 0;
    while( read FH, $buf, 2 )
    {
	my $ngauss;
	$ngauss = unpack $pks, $buf;
	$model->{PDF}[$nmix]{NUMGAUSS} = $ngauss;

	my $gauss;
	for $gauss ( 0 .. $ngauss-1 )
	{
	    my @ginfo;
	    read FH, $buf, 6;
	    @ginfo = unpack $pks, $buf;
	    $ginfo[0] -= 2**16 if $ginfo[0] >= 2**15;

	    @{$model->{PDF}[$nmix]{GAUSS}[$gauss]}{WGT,MEAN,VAR} = @ginfo;
	}
	$nmix++;
    }
    close(FH);
}


#---------------------------------------------------------------
# get_cluster

# Get cluster information that supports rapid JAC and ORM if 
# the files exist.
#---------------------------------------------------------------
sub get_cluster
{
    my( $modeldir, $model, $pks ) = @_;

    # Get number of gaussians clustered
    open(FH, "$modeldir/o2amidx.bin" ) or return;

    my $buf;
    my $ngauss;

    read FH, $buf, 2;
    ($ngauss) = unpack $pks, $buf;

    die "Invalid cluster data\n" unless $ngauss == $model->{NUMMEANS};

    # Read in all cluster classes for each Gaussian
    read FH, $buf, $ngauss;
    my @clusters = unpack "C*", $buf;
    $model->{CLUSTER} = \@clusters;
    close(FH);
    

    # Get number of class cluster mean centroids
    open(FH, "$modeldir/vqcentr.bin" ) or 
	die "Failed to open ${modeldir}/vqcentr.bin\n";

    read FH, $buf, 4;
    my $ncluster;
    my $ndim;
    ($ncluster, $ndim) = unpack $pks, $buf;

    $model->{NUMCLUSTERS} = $ncluster;
    
    # Read all class cluster centroid vectors
    for( my $class=0; $class < $ncluster; $class++ )
    {
	my $r_vec = [];
	
	read FH, $buf, 2*$ndim;
	@$r_vec = unpack $pks, $buf;

	foreach my $val ( @$r_vec )
	{
	    $val -= 2**16  if $val >= 2**15;
	}


	$model->{CENTROID}[$class] = $r_vec;
    }

    close(FH);
}


#----------------------------------------------------------------
#  get_hmm

#  Retrieve the gender-dependent HMM info 
#----------------------------------------------------------------
sub get_hmm
{
    my( $modeldir, $model, $pks ) = @_;

    # Get number of gender-dependent HMMs from net.bin file
    open(FH, "$modeldir/net.bin" ) or die "Could not open net.bin\n";
    my $buf;
    my $fsize;
    my $nhmms;
    read FH, $buf, 4;
    ($fsize,$nhmms) = unpack $pks, $buf;
    close(FH);
    $model->{NUMHMMS} = $nhmms;


    # Build a hash that maps from a transition offset to a transition info index.
    # This is needed because hmm.bin has transition offsets rather than indices.
    my %toffset;
    my $index = 0;
    my $offset = 0;
    foreach my $tran ( @{$model->{TRANS}} )
    {
	$toffset{$offset} = $index;
	$offset += $tran->{NUMSTATES} * $tran->{NUMSTATES};
	$index++;
    }
    
    # Open hmm.bin file
    open(FH, "$modeldir/hmm.bin" ) or die "Could not open hmm.bin\n";
    read FH, $buf, 2;
    $fsize = unpack $pks, $buf;

    # Read past offsets
    read FH, $buf, 2*$nhmms;

    #Gather data for all hmms
    foreach my $hmm ( 0 .. $nhmms-1 )
    {

	# Transition index from offset
	my $trnoff;
	read FH, $buf, 2;
	$trnoff = unpack $pks, $buf;
	die "Bad hmm offset\n" unless exists $toffset{$trnoff};
	my $tran =$toffset{$trnoff}; 
	$model->{HMM}[$hmm]{TRANS} = $tran;
	
	# PDF indices for each emitting state
	my $nstates = $model->{TRANS}[$tran]{NUMSTATES};
	my $r_pdf = [];
	read FH, $buf, 2*($nstates-1);
	@$r_pdf = unpack $pks, $buf;
	$model->{HMM}[$hmm]{PDF} = $r_pdf;
    }

    close(FH);
}


#----------------------------------------------------------------
# get_vectors

# Retrieve vectors of means or variances in short.
#----------------------------------------------------------------
sub get_vectors
{
    local (*FH) = shift;
    my ( $numvec, $dim, $pks, $byteflag, $r_scales ) = @_;

    my @vectors;
    my $vecidx;
    my $buf;
    for $vecidx (0 .. $numvec-1)
    {
	my $r_vec = [];
	if( $byteflag )
	{
	    # Read static and delta bytes packed into short values
	    read FH, $buf, 2*$dim;
	    my @vec;
	    @vec = unpack $pks, $buf;
	    
	    # unpack byte information from short data in vec
	    my $index;
	    for $index ( 0 .. $dim - 1 )
	    {
		# Convert unsigned short to two signed bytes
		$buf = pack "cc", ( ($vec[$index] & 0xff00) >> 8 ), ($vec[$index] & 0x00ff);
		my @bytes = unpack "cc", $buf;

		# byte vectors have interleaved static/dynamic elements
		$$r_vec[$index] = ( $bytes[0] * (2**8) )/( 2**$$r_scales[$index]);
		$$r_vec[$index + $dim] =  ( $bytes[1] * (2**8) )/(2**$$r_scales[$index+$dim]);
	    }
	}

	else
	{
	    # Read static and delta short data elements
	    read FH, $buf, $dim*2*2;
	    my @vec = unpack $pks, $buf;

	    # convert unsigned short to signed short
	    foreach my $val (@vec)
	    {
		$val -= 2**16 if $val >= 2**15;
	    }

	    @$r_vec = @vec;
	}

	$vectors[$vecidx] = $r_vec;
    }

    return \@vectors;
}


#------------------------------------------------------------------
# output

# Output the model data to a directory in the format requested.
#------------------------------------------------------------------
sub output
{
    my( $model, $outdir, $bytemean, $bytevar, $little_endian ) = @_;


    my $pkshort = $little_endian ?  "v*"  : "n*" ;

    # Output the configuration file
    open( CNF, ">${outdir}/config.bin" ) or
	die "Failed to open ${outdir}/config.bin\n";
    print CNF pack $pkshort, ( $bytemean ?  1 : 0 );
    print CNF pack $pkshort, ( $bytevar ?  1 : 0 );
    print CNF pack $pkshort, ( $little_endian ?  1 : 0 );
    close( CNF );


    # Output dim.bin
    open( DIM, ">${outdir}/dim.bin" ) or 
	die "Failed to open ${outdir}/dim.bin\n";
    print DIM pack $pkshort, $model->{DIM};
    close(DIM);


    # Output the word list
    open( WRD, ">${outdir}/word.lis" ) or
	die "Failed to open ${outdir}/word.lis\n";
    foreach my $word ( @{ $model->{WORDS} } )
    {
	print WRD "$word\n";
    }
    close( WRD );


    # Output hmm to phone mapping array
    open( MAP, ">${outdir}/hmm2phone.bin" ) or
	die "Failed to open ${outdir}/hmm2phone.bin\n";
    print MAP pack $pkshort, @{ $model->{HMMMAP} };
    close( MAP );


    # Output hmm transition array information
    output_trans( $model, $outdir, $pkshort );


    # Output state pdf mixture information
    output_mixture( $model, $outdir, $pkshort );

    
    output_cluster( $model, $outdir, $pkshort );

    
    # Output hmm information
    output_hmm( $model, $outdir, $pkshort );


    # Output Gaussian information, based on what characteristics 
    # of the output means and variances are requested.
    output_gconst( $model, $outdir, $pkshort );

    my $refmscales = output_mean( $model, $outdir, $pkshort, $bytemean );

    my $refvscales = output_var( $model, $outdir, $pkshort, $bytevar );

    if( $bytemean or $bytevar )
    {
	open( SC, ">${outdir}/scale.bin" ) or 
	    die "Failed to open ${outdir}/scale.bin\n";

	print SC pack $pkshort, @{ $refmscales };
	print SC pack $pkshort, @{ $refvscales };
    }
}

#-------------------------------------------------------------
# output_trans

# Output transition array data for each transition
#-------------------------------------------------------------
sub output_trans
{
    my ( $model, $outdir, $pkshort ) = @_;

    my $trnsize = 0;
    open( TRN, ">${outdir}/tran.bin" ) or
	die "Failed to open ${outdir}/tran.bin\n";

    # Dummy initial size
    print TRN pack $pkshort, $trnsize;

    #Output all transition array info
    foreach my $trnref ( @{ $model->{TRANS} } )
    {
	# Number of states in transition array including final non-emitting state
	my $nstates = $trnref->{NUMSTATES};
	print TRN pack $pkshort, $nstates;
	$trnsize++;

	# Entry probabilities in integer log
	print TRN pack $pkshort, @{ $trnref->{ENTRY} };
	$trnsize += scalar( @{ $trnref->{ENTRY} } );
	

	# Transition probabilities in integer log for each state
	foreach my $state ( 0 .. $nstates-2 )
	{
	    print TRN pack $pkshort, @{ $trnref->{TRANS}[$state] };
	    $trnsize += scalar(@{ $trnref->{TRANS}[$state] } ); 
	}
    }
    
    # output final size of tran data
    seek TRN, 0, 0;
    print TRN pack $pkshort, $trnsize;
    close( TRN );
}

#-----------------------------------------------------------------
# output_mixture

# output Gaussian mixture info. Also outputs the pdf.bin offsets
#-----------------------------------------------------------------
sub output_mixture
{
    my ( $model, $outdir, $pkshort ) = @_;

    open( MIX, ">${outdir}/mixture.bin" ) or
	die "Failed to open ${outdir}/mixture.bin\n";


    open( PDF, ">${outdir}/pdf.bin" ) or
	die "Failed to open ${outdir}/pdf.bin\n";


    # Initialize dummy size of mixture data
    my $mixsize = 0;
    print MIX pack $pkshort, $mixsize;

    # Put number of pdfs in pdf.bin
    print PDF pack $pkshort, scalar( @{ $model->{PDF} } );


    # Output each Gaussian mixture
    foreach my $mixref ( @{ $model->{PDF} } )
    {
	# Output offset of this pdf 
	print PDF pack $pkshort, $mixsize;

	# Number of Gaussian components in mixture
	my $ngauss = $mixref->{NUMGAUSS};
	print MIX pack $pkshort, $ngauss;
	$mixsize++;


	foreach my $gauss ( 0 ..  $ngauss-1 )
	{
	    my @ginfo = @{ $mixref->{GAUSS}[$gauss] }{WGT,MEAN,VAR};
	    print MIX pack $pkshort, @ginfo;
	    $mixsize += scalar( @ginfo );
	}
			   
    }

    close( PDF );

    # Output final size of mixture data
    seek MIX, 0, 0;
    print MIX pack $pkshort, $mixsize;
    close( MIX );
}

#----------------------------------------------------------------------
# output_cluster

# Output cluster information if available.
#---------------------------------------------------------------------
sub output_cluster
{
    my ( $model, $outdir, $pkshort ) = @_;

    return if ! exists $model->{NUMCLUSTERS};


    # Output cluster index for each mean vector
    open( CLS, ">${outdir}/o2amidx.bin" ) or
	die "Failed to open ${outdir}/o2amidx.bin\n";
    
    print CLS pack $pkshort, $model->{NUMMEANS};

    print CLS pack "C*", @{ $model->{CLUSTER} };

    close( CLS );


    # Output cluster centroids for each cluster class
    open( CEN, ">${outdir}/vqcentr.bin" ) or 
	die "Failed to open ${outdir}/vqcentr.bin\n";

    print CEN pack $pkshort, $model->{NUMCLUSTERS};
    print CEN pack $pkshort, scalar( @{ $model->{CENTROID}[0] } );
    for( my $class = 0; $class < $model->{NUMCLUSTERS}; $class++ )
    {
	print CEN pack $pkshort, @{ $model->{CENTROID}[$class] };
    }
    close( CEN );
}




#-----------------------------------------------------------------------
# output_hmm

# Output HMM info which tells which transition matrix, and state pdfs are
# used by each HMM.
#-----------------------------------------------------------------------
sub output_hmm
{

    my ( $model, $outdir, $pkshort ) = @_;

    open( HMM, ">${outdir}/hmm.bin" ) or
	die "Failed to open ${outdir}/hmm.bin\n";

    # Initialize dummy size
    my $hmmsize = 0;
    print HMM pack $pkshort, $hmmsize;

    # Initialize dummy HMM offsets
    my $nhmms = $model->{NUMHMMS};
    my @hmmoffset = (0) x $nhmms;
    print HMM pack $pkshort, @hmmoffset;
    $hmmsize += $nhmms;

    # Build a list that maps from a transition indices to offsets
    # in tran.bin. This is needed because HMM definitions in
    # hmm.bin specify transition offsets in tran.bin rather than indices.
    my @toffset;
    my $trnindex = 0;
    my $trnoffset = 0;
    foreach my $tran ( @{$model->{TRANS}} )
    {
	$toffset[$trnindex] = $trnoffset;
	$trnoffset += $tran->{NUMSTATES} * $tran->{NUMSTATES};
	$trnindex++;
    }


    # Output data for each HMM
    my $hmmindex = 0;
    foreach my $hmmref ( @{ $model->{HMM} } )
    {
	$hmmoffset[$hmmindex++] = $hmmsize;

	# Offset to transition array info in tran.bin
	$trnindex = $hmmref->{TRANS};
	print HMM pack $pkshort, $toffset[$trnindex];
	$hmmsize++;

	# PDF mixture indices for each state
	print HMM pack $pkshort, @{ $hmmref->{PDF} };
	$hmmsize += scalar( @{ $hmmref->{PDF} } );
    }


    # Output final file size and hmm offset info
    seek HMM, 0, 0;
    print HMM pack $pkshort, $hmmsize;
    print HMM pack $pkshort, @hmmoffset;
    close( HMM );
}


#------------------------------------------------------------
# output_gconst

# Output Gaussian constant for each Gaussian
#------------------------------------------------------------
sub output_gconst
{
    my ( $model, $outdir, $pkshort ) = @_;

    open( GC, ">${outdir}/gconst.bin" ) or
	die "Failed to open ${outdir}/gconst.bin\n";

    print GC pack $pkshort, scalar( @{ $model->{GCONST} } );
    
    print GC pack $pkshort, @{ $model->{GCONST} };

    close( GC );
}


#------------------------------------------------------------
# output_mean

# Output mean vectors. If they are to be byte-packed, then
# scales will have to be determined, and the vectors packed
# into bytes.
#------------------------------------------------------------
sub output_mean
{
    my ( $model, $outdir, $pkshort, $bytemean ) = @_;


    my @mscales;

    open( MN, ">${outdir}/mu.bin" ) or
	die "Failed to open ${outdir}/mu.bin\n";


    # Output number of mean vectors
    print MN pack $pkshort, $model->{NUMMEANS};



    # Output 16-bit mean vectors if requested
    if( ! $bytemean )
    {
	foreach my $meanref ( @{ $model->{MEANS} } )
	{
	    print MN pack $pkshort, @{ $meanref };
	}
	close( MN );

	return \@mscales;
    }
	

    # Must calculate scale vector for each static and dynamic element
    # of the mean vector
    my @maxval;
    my $mindex;
    foreach my $meanref  ( @{ $model->{MEANS} } )
    {
	$mindex = 0;
	foreach my $meanel ( @{$meanref} )
	{
	    if(  abs $meanel > $maxval[$mindex] or ! defined $maxval[$mindex] )
	    {
		$maxval[$mindex] = abs $meanel;
	    }
	    $mindex++;
	}
    }

    my $ndim = $model->{DIM};
    for( $mindex = 0; $mindex < 2 * $ndim; $mindex++ )
    {
	my $scale = 0;

	while( $maxval[$mindex] < 0x4000 )
	{
	    $maxval[$mindex] *= 2;
	    $scale++;
	}

	$mscales[$mindex] = $scale;
    }


    # Pack the mean vectors into scaled bytes
    foreach my $meanref  ( @{ $model->{MEANS} } )
    {
	for( $mindex = 0; $mindex < $ndim; $mindex++ )
	{
	    my $static = $meanref->[$mindex];
	    my $dynamic = $meanref->[$mindex + $ndim];

	    $static = $static * 2**($mscales[$mindex] - 8 );
	    $dynamic = $dynamic * 2**($mscales[$mindex + $ndim] - 8 );

	    my $outval = ( ($static & 0xff) << 8 ) | ( $dynamic & 0xff );
	    print MN pack $pkshort, $outval;
	}
    }

    return \@mscales;
}



#------------------------------------------------------------
# output_var

# Output var vectors. If they are to be byte-packed, then
# scales will have to be determined, and the vectors packed
# into bytes.
#------------------------------------------------------------
sub output_var
{
    my ( $model, $outdir, $pkshort, $bytevar ) = @_;


    my @vscales;

    open( VAR, ">${outdir}/var.bin" ) or
	die "Failed to open ${outdir}/var.bin\n";


    # Output number of variance vectors
    print VAR pack $pkshort, $model->{NUMVARS};


    # Output 16-bit var vectors if requested
    if( ! $bytevar )
    {
	foreach my $varref ( @{ $model->{VARS} } )
	{
	    print VAR pack $pkshort, @{ $varref };
	}
	close( VAR );

	return \@vscales;
    }
	

    # Must calculate scale vector for each static and dynamic element
    # of the var vector
    my @maxval;
    my $vindex;
    foreach my $varref  ( @{ $model->{VARS} } )
    {
	$vindex = 0;
	foreach my $varel ( @{ $varref } )
	{
	    if(  abs $varel > $maxval[$vindex] or ! defined $maxval[$vindex] )
	    {
		$maxval[$vindex] = abs $varel;
	    }
	    $vindex++;
	}
    }

    my $ndim = $model->{DIM};
    for( $vindex = 0; $vindex < 2 * $ndim; $vindex++ )
    {
	my $scale = 0;

	while( $maxval[$vindex] < 0x4000 )
	{
	    $maxval[$vindex] *= 2;
	    $scale++;
	}

	$vscales[$vindex] = $scale;
    }


    # Pack the mean vectors into scaled bytes
    foreach my $varref  ( @{ $model->{VARS} } )
    {
	for( $vindex = 0; $vindex < $ndim; $vindex++ )
	{
	    my $static = $varref->[$vindex];
	    my $dynamic = $varref->[$vindex + $ndim];

	    $static = $static * 2**($vscales[$vindex] - 8 );
	    $dynamic = $dynamic * 2**($vscales[$vindex + $ndim] - 8 );

	    my $outval = ( ($static & 0xff) << 8 ) | ( $dynamic & 0xff );
	    print VAR pack $pkshort, $outval;
	}
    }

    return \@vscales;
}
