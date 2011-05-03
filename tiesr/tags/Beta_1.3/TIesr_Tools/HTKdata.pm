#-------------------------------------------------------------

 #
 # HTKdata.pm
 #
 # Module to parse HTK MMF files.
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

	
# This module contains functions to act on an HTK HMM dataset
# in ascii MMF format.  The description of the  functions that
# the user will call are provided below.

# $hmms = HTKdata->new()
# Creates a reference to a hash which will be filled to
# contain all of the hmm's and each macro structure in the MMF.
# Note that all components of an HMM are completely created as
# macros.  Thus each mean vector will have its own macro name, etc.

# The $hmms structure contains all HMM information in the MMF files.
# The detail of the structure is:
#
# Global options:
# $hmms->{HMMSETID} = $set_id_name
# $hmms->{STREAMINFO}{SIZE} = $number_of_streams
# $hmms->{STREAMINFO}{VECTOR} = [@width_of_each_stream]
# $hmms->{DURKIND} = $kind_string
# $hmms->{PARMKIND} = $kind_string

# Regression class macros:
# $hmms->{REGTREE}{$tree_name}{USAGE} = $usage_counter
# $hmms->{REGTREE}{$tree_name}{LEAFS} = $number_of_leaf_nodes
# $hmms->{REGTREE}{$tree_name}{NODE}[$node] = [@child nodes]
# $hmms->{REGTREE}{$tree_name}{TNODE}[$node] = $num_mixes

# Mean macros:
# $hmms->{MEAN}{$mean_name}{USAGE} = $usage_counter
# $hmms->{MEAN}{$mean_name}{TMFRCLASS} = $tmf_rclass_applied
# $hmms->{MEAN}{$mean_name}{STREAM} = $stream_associated
# $hmms->{MEAN}{$mean_name}{SIZE} = $param_vector_size
# $hmms->{MEAN}{$mean_name}{VECTOR} = [@mean_vector]

# Variance/covariance macros:
# $hmms->{VARIANCE}{$var_name}{USAGE} = $usage_counter
# $hmms->{VARIANCE}{$var_name}{TMFRCLASS} = $tmf_rclass_applied
# $hmms->{VARIANCE}{$var_name}{STREAM} = $stream_associated
# $hmms->{VARIANCE}{$var_name}{TYPE} = $type (v or i)
# $hmms->{VARIANCE}{$var_name}{SIZE} = $param_vector_size
# $hmms->{VARIANCE}{$var_name}{VECTOR} = [@cov_data] (may be var or cov)

# Transition macros:
# $hmms->{TRANSP}{$trans_name}{USAGE} = $usage_counter
# $hmms->{TRANSP}{$trans_name}{SIZE} = $trans_state_size
# $hmms->{TRANSP}{$trans_name}{MATRIX}[$row][$col]  = $trans_prob

# Stream weights macros:
# $hmms->{SWEIGHTS}{$weight_name}{USAGE} = $usage_counter
# $hmms->{SWEIGHTS}{$weight_name}{SIZE} = $number_of_streams
# $hmms-SWEIGHTS}{$weight_name}{VECTOR} = [ @stream_weights]

# Mixture macros:
# $hmms->{MIXPDF}{$mix_name}{USAGE} = $usage_counter
# $hmms->{MIXPDF}{$mix_name}{STREAM} = $stream_associated
# $hmms->{MIXPDF}{$mix_name}{RCLASS} = $regression_class
# $hmms->{MIXPDF}{$mix_name}{MEAN} = $mean_macro_name
# $hmms->{MIXPDF}{$mix_name}{VARIANCE} = $var_macro_name
# $hmms->{MIXPDF}{$mix_name}{GCONST} = $gconst_value


# State macros:
# $hmms->{STATE}{$state_name}{USAGE} = $usage_counter
# $hmms->{STATE}{$state_name}{SWEIGHTS} = $swgts_macro_name
# $hmms->{STATE}{$state_name}{NUMMIXES} = [@num_mixes_per_stream]
# $hmms->{STATE}{$state_name}{STREAMMIX}[$stream][$mixture]{PROB} = $prob
# $hmms->{STATE}{$state_name}{STREAMMIX}[$stream][$mixture]{MIXPDF} = $mix_macro_name

# HMM macros:
# $hmms->{HMM}{$hmm_name}{NUMSTATES} = $number_of_states
# $hmms->{HMM}{$hmm_name}{STATE}[$state] = $state_macro_name
# $hmms->{HMM}{$hmm_name}{TRANSP} = $trans_macro_name

# $hmms->parsemmf($filename, $regexp, @hmmnames );
# Parses an mmf file and puts the parts of the HMMs in the $hmms hash
# structure.  $filename is the name of the mmf.  $regexp defines a
# regular expression used to determine which HMMs in the MMF will be loaded. HMM
# names that match $regexp will be loaded.  @hmmnames is
# an array of hmm names that will also be loaded. Names in 
# the array will be loaded.  If both are undef or empty, all HMMs will be
# loaded by default.

# $hmms->checklist( @hmmnames )
# This function checks to ensure that all hmms in a list have been loaded 
# into the hmm object and dies if an hmm is missing.

# $hmms->markusagehmm($opt)
# This function marks which macro elements are needed by the set of
# loaded HMMs by updating usage counters.  Since the HMMs are the last
# things loaded, all mmf macros must be loaded, even though not
# used by an hmm.  $opt is a string that contains macro identifier 
# characters (smuvitw).  If any macro identifier character is specified,
# then that item will not be output as a macro, but rather will be expanded
# inline in the output, removing the macro and duplicating the data each
# time it is used.  This is flagged by using a negative usage count.

# $hmms->transformpdfs($tmf)
# This function transforms all of the pdfs that will be used via a TMF structure
# as defined in HTKtmf.pm.  It should be called after markusagehmm.  The MIXPDF
# structures must have a valid RCLASS defined or the transformation will not
# take place.

# $hmms->compressedmmf( \*FH )
# Output an mmf to the file handle which includes only the macros and data
# necessary for the given HMM set and discards the other macros and data.
# You must call markusagehmm first.

# $hmms->mergehmms( $mrghfile )
# This function merges multiple hmms.  The hmms must have the same
# number of states.  Merging is performed by collecting all stream
# mixtures for each model into a single larger stream mixture distribution. 
# Note that if models share components via macros, these will remain
# shared, though the number of mixtures will still increase.  Thus shared
# variances will still be shared accordingly, etc.  Most likely the user
# would not have shared means, etc.
# The transition probability matrix is the average of all matrices.
# Stream weights, if state specific, are those of the first model in the list.
# If a model is not found for sharing, an informational message is output,
# but this does not stop the merging.  The final HMM model name will be
# that of the first HMM model in the list.  All other HMMs will be deleted
# from $hmms.  $mrghfile is the name of a file that contains sets of hmms
# to merge, where each line of the file contains a list of hmms to merge as
# one set.

# $hmms->mergestates( $mrgsfile )
# This function merges multiple states.  Merging is performed by
# collecting all stream mixtures for each state into the first state,
# creating a larger stream mixture distribution.  Note that if states
# share components via macros, these will remain shared, though the
# number of mixtures will still increase.  Thus shared variances will
# still be shared accordingly, etc.  Most likely the user would not
# have shared means in the merged states. Stream weights, if state
# specific, are those of the first state in the list.  If a state name
# is not found for sharing, an informational message is output, but
# this does not stop the merging.  The final states after merging will
# be used to modify HMM model state names to reflect the reduced
# number of states.  All states merged with the first state will be
# deleted.  $mrgsfile is a file that contains sets of states to merge, where
# all of the state macro names on each line of the file are merged as a set.

# $hmms->mergevar( $mrgvfile )
# This function merges multiple variances.  Merging is performed by
# collecting all of the variances specified, and substituting the macro
# name of the first variance for all of the remaining variance macro names.
# The variance output will be the average for all variances.  $mrgvfile is
# a file that contains sets of variance macros to merge, where each line
# contains the macro names of variances to merge into a single variance.

# $hmms->mergemean( $mrgmfile )
# This function merges multiple mean vectors.  Merging is performed by
# collecting all of the means specified, and substituting the macro
# name of the first mean on each line for all of the remaining mean macro names.
# The mean output will be the average for all means.  $mrgmfile is
# a file that contains sets of mean macros to merge, where each line
# contains the macro names of means to merge into a single mean.

#----------------------------------------------------------------
package HTKdata;

use strict qw( vars refs );

use HTKtmf;

use Exporter ();

@HTKdata::ISA = qw( Exporter );


#================================================================
#  new

#  construct new hmm data structure

#--------------------------------
sub new
{
    my $class = shift;
    my $this = {};
    return bless $this, $class;
}


#================================================================
#  parsemmf

#  Parses an HTK HMM model file or MMF mmf into an htkdata structure
#  for further processing.

