#================================================================
#  HTKtmf.pm

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


#  This module creates an object to encapsulate an HTK MLLR transform
#  set.  The module creates the object and provides a method to 
#  transform data using one of the transforms from the object.  The
#  public interface to this object is given below.


#  The object has the following structure
#  $tmf->{UID} = $user_id_string
#  $tmf->{NAME} = $user_name_string
#  $tmf->{MMFID} = $mmf_id_string
#  $tmf->{RCID} = $rec_class_id_string
#  $tmf->{CHAN} = $channel_type_string
#  $tmf->{DESC} = $description_string
#  $tmf->{NBLOCKS} = $num_blocks_in_xfm
#  $tmf->{NODETHRESH} = $thresh_for_node_xfm

#  For each node, there is an occupancy count
#  $tmf->{NODEOCC}[$node] = $occupancy_float

#  The tmf may have many regression class transforms
#  $tmf->{TRANSFORM}[$class]{MEAN_TR}{SIZE} = $square_mtx_dimension_per_block
#  $tmf->{TRANSFORM}[$class]{MEAN_TR}{BLOCK}[$block][$row][$col] = $xfm_float
#  $tmf->{TRANSFORM}[$class]{BIASOFFSET} = [@bias_offset_array]
#  $tmf->{TRANSFORM}[$class]{VARIANCE_TR} = [@variance_xfm_float_array]


#  The object has the following available public interface methods

#  Constructor for a reference to a tmf object.

#  $tmf = HTKtmf->new() 

#  Parse an HTK tmf file into the object structure

#  $tmf->parsetmf( $file_name )

#  Transform a pdf using the given transform.  This transforms the
#  mean, variance, and gconst, given a transform and a mean and an
#  optional variance vector and an optional gconst value.  If you do
#  not wish the variance and gconst transformed, then use undef for
#  the input of these values, or do not include them.  If a variance 
#  transform is not included in $tmf, then the variance and gconst
#  will not be affected.  NOTE: this does the transform of the data 
#  supplied by the references directly, so it changes the original
#  contents referred to by the mean, var and gconst values.

#  $tmf->transformpdf( $class, \@mean, \@var, \$gconst );

#----------------------------------------------------------------

#  Package header.  Now nothing exported.

package HTKtmf;

use strict qw( vars refs );

use Exporter ();
@HTKtmf::ISA = qw( Exporter );


#================================================================
#  new

#  Construct a new tmf hash data structure and return a reference
#  to it.

#================================================================
sub new
{
    my $class = shift;
    my $this = {};
    return bless $this, $class;
}


#================================================================

#  parsetmf

#  This function is the top level function that opens a tmf file and
#  parses it into the structure.

