#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # jac_convert.pl
 #
 # Convert TIesr JAC file and display contents.
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
tiesr_jac_convert [-Help] [-SVA] [-WSVA] [-SBC] [-ORM] [-CNF] 
   [-d] [-inshort shortbytes] [-inlong longbytes] 
   [-outshort shortbytes] [-outlong longbytes] [-L] [-o outfile] jac_file";


$DOCUMENTATION = " 

This perl script is used to convert a given tiesr JAC file into little- or
big-endian format.  The user may optionally output the information
in text form on stdout using the -d option.  The -Help option outputs
this help information. 

This tool automatically determines whether the input jac_file is in
little or big endian format.  However, some data in the TIesr JAC file
is stored as short, and some stored as long.  The sizes of these may
be different on different machines.  The sizes in the input JAC file
can be specified using -inshort and -inlong options.  If not
specified, they are assumed to be 2 and 4 respectively. The sizes in
the output file can be specified by the -outshort and -outlong
options.  Use care because if the correct sizes are not specified, then the
output will not be correct.

Options are required to specify the information in the jac file.
These must be set if the input jac file contains the respective
information.

-SVA : jac file contains SVA information
-WSVA: jac file contains Weighted SVA information (implies SVA)
-SBC: jac file contains SBC information
-ORM: jac file contains ORM data
-CNF: jac file contains confidence information

If the -o option is not specified, then no jac file will be output.
This allows one to just view the contents of the jac file using the -d
option.  To specify little-endian output use -L, otherwise, big-endian
output is assumed.";

#----------------------------------------------------------------

use Getopt::Long;

# Global variables used by the script

# Scaling for mfcc mean and variance, up to 16 dimensions.  
$CM_NBR_BETA = 3;

# read mmf files and a file containing a list of hmms to load
GetOptions('Help' => \$outdoc,
	   'SVA' => \$SVA,
	   'WSVA' => \$WSVA,
	   'SBC' => \$SBC,
	   'ORM' => \$ORM,
	   'CNF' => \$CNF,
	   'L'   => \$LITTLEENDIAN,
	   'd'   => \$DISPLAY,
	   'o=s'  => \$outfile,
	   'inshort=i' => \$inshort,
	   'inlong=i' => \$inlong,
	   'outshort=i' => \$outshort,
	   'outlong=i'=> \$outlong
	   );


# Output only the documentation if requested

if( $outdoc )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}

if( ! defined $ARGV[0] )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}


# Set short and long sizes
$inshort = 2 if ! $inshort;
$inlong = 4 if ! $inlong;

$outshort = 2 if ! $outshort;
$outlong = 4 if ! $outlong;


# Open the jac input file for reading
open( JAC, "<$ARGV[0]" ) or die "Can not open JAC file\n";

if( defined $outfile )
{
    open(OUT, ">$outfile") or die "Can not open $outfile\n";
}


# Determine packing type based on n_filter parameter in JAC file
# This assumes n_filters < 256 which should always be true.
read( JAC, $buf, 1 );
$jacle = ( unpack( "C", $buf ) != 0 )  ?  1 : 0;
seek JAC, 0, 0;

# Determine machine packing type
$tmp = unpack( "s", pack( "v", 0x1234) );
$machle = ($tmp == 0x1234);
undef $tmp;


# Get basic parameters
&get_basic;

&get_sva if( defined $SVA or defined $WSVA );

&get_wsva if( defined $WSVA );

&get_sbc if( defined $SBC );

&get_orm if( defined $ORM );

&get_conf if(defined $CNF );

close( JAC );

close( OUT) if( defined $outfile );

exit;



#----------------------------------------------------------------
# get basic JAC parameters: n_filter, nbr_dim, log_H and accum
#
sub get_basic
{
    ($n_filter, $nbr_dim) = get_values( \*JAC, $inshort, 0, 2);

    @log_H = get_values( \*JAC, $inshort, 1, $n_filter );

    @accum = get_values( \*JAC, $inlong, 1, 2*$n_filter );

    # Display values if requested
    if( defined $DISPLAY )
    {
	print "BASIC JAC PARAMETERS:\n";
        print "n_filter: ", $n_filter, "\n";
	print " nbr_dim: ", $nbr_dim, "\n";

	print "   log_H: ";
	print_array(@log_H);

	print "   accum: ";
	print_array( @accum );
    }

    # Output values to file in proper order if requested
    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 0, $n_filter, $nbr_dim  );
	put_values( \*OUT, $outshort, 1, @log_H );
	put_values( \*OUT, $outlong, 1, @accum );
    }

    return;
}