#--------------------------------
sub parsemmf
{
    my ($this, $mmffil, $regexp, @hmmlist ) = @_;
    my( $mname, $wname, $pname, $sname, $hname, %hmmhash );
    my ( $lin, $tname, $vtype, $vname );
    my ( $rname );

    if( @hmmlist ) { @hmmhash{ @hmmlist } = ( 0 .. $#hmmlist ); }

    open(MMF, "$mmffil" ) or die "Can not open $mmffil\n";
 
    $lin = <MMF>;
    while( $lin )
    {
	chomp($lin);

	# ignore blank lines
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <MMF>;
	    next;
	}

	# global options macro
	if ( $lin =~ /^\s*~o/ )
	{
	    $lin = &parseopt( $this, \*MMF, $lin );
	}
	    
	# regression tree macro
	elsif ( $lin =~ /^\s*~r/ )
	{
	    ($rname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = parsereg( $this, \*MMF, $lin, $rname );
	}

	# transition macro
	elsif ( $lin =~ /^\s*~t/ )
	{
	    ($tname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = parsetrans( $this, \*MMF, $lin, $tname );
	}

	# mean macro
	elsif ( $lin =~ /^\s*~u/ )
	{
	    ($mname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = parsemean( $this, \*MMF, $lin, $mname );
	}

	# variance/covariance macro
	elsif ( $lin =~ /^\s*~[vi]/ )
	{
	    ($vtype, $vname) = $lin =~ /^\s*~([vi])\s+\"([^\"]+)\"/;
	    $lin = parsecov( $this, \*MMF, $lin, $vtype, $vname );
	}
	
	# stream weights macro
	elsif ( $lin =~ /^\s*~w/ )
	{
	    ($wname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = parseswgt( $this, \*MMF, $lin, $wname );
	}

	# mixpdf component macro
	elsif ( $lin =~ /^\s*~m/ )
	{
	    ($pname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = &parsemix( $this, \*MMF, $lin, $pname );
	}

	# state macro
	elsif ( $lin =~ /^\s*~s/ )
	{
	    ($sname) = $lin =~ /\"([^\"]+)\"/;
	    $lin = &parsestate( $this, \*MMF, $lin, $sname );
	}
	    
	# hmm macro
	elsif ( $lin =~ /^\s*~h/ )
	{
	    ($hname) = $lin =~ /\"([^\"]+)\"/;
	    if( ( $regexp && $hname =~ /$regexp/ ) ||
		( @hmmlist && exists $hmmhash{$hname} ) ||
		( ! $regexp && ! @hmmlist ) )
	    { 
 		$lin = parsehmm( $this, \*MMF, $lin, $hname );
	    }
	    else
	    {
		#next line past hmm
		while( $lin !~ /<ENDHMM>/i )
		{
		     $lin = <MMF>;
		}
		$lin = <MMF>;
	    }
	}

	# no macro recognized
	else
	{
	    $lin = <MMF>;
	}
    }
    close( MMF );
}


#================================================================

#  parseopt

#  Parse options and load into the data structure.  Check multiple 
#  mmf files for consistency.

#--------------------------------
sub parseopt
{
    my ( $this, $fh, $lin ) = @_;
    my ( $var, $numstrm, @strmdata, @elem );
    my ( $i, $data );

  LINE:
    while( $lin )
    {
	chomp($lin);
	
	# ignore line with only ~o or blank line
	if( $lin =~ /^\s*(~o)?\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	($elem[0] ne '' ) or shift @elem;
	shift @elem if( $elem[0] =~ /~o/ );
	
	while( @elem )
	{
	    $var = $elem[0];

	    # streaminfo tag
	    if( $var =~ /STREAMINFO/i )
	    {
		$numstrm = $elem[1];
		@strmdata = @elem[ 2 .. 2+$numstrm-1 ];
		
		# consistency check, stream info size
		die "Inconsistent stream info size\n" 
		    if ( exists $this->{STREAMINFO}{SIZE} &&
			 $numstrm != $this->{STREAMINFO}{SIZE} );

		$this->{STREAMINFO}{SIZE} = $numstrm;

		if( exists $this->{STREAMINFO}{VECTOR} )
		{
		    $i = 0;
		    foreach $data ( @{ $this->{STREAMINFO}{VECTOR} } )
		    {			
			( $data == $strmdata[$i] ) or 
			    die "Inconsistent stream size\n";
			$i++;
		    }
		}
		else
		{
		    $this->{STREAMINFO}->{VECTOR} = [@strmdata];
		}

		@elem =  @elem[$numstrm+2 .. $#elem];
	    }
		
	    # vecsize tag
	    elsif( $var =~ /VECSIZE/i )
	    {
		die "Inconsistent vecsize\n" 
		    if (  exists $this->{VECSIZE} &&
			  $this->{VECSIZE} != $elem[1] );

		$this->{VECSIZE} = $elem[1];

		@elem = @elem[2 ..$#elem];
	    }

	    elsif( $var =~ /HMMSETID/i )
	    {
		die "Multiple different hmm sets\n"
		    if ( exists $this->{HMMSETID} && 
			 $this->{HMMSETID} ne $elem[1] );
		$this->{HMMSETID} = $elem[1];
		@elem =  @elem[ 2 .. $#elem ];
	    }

	    elsif( $var =~ /NULLD|POISSOND|GAMMAD|GEND/i  )
	    {
		# define duration kind key
		die "Inconsistent duration kinds\n"
		    if( exists $this->{DURKIND} &&
			$this->{DURKIND} ne $var );
		$this->{DURKIND} = $var;
		shift @elem;
	    }

	    elsif( $var =~/DIAGC|INVDIAGC|FULLC|LLTC|XFORMC/i )
	    {
		die "Inconsistent cov kind\n"
		    if( exists $this->{COVKIND} && $this->{COVKIND} ne $var );
		$this->{COVKIND} = $var;
		shift @elem;
	    }

	    elsif( $var =~ /^(DISCRETE|LPC|LPCEPSTRA|MFCC|FBANK|MELSPEC|LPREFC|LPDELCEP|USER)/i )
	    {
		die "Inconsistent parm kind\n"
		    if( exists $this->{PARMKIND} && $this->{PARMKIND} ne $var );
		$this->{PARMKIND} = $var;
		shift @elem;
	    }

	    # not recognized tag
	    else
	    {
		last LINE;
	    }
	}

	# get next line
	$lin = <$fh>;
    }

    return $lin;
}



#================================================================

#  parsereg

#  Parse a regression tree structure

#--------------------------------

sub parsereg
{
    my ($this, $fh, $lin, $rname ) = @_;
    my ( $startofparse, @elem );
    my ( $numleafs );

    $rname or die "No regression tree name\n";

    if(  exists $this->{REGTREE}{$rname} )
    {
	die "Duplicate regression tree name $rname\n";
    }

    #initialize usage of regression tree macro
    $this->{REGTREE}{$rname}{USAGE} = 0;

    $startofparse = 1;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~r/ )
    {
	$lin = <$fh>;
    }

  LINE:
    while( $lin )
    {

	chomp($lin);

	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	# get components on the line
	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # reg tag parse and init
	    if( $startofparse )
	    {
		die "Invalid regression line $lin\n" unless ( $elem[0] =~ /REGTREE/i );
		$startofparse = 0;

		$numleafs = $elem[1];
		$this->{REGTREE}{$rname}{LEAFS} = $numleafs;

		@elem = @elem[2 .. $#elem];
		$lin = join( ' ', @elem);
	    }

	    # gather regression tree macro components
	    else
	    {
		if( $elem[0] =~ /^NODE/i )
		{
		    die "Invalid regression node\n" unless ( $#elem == 3 );
		    $this->{REGTREE}{$rname}{NODE}[ $elem[1] ] = [ @elem[2 .. $#elem] ];
		    
		    @elem = @elem[4 .. $#elem];
		}

		elsif( $elem[0] =~ /TNODE/i )
		{
		    die "Invalid regression terminal node\n" unless ( $#elem == 2 );

		    # the original tree mixture counts may not be valid if loading or
		    # outputting only a subset of the models
		    # $this->{REGTREE}{$rname}{TNODE}[ $elem[1] ]  = $elem[2];

		    # Keep a count of pdfs that cluster to this node when doing markusage
		    $this->{REGTREE}{$rname}{TNODE}[ $elem[1] ]  = 0;

		    @elem = @elem[3 .. $#elem];
		}

		else
		{
		    last LINE;
		}
	    }
	}

	# get next line to process
	$lin = <$fh>;
    }

    return $lin;
}


#================================================================

#  parsemean

#  Loads a mean vector macro into the hmmdata structure

#--------------------------------
sub parsemean
{
    my ( $this, $fh, $lin, $mname ) = @_;
    my ( $startofparse, @elem, $nummean, @mnvec );

    # mean vector name

    $mname or die "No mean name\n";

    if(  exists $this->{MEAN}->{$mname} )
    {
	die "Duplicate mean name $mname\n";
    }
	
    # initialize usage of this mean macro
    $this->{MEAN}->{$mname}->{USAGE} = 0;

    $startofparse = 1;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~u/ )
    {
	 $lin = <$fh>;
    }

  LINE:
    while( $lin )
    {
	chomp($lin);

	# ignore blank lines
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # mean tag parse and init
	    if( $startofparse )
	    {
		die "Invalid mean line $lin\n" unless ( $elem[0] =~ /MEAN/i );
		$startofparse = 0;

		$nummean = $elem[1];
		$this->{MEAN}->{$mname}->{SIZE} = $nummean;
		@mnvec = ();

		@elem = @elem[2 .. $#elem];
		$lin = join( ' ', @elem);
	    }

	    # gather mean vector
	    if( ! $startofparse )
	    {
		if( $lin !~ /[~<>A-DF-Z]/i )
		{
		    push @mnvec, @elem;
		   ( @mnvec <= $nummean ) or
		       die "Invalid mean vector\n";
		    @elem = ();
		}
		else
		{
		    last LINE;
		}
	    }
	}

	# get next line to process
	$lin = <$fh>;
    }

    # have read available input data, check and load
    ( @mnvec == $nummean ) or die "Invalid mean vector length\n";
    $this->{MEAN}->{$mname}->{VECTOR} = [@mnvec];

    return $lin;
}



#================================================================

#  parsecov

#  Loads a variance/covariance macro into the hmmdata structure

#--------------------------------
sub parsecov
{
    my ( $this, $fh, $lin, $vtype, $vname ) = @_;
    my ( $startofparse, @elem, $vsize, $numvar, @varvec );

    $vname or die "No cov name\n";

    if(  exists $this->{VARIANCE}->{$vname} )
    {
	die "Duplicate cov name $vname\n";
    }

    # initialize usage of this variance macro
    $this->{VARIANCE}->{$vname}->{TYPE} = $vtype;
    $this->{VARIANCE}->{$vname}->{USAGE} = 0;

    $startofparse = 1;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~[vi]/ )
    {
	$lin = <$fh>;
    }

  LINE:
    while( $lin )
    {
	chomp($lin);

	# ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # variance or invcovar tag and initialize
	    if( $startofparse )
	    {
		( $elem[0] =~ /VARIANCE|INVCOVAR/i ) or
		    die "Invalid cov line $lin\n";

		$startofparse = 0;
		$vsize = $elem[1];
		$this->{VARIANCE}->{$vname}->{SIZE} = $vsize;
		$numvar = ( $vtype =~ /v/ ) ? $vsize : $vsize*($vsize+1)/2;
		@varvec = ();

		@elem = @elem[2 .. $#elem];
		$lin = join ' ', @elem;
	    }

	    # input variance or covariance data into vector
	    if( ! $startofparse )
	    {
		if( $lin !~ /[~<>A-DF-Z]/i )
		{
		    push @varvec, @elem;
		    ( @varvec <= $numvar ) or
			die "Invalid variance vector\n";
		    @elem = ();
		}
		else
		{
		    last LINE;
		}
	    }
	}

	# get next line to process
	$lin = <$fh>;
    }


    # have all available input data, check and load
    ( @varvec == $numvar ) or die "Invalid variance vector length\n";
    $this->{VARIANCE}->{$vname}->{VECTOR} = [@varvec];

    return $lin;
}


#================================================================

#  parsetrans

#  Loads a trans array macro into the hmmdata structure

#--------------------------------
sub parsetrans
{
    my ($this, $fh, $lin, $tname ) = @_;
    my ( $startofparse, @elem, $numtrans, @tmtx, @tvec );

    $tname or die "No transition name\n";

    if(  exists $this->{TRANSP}->{$tname} )
    {
	die "Duplicate trans name $tname\n";
    }

    #initialize usage of transp macro
    $this->{TRANSP}->{$tname}->{USAGE} = 0;

    $startofparse = 1;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~t/ )
    {
	$lin = <$fh>;
    }

  LINE:
    while( $lin )
    {

	chomp($lin);

	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # transp tag and initialize matrix
	    if( $startofparse )
	    {
		die "Invalid trans line $lin\n" unless 
		    ( $elem[0] =~ /TRANSP/i );
		$startofparse = 0;

		$numtrans = $elem[1];
		$this->{TRANSP}->{$tname}->{SIZE} = $numtrans;

		@tmtx = ();
		@tvec = ();
		@elem = @elem[2 .. $#elem];
		$lin = join ' ', @elem;
	    }

	    # read all rows of transp matrix
	    if( ! $startofparse )
	    {
		if( $lin !~ /[~<>A-DF-Z]/i )
		{
		    push @tvec, @elem;
		    ( @tvec <= $numtrans ) or die "Invalid trans row $tname\n";
		    if( @tvec == $numtrans )
		    {
			push @tmtx, [@tvec];
			@tvec = ();
		    }
		    @elem = ();
		}
		else
		{
		    last LINE;
		}
	    }
	}

	# get next line to process
	$lin = <$fh>;
    }

    # have all available data, check and load hmm structure
    ( @tmtx == $numtrans ) or die "Invalid trans matrix size\n";
    $this->{TRANSP}->{$tname}->{MATRIX} = [@tmtx];

    return $lin;
}


#================================================================

#  parsewgts

#  Loads a stream weight vector macro into the hmmdata structure

#--------------------------------
sub parsewgts
{
    my ( $this, $fh, $lin, $wname ) = @_;
    my ( $startofparse, @elem, $wsize, @wvec );

    $wname or die "No weight name\n";

    if(  exists $this->{SWEIGHTS}->{$wname} )
    {
	die "Duplicate weight name $wname\n";
    }

    # initialize usage of stream weight macro
    $this->{SWEIGHTS}->{$wname}->{USAGE} = 0;

    $startofparse = 1;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~w/ )
    {
	$lin = <$fh>;
    }

  LINE:
    while( $lin )
    {

	chomp($lin);

	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    if( $startofparse )
	    {
		( $elem[0] =~ /SWEIGHTS/i ) or
		    die "Invalid wgts line $lin\n";
		$wsize = $elem[1];
		$this->{SWEIGHTS}->{$wname}->{SIZE} = $wsize;
		$startofparse = 0;
		@wvec = ();
		@elem = @elem[2 .. $#elem];
		$lin = join ' ', @elem;
	    }

	    if( ! $startofparse )
	    {
		if( $lin !~ /[~<>A-DF-Z]/i )
		{
		    push @wvec, @elem;
		    ( @wvec <= $wsize ) or
			die "Invalid sweight vector\n";
		    @elem = ();
		}
		else
		{
		    last LINE;
		}
	    }
	}

	# get next line to process
	$lin = <$fh>;
    }

    # have data, check and load
    ( @wvec == $wsize ) or die "Invalid sweight vector length\n";
    $this->{SWEIGHTS}->{$wname}->{VECTOR} = [@wvec];

    return $lin;
}


#================================================================

#  parsemix

#  Loads a mixture component pdf

#--------------------------------
sub parsemix
{
    my ( $this, $fh, $lin, $pname ) = @_;
    my ( @elem, $macro, $mname, $vtype, $vname );
    my ( $uniqct );

    $pname or die "No mixture component name\n";

    if(  exists $this->{MIXPDF}->{$pname} )
    {
	die "Duplicate mixture component name $pname\n";
    }

    # initialize usage of MIXPDF macro
    $this->{MIXPDF}->{$pname}->{USAGE} = 0;

    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~m/ )
    {
	$lin = <$fh>;
    }

  LINE:
    while( $lin )
    {

	chomp($lin);
	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # regression class tag
	    if( $elem[0] =~ /RCLASS/i )
	    {
		$this->{MIXPDF}->{$pname}->{RCLASS} = $elem[1];
		@elem = @elem[ 2 .. $#elem ];
	    }

	    # gconst tag
	    elsif( $elem[0] =~ /GCONST/i )
	    {
		$this->{MIXPDF}->{$pname}->{GCONST} = $elem[1];
		@elem = @elem[ 2 .. $#elem ];
	    }

	    # mean macro called in mixpdf
	    elsif( $elem[0] =~ /~u/ )
	    {
		($macro) = $elem[1] =~ /\"(.*)\"/;
		$this->{MIXPDF}->{$pname}->{MEAN} = $macro;

		@elem = @elem[ 2 .. $#elem ];
	    }

	    # variance macro called in mixpdf
	    elsif( $elem[0] =~ /~[vi]/ )
	    {
		($macro) = $elem[1] =~ /\"(.*)\"/;
		$this->{MIXPDF}->{$pname}->{VARIANCE} = $macro;
		
		@elem = @elem[ 2 .. $#elem ];
	    }
		
	    # mean defined in mixpdf, make a macro
	    elsif( $elem[0] =~ /MEAN/i )
	    {
		$mname = "${pname}_MEAN";

		# ensure the macro name is unique, since mean explicitly defined
		$uniqct = 0;
		while( exists $this->{MEAN}->{$mname} )
		{
		    $uniqct++;
		    $mname = "${pname}_MEAN_D$uniqct";
		}

		$lin = parsemean( $this, $fh, $lin, $mname );
		$this->{MIXPDF}->{$pname}->{MEAN} = $mname;
		
		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }

	    # variance defined in mixpdf, make a macro
	    elsif( $elem[0] =~ /VARIANCE|INVCOVAR/i )
	    {
		$vtype = ( $elem[0] =~ /VARIANCE/i ) ? "v" : "i";
		$vname = "${pname}_VARIANCE";

		# ensure the macro name is unique, since variance explicitly defined
		$uniqct = 0;
		while(  exists $this->{VARIANCE}->{$vname} )
		{
		    $uniqct++;
		    $vname = "${pname}_VARIANCE_D$uniqct";
		}

		$lin = parsecov( $this, $fh, $lin, $vtype, $vname );
		$this->{MIXPDF}->{$pname}->{VARIANCE} = $vname;

		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;

	    }		

	    # no element to parse
	    else
	    {
		last LINE;
	    }

	}

	# get next line to process
	$lin = <$fh>;
    }

    return $lin;
}


#================================================================

#  parsestate

#  Loads a state structure

#--------------------------------
sub parsestate
{
    my ( $this, $fh, $lin, $sname ) = @_;
    my ( $stream, $nstream, $i, @nummix, $mixture, $mixprob );
    my ( @elem, $wname, $pname );

    $sname or die "No state name\n";

    if(  exists $this->{STATE}->{$sname} )
    {
	die "Duplicate state name $sname\n";
    }

    # initialize this state macro usage
    $this->{STATE}->{$sname}->{USAGE} = 0;


    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~s/ )
    {
	$lin = <$fh>;
    }

    #default stream list element number (0 based)
    $stream = 0;

    # number of streams (1 based)
    $nstream = $this->{STREAMINFO}->{SIZE};
    unless( $nstream ) { $nstream = 1 };

    #default of one mixture per stream
    for( $i=0; $i< $nstream; $i++ ) {$nummix[$i] = 1;}

    $this->{STATE}->{$sname}->{NUMMIXES} = [@nummix];

    #default mixture number ( 0 based ) and prob
    $mixture = 0;
    $mixprob = 1;

  LINE:
    while( $lin )
    {
	chomp($lin);

	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # nummixes tag
	    if( $elem[0] =~ /NUMMIXES/i )
	    {
		@nummix = @elem[ 1 .. $nstream];
		$this->{STATE}->{$sname}->{NUMMIXES} = [@nummix];

		@elem = @elem[ $nstream+1 .. $#elem ];
	    }

	    # stream weights tag defined in state
	    elsif( $elem[0] =~ /SWEIGHTS/i )
	    {
		$wname = "${sname}_SWEIGHTS";
		$lin = parsewgts( $this, $fh, $lin, $wname );

		$this->{STATE}->{$sname}->{SWEIGHTS} = $wname;
		
		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }

	    # stream weight macro called in state
	    elsif( $elem[0] =~ /~w/ )
	    {
		($wname) = $elem[1] =~ /\"(.*)\"/;
		$this->{STATE}->{$sname}->{SWEIGHTS} = $wname;
		
		@elem = @elem[ 2 .. $#elem];
	    }		    

	    #stream tag, number used when mixture defined
	    # initialize mixture counter to 0 for next mixture
	    elsif( $elem[0] =~ /^STREAM$/i )
	    {
		$stream = $elem[1] - 1;
		($stream >=0 && $stream < $nstream ) or
		    die "Invalid stream number\n";

		$mixture = 0;
		$mixprob = 1;

		@elem = @elem[ 2 .. $#elem];
	    }
	    
	    #mixtures tag, number used when mixture defined (0 based)
	    elsif( $elem[0] =~ /^MIXTURE$/i )
	    {
		$mixture = $elem[1] - 1;
		( $mixture >=0 && $mixture < $nummix[$stream] )
		    or die "Invalid mixture number\n";
		
		$mixprob = $elem[2];
		    
		@elem = @elem[ 3 .. $#elem];
	    }		    

	    #mixpdf macro defines mixpdf
	    elsif( $elem[0] =~ /~m/ )
	    {
		($pname) = $elem[1] =~ /\"(.*)\"/;
		$this->{STATE}{$sname}{STREAMMIX}[$stream][$mixture]{MIXPDF} = 
		    $pname;
		$this->{STATE}{$sname}{STREAMMIX}[$stream][$mixture]{PROB} = 
		    $mixprob;

		@elem = @elem[ 2 .. $#elem];
	    }		    
	    
	    #detection of start of mixture pdf component - parse it
	    elsif( $elem[0] =~ /~[uvi]/ ||
		$elem[0] =~ /RCLASS|MEAN|VARIANCE|INVCOVAR|GCONST/i )
	    {
		$pname = "${sname}_MIXPDF_" . ($stream+1) . "_" . ($mixture+1);
		$lin = parsemix( $this, $fh, $lin, $pname );
		
		$this->{STATE}{$sname}{STREAMMIX}[$stream][$mixture]{MIXPDF} = 
		    $pname;
		$this->{STATE}{$sname}{STREAMMIX}[$stream][$mixture]{PROB} = 
		    $mixprob;

		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }

	    # no tag or macro for mixpdf
	    else
	    {
		last LINE;
	    }
	}
    
	# get next line to process
	$lin = <$fh>;
    }


    # Check that all streams have the right number of mixtures;
    for( $stream = 0; $stream < $nstream; $stream++ )
    {
	my $nmix = $this->{STATE}{$sname}{NUMMIXES}[$stream];
	foreach my $mix ( 0 .. $nmix-1 )
	{
	    unless( defined $this->{STATE}{$sname}{STREAMMIX}[$stream][$mix] )
	    {
		print STDERR "Missing mixture for $sname, stream $stream, mix $mix\n";
		last;
	    }
	}
    }

    return $lin;
}


#================================================================

#  parsehmm

#  Loads an hmm structure

#--------------------------------
sub parsehmm
{
    my ( $this, $fh, $lin, $hname ) = @_;
    my ( @elem, $sname, $state, $snum, $tname );
    my ( $uniqct );

    $hname or die "No hmm name\n";

    if(  exists $this->{HMM}->{$hname} )
    {
	die "Duplicate hmm name $hname\n";
    }
    
    # get first line past macro header if on macro line
    if( $lin =~ /^\s*~h/ )
    {
	$lin = <$fh>;
    }
    
    # check starting tag
    ( $lin =~ /<BEGINHMM>$/i ) or die "Invalid hmm start\n";

    $lin = <$fh>;
    
    
  LINE:
    while( $lin )
    {
	chomp($lin);

	#ignore blank line
	if( $lin =~ /^\s*$/ )
	{
	    $lin = <$fh>;
	    next;
	}

	@elem = split( /[\s<>]+/, $lin);
	( $elem[0] ne '' ) or shift @elem;
	
	while( @elem )
	{
	    # global options macro within hmm
	    if( $elem[0] =~ /~o/ )
	    {
		$lin = parseopt( $this, $fh, $lin );

		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }
	    
	    # number of states, including non-emitting start and end
	    elsif( $elem[0] =~ /NUMSTATES/i )
	    {
		$this->{HMM}->{$hname}->{NUMSTATES} = $elem[1];

		@elem = @elem[ 2 ..$#elem];
	    }
	    
	    # state number, converted to 0 based here for index
	    elsif( $elem[0] =~ /STATE/i )
	    {
		# 0 based state list 
		$state = $elem[1] - 2;
		
		@elem = @elem[ 2 ..$#elem];

		# strip state from line if necessary 
		# since state may appear with other elements
		if( $#elem )
		{
		    $lin =~ s/\s*<STATE>\s+[0-9]+\s*//i;
		}
	    }
	    
	    # state is defined by macro
	    elsif( $elem[0] =~ /~s/ )
	    {
		($sname) = $elem[1] =~ /\"(.*)\"/;
		$this->{HMM}->{$hname}->{STATE}[$state] = $sname;

		@elem = @elem[ 2 .. $#elem ];
	    }

	    # state is internally defined within hmm, make a macro
	    elsif( $elem[0] =~ /NUMMIXES|SWEIGHTS|STREAM|MIXTURE|RCLASS/i ||
		   $elem[0] =~ /MEAN|VARIANCE|INVCOVAR|GCONST/i ||
		   $elem[0] =~ /~[wmuvi]/ )
	    {
		$snum = $state + 2;
		$sname = "HMM_${hname}_STATE_${snum}";

		# ensure sname unique, since explicitly defined
		$uniqct = 0;
		while( exists $this->{STATE}->{$sname} )
		{
		    $uniqct++;
		    $sname = "HMM_${hname}_STATE_${snum}_D$uniqct";
		}		    

		$lin = parsestate( $this, $fh, $lin, $sname );
		$this->{HMM}->{$hname}->{STATE}[$state] = $sname;

		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }
	    
	    # transition matrix defined by macro
	    elsif( $elem[0] =~ /~t/ )
	    {
		($tname) = $elem[1] =~ /\"(.*)\"/;
		$this->{HMM}->{$hname}->{TRANSP} = $tname;

		@elem = @elem[ 2 .. $#elem ];
	    }

	    
	    # transition matrix defined internal to hmm, make a macro
	    elsif( $elem[0] =~ /TRANSP/i )
	    {
		$tname = "HMM_${hname}_TRANSP";

		# ensure name is unique, since trans explicitly defined
		$uniqct = 0;
		while( exists $this->{TRANSP}->{$tname} )
		{
		    $uniqct++;
		    $tname = "HMM_${hname}_TRANSP_D$uniqct";
		}

		$lin = parsetrans( $this, $fh, $lin, $tname );
		$this->{HMM}->{$hname}->{TRANSP} = $tname;

		@elem = split( /[\s<>]+/, $lin);
		( $elem[0] ne '' ) or shift @elem;
	    }		

	    # end of hmm definition
	    elsif( $elem[0] =~ /ENDHMM/i )
	    {
		# read past end of hmm
		$lin = <$fh>;
		last LINE;
	    }

	    # no hmm tag or macro
	    else
	    {
		last LINE;
	    }
	}
	
	# get next line to process
	$lin = <$fh>;
    }

    return $lin;
}

#================================================================
# checklist

# Checks to ensure that all hmms in a list have been loaded.

#-----------------------------
sub checklist
{
    my ($this, @hmmlist ) = @_;
    my(  $hname, $doabort );

    if( @hmmlist ) 
    { 
	$doabort = 0;
	foreach $hname ( @hmmlist )
	{
	    if( ! exists $this->{HMM}{$hname} )
	    {
		$doabort = 1;
		print "HMM '$hname' not in mmf files\n";
	    }
	}
	exit 0 if $doabort;
    }
}

#================================================================

# markusagehmm

# Marks the usage of all substructures of an hmm.  Also does checking
# to see if the hmm structure is consistent.

#--------------------------------

sub markusagehmm
{
    my( $this, $mexpand ) = @_;
    my( $hname, $tname, $sname, $wname, $mname, $vname, $pname );
    my( $hmmref, $stateref, $streamref, $mixref, $pdfref );
    my( $trnref );
    my( $vref, $mref );
    my( $streamct );
    my( $regref, $rclass );


    while( ($hname, $hmmref) = each %{ $this->{HMM} } )
    {
	#mark transition matrix as used
	$tname = $hmmref->{TRANSP};
	$trnref =  $this->{TRANSP}{$tname};

	#check for consistency 
	( $hmmref->{NUMSTATES} == $trnref->{SIZE} ) or 
	    die "$hname has wrong transition size in $tname\n";

	# mark transition as used

#	( $mexpand =~ /t/ ) ?  $this->{TRANSP}{$tname}{USAGE}-- :
#	    $this->{TRANSP}{$tname}{USAGE}++;

	( $mexpand =~ /t/ ) ?  $trnref->{USAGE}-- : $trnref->{USAGE}++;


	#mark each state as used
	foreach $sname ( @{ $hmmref->{STATE} } )
	{
	    # mark state as used
	    $stateref = $this->{STATE}{$sname};

	    ( $mexpand =~ /s/ ) ?  $stateref->{USAGE}-- :  $stateref->{USAGE}++;

	    # mark state stream weights as used
	    if( exists $stateref->{SWEIGHTS} )
	    {
		$wname = $stateref->{SWEIGHTS};
		( $mexpand =~ /w/ ) ?  $this->{SWEIGHTS}{$wname}{USAGE}-- :
		    $this->{SWEIGHTS}{$wname}{USAGE}++;
	    }

	    # mark each mixpdf and its components as used
	    # associate a stream with each pdf, mean, and variance
	    $streamct = 0;
	    foreach $streamref ( @{ $stateref->{STREAMMIX} } )
	    {
		foreach $mixref ( @{ $streamref } )
		{
		    $pname = $mixref->{MIXPDF};
		    $pdfref = $this->{MIXPDF}{$pname};
		    ( $mexpand =~ /m/ ) ? $pdfref->{USAGE}-- :  $pdfref->{USAGE}++;
		    if( exists $pdfref->{STREAM} && $pdfref->{STREAM} != $streamct )
		    { 
			die '$pname crosses streams';
		    }
		    else
		    {
			$pdfref->{STREAM} = $streamct;
		    }

		    # if pdf uses a regression class, then regression trees are used
		    if( $pdfref->{RCLASS} && exists $this->{REGTREE} )
		    {
			$rclass = $pdfref->{RCLASS};
			foreach $regref ( values %{ $this->{REGTREE} } )
			{			    
			    $regref->{USAGE}++;
			    
			    # keep track of number of pdfs mapping to each leaf
			    die "Invalid pdf RCLASS for $pname\n" if( ! defined $regref->{TNODE}[$rclass] );
			    
			    $regref->{TNODE}[$rclass]++;
			}
		    }


		    $vname = $pdfref->{VARIANCE};
		    $vref = $this->{VARIANCE}{$vname};
		    ( $mexpand =~ /v|i/ ) ? $vref->{USAGE}-- : 
			$vref->{USAGE}++ ;
		    if( exists $vref->{STREAM} && $vref->{STREAM} != $streamct )
		    {
			die '$vname crosses streams';
		    }
		    else
		    {
			$vref->{STREAM} = $streamct;
		    }

		    $mname = $pdfref->{MEAN};
		    $mref = $this->{MEAN}{$mname};
		    ( $mexpand =~ /u/ ) ?  $mref->{USAGE}-- :
			$mref->{USAGE}++;
		    if( exists $mref->{STREAM} && $mref->{STREAM} != $streamct )
		    {
			die '$mname crosses streams';
		    }
		    else
		    {
			$mref->{STREAM} = $streamct;
		    }
		}
		$streamct++;
	    }
	}
    }
}

#================================================================

#  transformpdfs

#  This function transforms all pdfs that are marked for usage
#  using a TMF object as defined in HTKtmf.

#--------------------------------
sub transformpdfs
{
    my( $this, $tmf ) = @_;
    my( $pdfname, $pref, $class );
    my( $meanname, $mref, $mvecref );
    my( $varname, $vref, $vvecref );
    my( $gconstref );

    foreach $pdfname ( keys %{ $this->{MIXPDF} } )
    {
	$pref = $this->{MIXPDF}{$pdfname};

	next if( $pref->{USAGE} == 0 || ! exists $pref->{RCLASS} );

	$class = $pref->{RCLASS};

	# determine if this pdf mean vector needs to be transformed
	$meanname = $pref->{MEAN};
	$mref = $this->{MEAN}{$meanname};
	if( exists $mref->{TMFRCLASS} )
	{
	    die "HTKdata: cross rclass mean $meanname\n" if( $mref->{TMFRCLASS} != $class );

	    # mean already transformed
	    $mvecref = undef;
	}
	else
	{
	    # need to transform the mean via this regression class
	    $mref->{TMFRCLASS} = $class;
	    $mvecref = $mref->{VECTOR};
	}

	# determine if this pdf variance vector needs to be transformed
	$varname = $pref->{VARIANCE};
	$vref = $this->{VARIANCE}{$varname};
	if( exists $vref->{TMFRCLASS} )
	{
	    die "HTKdata: cross rclass var $varname\n" if( $vref->{TMFRCLASS} != $class );

	    # variance already transformed
	    $vvecref = undef;
	}
	else
	{
	    # need to transform the variance via this regression class if it is diagonal vector
	    $vref->{TMFRCLASS} = $class if( $vref->{TYPE} eq 'v' );
	    $vvecref = ( $vref->{TYPE} eq 'v' )  ?  $vref->{VECTOR}  :  undef;
	}

	if( exists $pref->{GCONST} )
	{
	    $gconstref = \$pref->{GCONST};
	}


	$tmf->transformpdf( $class, $mvecref, $vvecref, $gconstref );
    }
}


#================================================================

# compressedmmf

# Output HTK MMF file containing all and only data for loaded
# hmm set

#--------------------------------
sub compressedmmf
{

    my( $this, $fh ) = @_;
    my( $haveopt, $ref, $opt );
    my( $rname, $rref );
    my( $tname, $tref, $ntrans, $row );
    my( $wname, $wref );
    my( $mname, $mref );
    my( $vname, $vref, $vtype, $elem );
    my( $pname, $pref );
    my( $sname, $sref, $nstream, @nummixes, $stream, $nmix, $mixref, $mix, $prob );
    my( $hname, $href, $state, $nstates );


    #output global options macro info - it should exist

    $haveopt = 0;
    if( exists $this->{STREAMINFO} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	$ref = $this->{STREAMINFO};
	print $fh "<STREAMINFO> ", $ref->{SIZE}, " ", join( " ", @{ $ref->{VECTOR} }), "\n";
    }

    if( exists $this->{VECSIZE} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	print $fh "<VECSIZE> ", $this->{VECSIZE}, "\n";
    }

    if( exists $this->{HMMSETID} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	print $fh "<HMMSETID> ", $this->{HMMSETID}, "\n";
    }

    if( exists $this->{DURKIND} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	$opt = $this->{DURKIND};
	print $fh "<$opt>\n";
    }

    if( exists $this->{COVKIND} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	$opt = $this->{COVKIND};
	print $fh "<$opt>\n";
    }

    if( exists $this->{PARMKIND} )
    {
	print $fh "~o\n" unless $haveopt;
	$haveopt = 1;

	$opt = $this->{PARMKIND};
	print $fh "<$opt>\n";
    }

    # output all regression tree macros if used
    foreach $rname( sort keys %{ $this->{REGTREE} } )
    {
	$rref = $this->{REGTREE}{$rname};
	if( $rref->{USAGE} > 0 )
	{
	    print $fh "~r \"$rname\"\n";
	    output_reg( $rref, $fh );
	}
    }

    #output all transition  matrix macros used
    foreach $tname   ( sort keys %{ $this->{TRANSP} } )
    {
	$tref = $this->{TRANSP}{$tname};

	if( $tref->{USAGE} > 0 )
	{
	    print $fh "~t \"$tname\"\n";
	    output_trans( $tref, $fh );
	}
    }
    
    # output all stream weight macros used
    foreach $wname ( sort keys  %{ $this->{SWEIGHTS} } )
    {
	$wref = $this->{SWEIGHTS}{$wname};

	if( $wref->{USAGE} > 0 )
	{
	    print $fh "~w \"$wname\"\n";
	    output_swgt( $wref, $fh );
	}
    }

    # output all mean vector macros used
    foreach $mname ( sort keys  %{ $this->{MEAN} } )
    {
	$mref = $this->{MEAN}{$mname};

	if( $mref->{USAGE} > 0 )
	{
	    print $fh "~u \"$mname\"\n";
	    output_mean( $mref, $fh );
	}
    }
    

    # output all variance vector or matrix macros used
    foreach $vname ( sort keys  %{ $this->{VARIANCE} } )
    {

	$vref = $this->{VARIANCE}{$vname};

	if( $vref->{USAGE} > 0 )
	{
	    $vtype = $vref->{TYPE};
	    print $fh "~$vtype \"$vname\"\n";
	    output_var( $vref, $fh );
	}
    }


    #output all pdf's used
    foreach $pname ( sort keys  %{ $this->{MIXPDF} } )
    {
	$pref = $this->{MIXPDF}{$pname};

	if( $pref->{USAGE} > 0 )
	{
	    print $fh "~m \"$pname\"\n";
	    output_mix( $this, $pref, $fh );
	}
    }


    # output all state macros used
    foreach $sname ( sort keys  %{ $this->{STATE} } )

    {
	$sref =  $this->{STATE}{$sname};

	if( $sref->{USAGE} > 0  )
	{
	    print $fh "~s \"$sname\"\n";

	    output_state( $this, $sref, $fh );
	}
    }


    #output all hmms
    foreach $hname ( sort keys  %{ $this->{HMM} } )
    {
	$href =  $this->{HMM}{$hname};

	print $fh "~h \"$hname\"\n";
	print $fh "<BEGINHMM>\n";

	$nstates = $href->{NUMSTATES};
	print $fh "<NUMSTATES> ${nstates}\n";

	for( $state = 0; $state < $nstates-2; $state++ )
	{
	    $sname = $href->{STATE}[$state];
	    print $fh "<STATE> ", $state+2, "\n";

	    $sref = $this->{STATE}{$sname};
	    if( $sref->{USAGE} > 0 )
	    {
		print $fh "~s \"${sname}\"\n";
	    }
	    else
	    {
		output_state( $this, $sref, $fh );
	    }
	}

	$tname = $href->{TRANSP};
	$tref = $this->{TRANSP}{$tname};
	if( $tref->{USAGE} > 0 )
	{
	    print $fh "~t \"$tname\"\n";
	}
	else
	{
	    output_trans( $tref, $fh );
	}

	print $fh "<ENDHMM>\n";
    }
}


#================================================================

# output_reg

# output regression tree information

#--------------------------------
sub output_reg
{
    my( $rref, $fh ) = @_;
    my( $lastnode, $nd );

    print $fh  "<REGTREE> $rref->{LEAFS}\n";

    $lastnode = $#{ $rref->{NODE} };
    for( $nd=0; $nd <= $lastnode; $nd++ )
    {
	if( defined $rref->{NODE}[$nd] )
	{
	    print $fh "<NODE> $nd @{$rref->{NODE}[$nd]}\n";
	}
    }

    $lastnode = $#{ $rref->{TNODE} };
    for( $nd=0; $nd <= $lastnode; $nd++ )
    {
	if( defined $rref->{TNODE}[$nd] )
	{
	    print $fh  "<TNODE> $nd $rref->{TNODE}[$nd]\n";
	}
    }
}

#================================================================

# output_trans

# output transition matrix

#--------------------------------

sub output_trans
{
    my( $tref, $fh ) = @_;
    my( $ntrans, $row );

    $ntrans = $tref->{SIZE};
    print $fh "<TRANSP> ", $ntrans, "\n";
    for($row=0; $row < $ntrans; $row++ )
    {
	print $fh join( " ", @{ $tref->{MATRIX}[$row] } ), "\n";
    }

    return;
}


#================================================================

# output_swgt

# output stream weight vector

#--------------------------------

sub output_swgt
{
    my( $wref, $fh ) = @_;

    print $fh "<SWEIGHTS> ", $wref->{SIZE}, " ";
    print $fh join( " ", @{ $wref->{VECTOR} } ), "\n";
    
    return;
}


#================================================================

# output_mean

# output mean vector

#--------------------------------

sub output_mean
{
    my( $mref, $fh ) = @_;

    print $fh "<MEAN> ", $mref->{SIZE}, "\n";
    print $fh join( " ", @{ $mref->{VECTOR} } ), "\n";
    
    return;
}


#================================================================

# output_var

# output variance or covariance data

#--------------------------------
sub output_var
{
    my( $vref, $fh ) = @_;
    my( $vtype, $elem, $row );

    $vtype = $vref->{TYPE};
    if( $vtype =~ /v/ )
    {
	print $fh "<VARIANCE> ", $vref->{SIZE}, "\n";
	print $fh join( " ", @{ $vref->{VECTOR} } ), "\n";
    }
    else
    {
	# type assumed "i"
	print $fh "<INVCOVAR> ", $vref->{SIZE}, "\n";
	$elem = 0;
	for( $row=$vref->{SIZE}; $row> 0; $row-- )
	{
	    print $fh join( " ", @{ $vref->{VECTOR} }[$elem .. $elem+$row - 1 ] ), "\n";
	    $elem += $row;
	}
    }

    return;
}



#================================================================

# output_mix

# output mixture data

#--------------------------------
sub output_mix
{
    my( $this, $pref, $fh ) = @_;
    my( $mname, $mref );
    my( $vname, $vref, $vtype );

    if( exists $pref->{RCLASS} )
    {
	print $fh "<RCLASS> ", $pref->{RCLASS}, "\n";
    }

    $mname = $pref->{MEAN};
    $mref = $this->{MEAN}{$mname};
    if( $mref->{USAGE} > 0 )
    {
	print $fh "~u \"$mname\"\n";
    }
    else
    {
	# must be used since ~m used, and thus must need to be expanded inline
	output_mean( $mref, $fh );
    }		

    $vname = $pref->{VARIANCE};
    $vref = $this->{VARIANCE}{$vname};
    $vtype = $vref->{TYPE};
    if(  $vref->{USAGE} > 0 )
    {
	print $fh "~$vtype \"$vname\"\n";
    }
    else
    {
	output_var( $vref, $fh );
    }

    if( exists $pref->{GCONST} )
    {
	print $fh "<GCONST> ", $pref->{GCONST}, "\n";
    }

    return;
}


#================================================================

# output_state

# output state data

#--------------------------------
sub output_state
{
    my( $this, $sref, $fh ) = @_;
    my( @nummixes, $nstream, $stream );
    my( $wname, $wref );
    my( $nmix, $mixref, $mix, $pname, $pref, $prob );

    @nummixes = @{ $sref->{NUMMIXES} };
    @nummixes or @nummixes = (1);

    if( @nummixes > 1 || $nummixes[0] > 1 )
    {
	print $fh "<NUMMIXES> ", join(" ", @nummixes), "\n";
    }		

    if( exists $sref->{SWEIGHTS} )
    {
	$wname = $sref->{SWEIGHTS};
	$wref = $this->{SWEIGHTS}{$wname};

	if( $wref->{USAGE} > 0 )
	{
	    print $fh "~w \"$wname\"\n";
	}
	else
	{
	    output_swgt( $wref, $fh );
	}
    }

    $stream = 0;
    $nstream = @nummixes;

    for( $stream=0; $stream < $nstream; $stream++ )
    {
	if( $nstream > 1 )
	{
	    print $fh "<STREAM> ", $stream+1, "\n";
	}
	    
	$nmix = $nummixes[$stream];
	
	for( $mix = 0; $mix < $nmix; $mix++ )
	{
	    $mixref = $sref->{STREAMMIX}[$stream][$mix];
	    
	    $pname = $mixref->{MIXPDF};
	    $prob = $mixref->{PROB};
	    $pref = $this->{MIXPDF}{$pname};

	    if( $nmix > 1 )
	    {
		print $fh "<MIXTURE> ", $mix+1, " ", $prob, "\n";
	    }

	    if( $pref->{USAGE} > 0 )
	    {
		print $fh "~m \"$pname\"\n";
	    }
	    else
	    {
		output_mix( $this, $pref, $fh );
	    }
	}
    }

    return;
}


#================================================================
# mergehmms
    
# Merge a set of HMMs to produce a single hmm
    
#--------------------------------
sub mergehmms
{
    my( $this, $mrgfile ) = @_;
    my( $firsthmm, $hname, $hbname, $hbref, $href );
    my( $tbref, $tref, $row, $col );
    my( $tbname, $tbmname, $tbmref, $tfact );
    my( $nstream, $state, $sbname, $sbref, $sname, $sref, $sfact );
    my( $stream, $nbmix, $bmix, $nmix, $mix );
    my( $lin, $snum, @hmmlist, %base );
    my( $sbmname, $sbmref );
    
    
    # number of data streams
    $nstream = $this->{STREAMINFO}{SIZE};

    # process all lines of merge file
    open( MRG, "$mrgfile" ) or die "Can not open merge file $mrgfile\n";

    while( $lin = <MRG> )
    {
	chomp($lin);
	@hmmlist = split( " ", $lin );
	

	# first hmm on the line is the base hmm to merge into
	$firsthmm  = 1;
    
      HMM:
	foreach $hname ( @hmmlist )
	{
	    if( $firsthmm )
	    {
		# initialize base hmm
		exists $this->{HMM}{$hname} or die "HMM $hname does not exist\n";
		$hbref = $this->{HMM}{$hname};
		$hbname = $hname;
		
		# keep track of base models to ensure no base to base merge
		$base{$hbname} = 1;

		# initialize if base model has not been used for merging
		unless( exists $hbref->{MERGE}{NAME}{$hbname} )
		{
		    $hbref->{MERGE}{NAME}{$hbname} = 1;
		    $hbref->{MERGE}{COUNT} = 1;
		}

		$firsthmm = 0;
		next HMM;
	    }
	    else
	    {
		unless( exists $this->{HMM}{$hname} )
		{
		    print STDERR "HMM $hname does not exist, not merged with $hbname\n";
		    next HMM;
		}		

		if( exists $base{$hname} )
		{
		    die "Attempt to merge base model $hname into $hbname\n";
		}

		if( exists $hbref->{MERGE}{NAME}{$hname} )
		{
		    print STDERR "HMM $hname multiple merge for $hbname, not merged\n";
		    next HMM;
		}
		else
		{
		    # add this hmm to the hmms merged with base
		    $hbref->{MERGE}{NAME}{$hname} = ++$hbref->{MERGE}{COUNT};
		}
	    }	    
	    
	    # reference to data for hmm to merge with base hmm
	    $href = $this->{HMM}{$hname};

	    # hmms to merge must have same number of states
	    ($hbref->{NUMSTATES} == $href->{NUMSTATES} ) or
		die "HMM state count conflict $hbname vs $hname\n";
	    

	    # transition probability merging

	    # check for first hmm $href using a different transition matrix than original $hbref
	    # this signals that the original transition matrix can not be used, (that is, all
	    # models being merged do not use the same transition matrix) and so 
	    # a new transition macro structure must be created for this base hmm
	    if( ! exists $hbref->{MERGE}{NEWTRANSP} &&
		($href->{TRANSP} ne $hbref->{TRANSP} ) )
	    {

		# flag new transition macro created for this base hmm
		$hbref->{MERGE}{NEWTRANSP} = 1;

		# create new transp macro with unique name ($tbmname)
		$tbname = $hbref->{TRANSP};
		$tbref = $this->{TRANSP}{ $tbname };
		$tbmname = "HMM_${hbname}_TRANSP_${tbname}_MRG";

		# note: must have this line to declare reference before defining $tbmref
		$this->{TRANSP}{$tbmname}{USAGE} = 0;

		# copy transition data to new macro
		$tbmref = $this->{TRANSP}{$tbmname};
		$tbmref->{SIZE} = $tbref->{SIZE};
		for( $row = 0; $row < $hbref->{NUMSTATES}; $row++ )
		{
		    for( $col = 0; $col < $href->{NUMSTATES}; $col++ )
		    {
			$tbmref->{MATRIX}[$row][$col] = $tbref->{MATRIX}[$row][$col];
		    }
		}
		# base hmm uses new transition macro
		$hbref->{TRANSP} = $tbmname;
	    }
	    

	    # must do merge of transition data if new base transition matrix defined
	    if( exists $hbref->{MERGE}{NEWTRANSP} )
	    {

		$tbref = $this->{TRANSP}{ $hbref->{TRANSP} };
		$tref = $this->{TRANSP}{ $href->{TRANSP} };
		$tfact = 1/( $hbref->{MERGE}{COUNT} );
	    
		for( $row = 0; $row < $href->{NUMSTATES}; $row++ )
		{
		    for( $col = 0; $col < $href->{NUMSTATES}; $col++ )
		    {
			$tbref->{MATRIX}[$row][$col] = (1-$tfact)*($tbref->{MATRIX}[$row][$col]) +
			    $tfact*($tref->{MATRIX}[$row][$col]);
		    }
		}
	    }	    


	    # state merging

	  STATE:
	    for( $state=0; $state < ($hbref->{NUMSTATES} - 2); $state++ )
	    {
		# base model state
		$sbname = $hbref->{STATE}[$state];
		$sbref = $this->{STATE}{$sbname};

		# construct new merged state macro for the base state if first hmm to merge
		if( $hbref->{MERGE}{COUNT} == 2 )
		{
		    $snum = $state+2;
		    $sbmname = "HMM_${hbname}_STATE_${snum}_MRG";
		    
		    # note: also declares new state name reference
		    $this->{STATE}{$sbmname}{USAGE} = 0;

		    #copy base state info to new state macro
		    $sbmref = $this->{STATE}{$sbmname};
		    $sbmref->{NUMMIXES} = [ @{ $sbref->{NUMMIXES} } ];
		    if( exists $sbref->{SWEIGHTS} )
		    {
			$sbmref->{SWEIGHTS} = $sbref->{SWEIGHTS}
		    }

		    for( $stream = 0; $stream < $nstream; $stream++ )
		    {
			$nmix = $sbref->{NUMMIXES}[$stream];
			for( $mix = 0; $mix < $nmix; $mix++ )
			{
			    $sbmref->{STREAMMIX}[$stream][$mix]{PROB} = 
				$sbref->{STREAMMIX}[$stream][$mix]{PROB};
			    
			    $sbmref->{STREAMMIX}[$stream][$mix]{MIXPDF} = 
				$sbref->{STREAMMIX}[$stream][$mix]{MIXPDF};
			}
		    }
		    
		    # make base hmm model use new merged state macro
		    $hbref->{STATE}[$state] = $sbmname;
		    $sbref = $sbmref;

		    # initialize merge statistics
		    $sbref->{MERGE}{NAME}{$sbname} = 1;
		    $sbref->{MERGE}{COUNT} = 1;
		    $sbname = $sbmname;
		}

		# model to merge with base
		$sname = $href->{STATE}[$state];
		$sref = $this->{STATE}{$sname};
		

		$sbref->{MERGE}{NAME}{$sname} = ++$sbref->{MERGE}{COUNT};
		$sfact = 1/( $sbref->{MERGE}{COUNT} );
	    
		for( $stream=0; $stream < $nstream; $stream++ )
		{
		    # precondition the mixture pdf probabilities in the base state
		    $nbmix = $sbref->{NUMMIXES}[$stream];
		    for( $bmix = 0; $bmix < $nbmix; $bmix++ )
		    {
			$sbref->{STREAMMIX}[$stream][$bmix]{PROB} *= (1-$sfact);
		    }

		    # add mixture information from $sref to the base state
		    $nmix = $sref->{NUMMIXES}[$stream];
		  MIX:
		    for( $mix = 0; $mix < $nmix; $mix++ )
		    {
			# check if mixture pdf already is in base state
			# if so, just adjust the mixture probability
			$nbmix = $sbref->{NUMMIXES}[$stream];
			for( $bmix = 0; $bmix < $nbmix; $bmix++ )
			{
			    if( $sbref->{STREAMMIX}[$stream][$bmix]{MIXPDF} eq
				$sref->{STREAMMIX}[$stream][$mix]{MIXPDF} )
			    {
				$sbref->{STREAMMIX}[$stream][$bmix]{PROB} = 
				    ($sbref->{STREAMMIX}[$stream][$bmix]{PROB} ) +
					$sfact*( $sref->{STREAMMIX}[$stream][$mix]{PROB} );
				next MIX;
			    }
			}

			# if you get to here, then this is a new mixture pdf not in the base state
			# for this stream - add new pdf to base state
			$sbref->{STREAMMIX}[$stream][$nbmix]{MIXPDF} = 
			    $sref->{STREAMMIX}[$stream][$mix]{MIXPDF};

			$sbref->{STREAMMIX}[$stream][$nbmix]{PROB} = 
				$sfact*( $sref->{STREAMMIX}[$stream][$mix]{PROB} );
			
			$sbref->{NUMMIXES}[$stream]++;
		    }
		}
	    }
	    
	    # remove the HMM just merged with the base hmm
	    delete $this->{HMM}{$hname};
	}
    }
    
    close( MRG );


    # clean up - remove base hmm name hash and  merge information
    undef %base;

    while( ($hname, $href) = each %{ $this->{HMM} } )
    {
	delete $href->{MERGE} if exists $href->{MERGE};
    }

    while( ($sname, $sref) = each %{ $this->{STATE} } )
    {
	delete $sref->{MERGE} if exists $sref->{MERGE};
    }
    
    return;
}


#================================================================
# mergestates
    
# Merge a set of states to produce single state and mark HMMs accordingly
    
# Note: the stream weighting is not merged - the stream weighting of the
# base state will be the stream weighting of the merged state

#--------------------------------
sub mergestates
{
    my( $this, $mrgfile ) = @_;
    my( $firststate, $sname, $sbname, $sbref, $sref );
    my( $nstream, $state, $nstates, $sfact );
    my( $stream, $nbmix, $bmix, $nmix, $mix );
    my( $lin, $snum, @statelist, %base, %map );
    my( $hname, $href );
    
    
    # number of data streams
    $nstream = $this->{STREAMINFO}{SIZE};

    # process all lines of merge file
    open( MRG, "$mrgfile" ) or die "Can not open merge file $mrgfile\n";

    while( $lin = <MRG> )
    {
	chomp($lin);
	@statelist = split( " ", $lin );
	

	# first state on the line is the base state to merge into
	$firststate  = 1;
    
      STATE:
	foreach $sname ( @statelist )
	{
	    if( $firststate )
	    {
		# initialize base state
		exists $this->{STATE}{$sname} or die "State $sname does not exist\n";
		$sbref = $this->{STATE}{$sname};
		$sbname = $sname;
		
		# keep track of base states, can not merge base states with each other
		$base{$sbname} = 1;

		# mapping to be used for converting states in hmms, map base to itself
		$map{$sbname} = $sbname;

		# initialize if base model has not been used for merging yet
		unless( exists $sbref->{MERGE}{NAME}{$sbname} )
		{
		    $sbref->{MERGE}{NAME}{$sbname} = 1;
		    $sbref->{MERGE}{COUNT} = 1;
		}

		$firststate = 0;
		next STATE;
	    }
	    else
	    {
		unless( exists $this->{STATE}{$sname} )
		{
		    print STDERR "State $sname does not exist, not merged with $sbname\n";
		    next STATE;
		}		

		if( exists $base{$sname} )
		{
		    die "Attempt to merge base state $sname into $sbname\n";
		}

		if( exists $sbref->{MERGE}{NAME}{$sname} )
		{
		    print STDERR "State $sname multiple merge for $sbname, not merged\n";
		    next STATE;
		}
		else
		{
		    # add this state to the states merged with base state
		    $sbref->{MERGE}{NAME}{$sname} = ++$sbref->{MERGE}{COUNT};

		    # map this state to its merged base state
		    $map{$sname} = $sbname;

		    # reference to data for state to merge with base state
		    $sref = $this->{STATE}{$sname};
		}
	    }
	    

	    # merge state with base state

	    $sfact = 1/( $sbref->{MERGE}{COUNT} );
	    
	    for( $stream=0; $stream < $nstream; $stream++ )
	    {
		# precondition the mixture pdf probabilities in the base state
		$nbmix = $sbref->{NUMMIXES}[$stream];
		for( $bmix = 0; $bmix < $nbmix; $bmix++ )
		{
		    $sbref->{STREAMMIX}[$stream][$bmix]{PROB} *= (1-$sfact);
		}

		# add mixture information from $sref to the base state
		$nmix = $sref->{NUMMIXES}[$stream];
	      MIX:
		for( $mix = 0; $mix < $nmix; $mix++ )
		{
		    # check if mixture pdf already is in base state
		    # if so, just adjust the mixture probability
		    $nbmix = $sbref->{NUMMIXES}[$stream];
		    for( $bmix = 0; $bmix < $nbmix; $bmix++ )
		    {
			if( $sbref->{STREAMMIX}[$stream][$bmix]{MIXPDF} eq
			    $sref->{STREAMMIX}[$stream][$mix]{MIXPDF} )
			{
			    $sbref->{STREAMMIX}[$stream][$bmix]{PROB} = 
				($sbref->{STREAMMIX}[$stream][$bmix]{PROB} ) +
				    $sfact*( $sref->{STREAMMIX}[$stream][$mix]{PROB} );
			    next MIX;
			}
		    }

		    # if you get to here, then this is a new mixture pdf not in the base state
		    # for this stream - add new pdf to base state
		    $sbref->{STREAMMIX}[$stream][$nbmix]{MIXPDF} = 
			$sref->{STREAMMIX}[$stream][$mix]{MIXPDF};

		    $sbref->{STREAMMIX}[$stream][$nbmix]{PROB} = 
			$sfact*( $sref->{STREAMMIX}[$stream][$mix]{PROB} );
		    
		    $sbref->{NUMMIXES}[$stream]++;
		}
	    }
	}
	    
	# remove the state just merged with the base state
	delete $this->{STATE}{$sname};
    }


    close( MRG );


    # Modify all HMMs to reflect the merged states that they now use
    while( ($hname, $href) = each %{ $this->{HMM} } )
    {
	$nstates = $href->{NUMSTATES};
	for( $state=0; $state < $nstates; $state++ )
	{
	    $sname = $href->{STATE}[$state];
	    if( exists $map{$sname} )
	    {
		$href->{STATE}[$state] = $map{$sname};
	    }
	}
    }


    # clean up - remove base and map hashes and merge information
    undef %base;
    undef %map;

    while( ($sname, $sref) = each %{ $this->{STATE} } )
    {
	delete $sref->{MERGE} if exists $sref->{MERGE};
    }
    
    return;
}


#================================================================
# mergevar
    
# Merge a set of variance vectors to a single variance vector, which
# in effect ties the variances together.
    
#--------------------------------
sub mergevar
{
    my( $this, $mrgfile ) = @_;
    my( $firstvar, $vname, $vbname, $vbref, $vref );
    my( $vfact, $vsize, $vix );
    my( $lin, $snum, @varlist, %base, %map );
    my( $mname, $mref );
    
    
    # process all lines of merge file
    open( MRG, "$mrgfile" ) or die "Can not open merge file $mrgfile\n";

    while( $lin = <MRG> )
    {
	chomp($lin);
	@varlist = split( " ", $lin );
	

	# first variance on the line is the base variance to merge into
	$firstvar  = 1;
    
      VAR:
	foreach $vname ( @varlist )
	{
	    if( $firstvar )
	    {
		# initialize base variance
		exists $this->{VARIANCE}{$vname} or die "Variance $vname does not exist\n";
		$vbref = $this->{VARIANCE}{$vname};

		( $vbref->{TYPE} =~ /v/ ) or die "Can only merge variances for now\n";

		# base variance macro name
		$vbname = $vname;
		
		# keep track of base variances, can not merge base variances with each other
		$base{$vbname} = 1;

		# mapping to be used for converting variances in mixtures, map base to itself
		$map{$vbname} = $vbname;

		# initialize merge info if base model has not been used for merging yet
		unless( exists $vbref->{MERGE}{NAME}{$vbname} )
		{
		    $vbref->{MERGE}{NAME}{$vbname} = 1;
		    $vbref->{MERGE}{COUNT} = 1;
		}

		$firstvar = 0;
		next VAR;
	    }
	    else
	    {
		unless( exists $this->{VARIANCE}{$vname} )
		{
		    print STDERR "VARIANCE $vname does not exist, not merged with $vbname\n";
		    next VAR;
		}		

		if( exists $base{$vname} )
		{
		    die "Attempt to merge base variance $vname into $vbname\n";
		}

		if( exists $vbref->{MERGE}{NAME}{$vname} )
		{
		    print STDERR "State $vname multiple merge for $vbname, not merged\n";
		    next VAR;
		}
		else
		{
		    # add this variance to the variances merged with base variance
		    $vbref->{MERGE}{NAME}{$vname} = ++$vbref->{MERGE}{COUNT};

		    # map this variance to its merged base variance name
		    $map{$vname} = $vbname;

		    # reference to data for variance to merge with base variance
		    $vref = $this->{VARIANCE}{$vname};
		}
	    }

	    if( $vref->{SIZE} != $vbref->{SIZE} )
	    {
		die "Merge size conflict for ${vbref} and ${vref}\n";
	    }

	    # merge variance with base variance

	    $vfact = 1/( $vbref->{MERGE}{COUNT} );
	    
	    # average base variance with new variance
	    $vsize = $vbref->{SIZE};
	    for( $vix = 0; $vix < $vsize; $vix++ )
	    {
		$vbref->{VECTOR}[$vix] *= (1-$vfact);
		$vbref->{VECTOR}[$vix] += $vfact * $vref->{VECTOR}[$vix];
	    }

	}
	    
	# remove the variance just merged with the base variance
	delete $this->{VARIANCE}{$vname};
    }

    close( MRG );

    # Modify all mixture macros to reflect the merged variances
    while( ($mname, $mref) = each %{ $this->{MIXPDF} } )
    {
	$vname = $mref->{VARIANCE};
	if( exists $map{$vname} )
	{
	    $mref->{VARIANCE} = $map{$vname};
	    
	    # remove GCONST if it exists, since it is no longer valid
	    # could recompute it here, but let HTK do that
	    if( exists $mref->{GCONST} )
	    {
		delete $mref->{GCONST};
	    }
	}
    }

    # clean up - remove base and map hashes and merge information
    undef %base;
    undef %map;

    while( ($vname, $vref) = each %{ $this->{VARIANCE} } )
    {
	delete $vref->{MERGE} if exists $vref->{MERGE};
    }
    
    return;
}


#================================================================
# mergemean
    
# Merge a set of mean vectors to a single mean vector, which
# in effect ties the means together.
    
#--------------------------------
sub mergemean
{
    my( $this, $mrgfile ) = @_;
    my( $firstmean, $uname, $ubname, $ubref, $uref );
    my( $ufact, $usize, $uix );
    my( $lin, $snum, @meanlist, %base, %map );
    my( $mname, $mref );
    
    
    # process all lines of merge file
    open( MRG, "$mrgfile" ) or die "Can not open merge file $mrgfile\n";

    while( $lin = <MRG> )
    {
	chomp($lin);
	@meanlist = split( " ", $lin );
	

	# first mean on the line is the base mean to merge into
	$firstmean  = 1;
    
      MEAN:
	foreach $uname ( @meanlist )
	{
	    if( $firstmean )
	    {
		# initialize base mean
		exists $this->{MEAN}{$uname} or die "Mean $uname does not exist\n";
		$ubref = $this->{MEAN}{$uname};

		# base mean macro name
		$ubname = $uname;
		
		# keep track of base means, can not merge base means with each other
		$base{$ubname} = 1;

		# mapping to be used for converting means in mixtures, map base to itself
		$map{$ubname} = $ubname;

		# initialize merge info if base model has not been used for merging yet
		unless( exists $ubref->{MERGE}{NAME}{$ubname} )
		{
		    $ubref->{MERGE}{NAME}{$ubname} = 1;
		    $ubref->{MERGE}{COUNT} = 1;
		}

		$firstmean = 0;
		next MEAN;
	    }
	    else
	    {
		unless( exists $this->{MEAN}{$uname} )
		{
		    print STDERR "MEAN $uname does not exist, not merged with $ubname\n";
		    next MEAN;
		}		

		if( exists $base{$uname} )
		{
		    die "Attempt to merge base mean $uname into $ubname\n";
		}

		if( exists $ubref->{MERGE}{NAME}{$uname} )
		{
		    print STDERR "State $uname multiple merge for $ubname, not merged\n";
		    next MEAN;
		}
		else
		{
		    # add this mean to the means merged with base mean
		    $ubref->{MERGE}{NAME}{$uname} = ++$ubref->{MERGE}{COUNT};

		    # map this variance to its merged base variance name
		    $map{$uname} = $ubname;

		    # reference to data for mean to merge with base mean
		    $uref = $this->{MEAN}{$uname};
		}
	    }

	    if( $uref->{SIZE} != $ubref->{SIZE} )
	    {
		die "Merge size conflict for ${ubref} and ${uref}\n";
	    }

	    # merge variance with base variance

	    $ufact = 1/( $ubref->{MERGE}{COUNT} );
	    
	    # average base mean with new mean
	    $usize = $ubref->{SIZE};
	    for( $uix = 0; $uix < $usize; $uix++ )
	    {
		$ubref->{VECTOR}[$uix] *= (1-$ufact);
		$ubref->{VECTOR}[$uix] += $ufact * $uref->{VECTOR}[$uix];
	    }

	}
	    
	# remove the mean just merged with the base mean
	delete $this->{MEAN}{$uname};
    }

    close( MRG );

    # Modify all mixture macros to reflect the merged means
    while( ($mname, $mref) = each %{ $this->{MIXPDF} } )
    {
	$uname = $mref->{MEAN};
	if( exists $map{$uname} )
	{
	    $mref->{MEAN} = $map{$uname};
	}
    }

    # clean up - remove base and map hashes and merge information
    undef %base;
    undef %map;

    while( ($uname, $uref) = each %{ $this->{MEAN} } )
    {
	delete $uref->{MERGE} if exists $uref->{MERGE};
    }
    
    return;
}