#--------------------------------
sub parsetmf
{
    my( $this, $fname ) = @_;
    my( $lin, @elem );

    open( FH, "<$fname" ) or die "HTKtmf: can not open $fname\n";

    while( $lin = <FH> )
    {
	chomp($lin);

	# skip empty lines
	if( $lin =~ /^\s*$/ )
	{
	    next;
	}

	@elem = split( /[<>\s]+/, $lin );
	shift @elem if( $elem[0] eq '' );

	if( $elem[0] =~ /(UID|NAME|MMFID|RCID|CHAN|DESC|NBLOCKS|NODETHRESH)/i )
	{
	    $this->{$1} = join( " ", @elem[1 .. $#elem] );
	}
	elsif( $elem[0] =~ /NODEOCC/i )
	{
	    $this->{NODEOCC}[ $elem[1] ] = $elem[2];
	}

	elsif( $elem[0] =~ /TRANSFORM/i )
	{
	    $lin = _parsexfm($this, \*FH, $lin );
	}

	else
	{
	    # no recognized tmf format line found
	    die "HTKtmf: invalid line: $lin ";
	}
    }

    close(FH)

}

#================================================================

#  _parsexfm

#  This sub parses the transformation section of a tmf file

#--------------------------------

sub _parsexfm
{
    my( $this, $fh, $lin ) = @_;
    my( @elem );
    my( $class, $blocksize, $block, @rowvals, $vecsize );
    my( $row );
    

    while( $lin )
    {
	@elem = split( /[<>\s]+/, $lin );
	shift @elem if( $elem[0] eq '' );

	if( $elem[0] =~ /TRANSFORM/i )
	{
	    $class = $elem[1];
	}

	elsif( $elem[0] =~ /MEAN_TR/i )
	{
	    $blocksize = $elem[1];
	    $this->{TRANSFORM}[$class]{MEAN_TR}{SIZE} = $blocksize;
	}

	elsif( $elem[0] =~ /BLOCK/i )
	{
	    $block = $elem[1];

	    for( $row = 0; $row < $blocksize; $row++ )
	    {
		$lin = <$fh> or die "HTKtmf: tmf file format error\n";
		chomp $lin;
		@rowvals = split( " ", $lin );
		$this->{TRANSFORM}[$class]{MEAN_TR}{BLOCK}[$block][$row] = [ @rowvals ];
	    }
	}
	
	elsif( $elem[0] =~ /BIASOFFSET/i )
	{
	    $vecsize = $elem[1];

	    $lin = <$fh> or die "HTKtmf: tmf file format error\n";
	    chomp $lin;
	    @rowvals = split( " ", $lin );
	    die "HTKtmf: vector error\n" if (scalar @rowvals != $vecsize );
	    $this->{TRANSFORM}[$class]{BIASOFFSET} =  [ @rowvals ];
	}

	elsif( $elem[0] =~ /VARIANCE_TR/i )
	{
	    $vecsize = $elem[1];

	    $lin = <$fh> or die "HTKtmf: tmf file format error\n";
	    chomp $lin;
	    @rowvals = split( " ", $lin );
	    die "HTKtmf: vector error\n" if (scalar @rowvals != $vecsize );
	    $this->{TRANSFORM}[$class]{VARIANCE_TR} =  [ @rowvals ];
	}	

	else
	{
	    # this line is not part of a transform
	    return $lin;
	}

	# get next line
	$lin = <$fh>;
    }

    # no remaining lines
    return $lin
}


#================================================================

#  transformpdf

#  This function transforms the mean, variance, and gconst, given a
#  transform class, and the mean vector, variance vector, and gconst
#  value as references.  If you do not wish a component transformed,
#  just provide undef for that reference.

#--------------------------------
sub transformpdf
{
    my( $this, $class, $meanref, $varref, $gconstref ) = @_;
    
    my( $nblocks, $blocksize, $vecsize );
    my( $offset, $block, $blockref, $row, $col, @xfmval, $index );
    my( $index, $el );
    

    # ensure the class exists
    if( ! defined $this->{TRANSFORM}[$class] )
    {
	die "HTKtmf: invalid transform RCLASS $class\n";
    }


    # transformation block information 
    $nblocks = $this->{NBLOCKS};
    $blocksize = $this->{TRANSFORM}[$class]{MEAN_TR}{SIZE};

    # transform the mean
    if( defined $meanref )
    {
	# size of mean vector
	$vecsize = scalar @{$meanref};
					  
	die "HTKtmf: mean size error\n" if( $vecsize != $nblocks*$blocksize );

	# Initialize transformed mean vector to bias value
	@xfmval = @{ $this->{TRANSFORM}[$class]{BIASOFFSET} };

	# transformation of mean by block matrix
	# blocks are one-based
	for( $block=1; $block <= $nblocks; $block++ )
	{
	    $offset = $blocksize*($block-1);
	    $blockref = $this->{TRANSFORM}[$class]{MEAN_TR}{BLOCK}[$block];
	    
	    for( $row= 0; $row < $blocksize; $row++ )
	    {
		$index = $offset + $row;
		
		for( $col=0, $el=$offset; $col < $blocksize; $col++, $el++ )
		{
		    $xfmval[$index] += $blockref->[$row][$col]*$meanref->[$el];
		}
	    }
	}

	# copy transformed mean vector to the model
	@{ $meanref } = @xfmval;
    }

    
    # transform the variance if variance exists and transform exists
    if( defined $varref )
    {
	return unless( defined $this->{TRANSFORM}[$class]{VARIANCE_TR} );

	$vecsize = scalar @{$varref};
	die "HTKtmf: var size error\n" if ( $vecsize != $nblocks*$blocksize );
	
	for( $row = 0; $row < $vecsize; $row++ )
	{
	    $varref->[$row] *= $this->{TRANSFORM}[$class]{VARIANCE_TR}[$row];
	}
    }

    if( defined $gconstref )
    {
	return unless( defined $this->{TRANSFORM}[$class]{VARIANCE_TR} );

	$vecsize = $nblocks*$blocksize;

	for( $row = 0; $row < $vecsize; $row++ )
	{
	    ${$gconstref} *=  $this->{TRANSFORM}[$class]{VARIANCE_TR}[$row];
	}
    }
    
    return;
}