#----------------------------------------------------------------
# Get SVA parameters: log_var_rho, var_fisher_IM
#
sub get_sva
{
    @log_var_rho = get_values( \*JAC, $inshort, 1, $nbr_dim );

    @var_fisher_IM = get_values( \*JAC, $inlong, 1, $nbr_dim );

    if( defined $DISPLAY)
    {
	print "\nSVA PARAMETERS:\n";
	print "log_var_rho: ";
	print_array(@log_var_rho);

	print "var_fisher_IM: ";
	print_array(@var_fisher_IM);
    }

    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 1, @log_var_rho );
	put_values( \*OUT, $outlong, 1, @var_fisher_IM );
    }

    return;
}


#----------------------------------------------------------------
# Get WSVA parameter noise_lvl
#
sub get_wsva
{
    ($noise_lvl) = get_values( \*JAC, $inshort, 1, 1 );

    if( defined $DISPLAY)
    {
	print "\nWSVA PARAMETERS:\n";
	print "noise_lvl: $noise_lvl\n";
    }

    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 1, $noise_lvl );
    }	    

    return;
}

#----------------------------------------------------------------
# Get SBC parameters
#
sub get_sbc
{
    ($n_utt, $n_nodes, $hlr_TRANSFORM_SIZE) = get_values( \*JAC, $inshort, 0, 3 );

    for( $nd = 0; $nd < $n_nodes; $nd++ )
    {
	($vector_count[$nd]) = get_values( \*JAC, $inshort, 0, 1 );
	($parent_node[$nd]) = get_values( \*JAC, $inshort, 1, 1 );
	$dif1[$nd] = [ get_values( \*JAC, $inlong, 1, $hlr_TRANSFORM_SIZE ) ];
	$dif2[$nd] = [ get_values( \*JAC, $inlong, 1, $hlr_TRANSFORM_SIZE ) ];
	$p_trans[$nd] = [ get_values( \*JAC, $inshort, 1, $hlr_TRANSFORM_SIZE ) ];
    }

    ($n_monophones ) = get_values( \*JAC, $inshort, 0, 1 );
    @hlr_Terminal = get_values( \*JAC, $inshort, 0, $n_monophones );
    @hlrTransformIndex = get_values( \*JAC, $inshort, 0, $n_nodes );
    @hlrTransforms = get_values( \*JAC, $inshort, 1, $n_nodes*$hlr_TRANSFORM_SIZE );

    if( defined $DISPLAY )
    {
	print "\nSBC PARAMETERS\n";
	print "n_utt: $n_utt\n";
	print "n_nodes: $n_nodes\n";
	print "hlr_TRANSFORM_SIZE: $hlr_TRANSFORM_SIZE\n";
	
	for( $nd = 0; $nd < $n_nodes; $nd++ )
	{
	    print "vector_count[$nd]: $vector_count[$nd]\n";

	    $pn = ($parent_node[$nd]>>8 ) & 0xff;
	    print "parent_node[$nd]: $pn\n";

	    $ti = $parent_node[$nd] & 0xff;
	    print "transform_index[$nd]: $ti\n";

	    print "dif1[$nd]: ";
	    print_array( @{ $dif1[$nd] } );

	    print "dif2[$nd]: ";
	    print_array( @{ $dif2[$nd] } );

	    print "p_trans[$nd]: ";
	    print_array( @{ $p_trans[$nd] } );
	}

	print "n_monophones: $n_monophones\n";

	print "hlr_Terminal: ";
	print_array( @hlr_Terminal );
	
	print "hlrTransformIndex: ";
	print_array( @hlrTransformIndex );

	
	for( $nd = 0; $nd < $n_nodes; $nd++ )
	{
	    print "hlrTransforms[$nd]: ";
	    print_array( @hlrTransforms[ $nd*$hlr_TRANSFORM_SIZE .. ($nd+1)*$hlr_TRANSFORM_SIZE -1 ] );
	}
    }


    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 0,  $n_utt, $n_nodes, $hlr_TRANSFORM_SIZE );

	for( $nd = 0; $nd < $n_nodes; $nd++ )
	{
	    put_values( \*OUT, $outshort, 0,  $vector_count[$nd] );

	    put_values( \*OUT, $outshort, 1,  $parent_node[$nd] );
	    
	    put_values( \*OUT, $outlong, 1, @{ $dif1[$nd] } );

	    put_values( \*OUT, $outlong, 1, @{ $dif2[$nd] } );

	    put_values( \*OUT, $outshort, 1, @{ $p_trans[$nd] } );
	}	    

	put_values( \*OUT, $outshort, 0, $n_monophones );

	put_values( \*OUT, $outshort, 0, @hlr_Terminal );

	put_values( \*OUT, $outshort, 0, @hlrTransformIndex );

	put_values( \*OUT, $outshort, 1, @hlrTransforms );
    }	    
}


#----------------------------------------------------------------
# Get ORM parameters
#
sub get_orm
{
    ($inum_prior) = get_values( \*JAC, $inshort, 1, 1);

    @iprior = get_values( \*JAC, $inshort, 1, $inum_prior ); 
    @pprior = get_values( \*JAC, $inshort, 1, $inum_prior ); 

    if( defined $DISPLAY )
    {
	print "\nORM PARAMETERS:\n";
	print "inum_prior: $inum_prior\n";
	
	print "iprior: ";
	print_array( @iprior );

	print "pprior: ";
	print_array( @pprior );
    }	

    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 1, $inum_prior );

	put_values( \*OUT, $outshort, 1, @iprior );

	put_values( \*OUT, $outshort, 1, @pprior );
    }
}


#----------------------------------------------------------------
# Get confidence parameters
#
sub get_conf
{

    ($h0mean, $h0inv, $h0gconst ) = get_values( \*JAC, $inshort, 1, 3 );
    ($h1mean, $h1inv, $h1gconst ) = get_values( \*JAC, $inshort, 1, 3 );
    ( $nsnr ) = get_values( \*JAC, $inshort, 1, 1 );

    @lgBeta = get_values( \*JAC, $inshort, 1, $CM_NBR_BETA );
    @acc_dif2_beta = get_values( \*JAC, $inlong, 1, $CM_NBR_BETA );

    if( defined $DISPLAY )
    {
	print "\nCONFIDENCE PARAMETERS\n";
	print "H0_mean: $h0mean\n";
	print "H0inv: $h0inv\n";
	print "H0gconst: $h0gconst\n";
	print "H1_mean: $h1mean\n";
	print "H1inv: $h1inv\n";
	print "H1gconst: $h1gconst\n";

	print "NSNR: $nsnr\n";

	print "lgBeta:";
	print_array( @lgBeta );

	print "acc_dif2_beta:";
	print_array( @acc_dif2_beta );
    }


    if( defined $outfile )
    {
	put_values( \*OUT, $outshort, 1, $h0mean, $h0inv, $h0gconst );

	put_values( \*OUT, $outshort, 1, $h1mean, $h1inv, $h1gconst );

	put_values( \*OUT, $outshort, 1, $nsnr );

	put_values( \*OUT, $outshort, 1, @lgBeta );

	put_values( \*OUT, $outlong, 1, @acc_dif2_beta );
    }	    
}


#----------------------------------------------------------------
# Get little/big endian short/long signed/unsigned integer values
# $le specifes little-endian order in file.  $intsize is the size of 
# the integers in bytes. $sign is a flag indicating signed or unsigned. 
#
sub get_values
{
    my $file = shift(@_);
    my $intsize = shift(@_);
    my $signed = shift(@_);
    my $num = shift(@_);
    
    my $buf;
    my @intval;
    
    my $doreverse = ( $jacle != $machle );


    read( $file, $buf, $num*$intsize );

    $utype = "S" if $intsize == 2;
    $utype = "L" if $intsize == 4;
    $utype = "Q" if $intsize == 8;


    @intval = unpack "${utype}*", $buf;
  
    foreach $val ( @intval )
    {
	if( $doreverse )
	{
	    my $tmp, @tmp;
	    $tmp = pack( $utype, $val );
	    @tmp = unpack( "C$intsize", $tmp);
	    @tmp = reverse(@tmp);
	    $tmp = pack( "C$intsize", @tmp );
	    $val = unpack( $utype, $tmp );
	}

	$val = unpack(  lc($utype), pack( $utype, $val ) ) if $signed;
    }


    return @intval;
}


sub print_array
{
    foreach $val (@_)
    {
	print "$val ";
    }
    print "\n";
}


sub put_values
{
    my ($file, $size, $signed, @vals) = @_;

    my ( $utype, $type );
    $utype = "S" if $size == 2;
    $utype = "L" if $size == 4;
    $utype = "Q" if $size == 8;

    my $type =  $signed ?  lc( $utype ) : $utype;


    my $doswap = ( ( $LITTLEENDIAN && ! $machle ) or 
		( ! $LITTLEENDIAN && $machle ) );


    for $val ( @vals )
    {
	if( $doswap )
	{
	    my ( $tmp, @tmp );
	    $tmp = pack( $type, $val );
	    @tmp = unpack( "C$size", $tmp);
	    @tmp = reverse(@tmp);
	    $tmp = pack( "C$size", @tmp );
	    $val = unpack( $type, $tmp );
	}
	    
	print $file pack( $type, $val );
    }
}
