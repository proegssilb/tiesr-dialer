#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # tiesr_net_convert.pl
 #
 # Convert and display contents of TIesr network binary file.
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
tiesr_net_convert [-Help] [-d] [-h hmmlist | -p phnlist] [-o outfile] [-l] net_bin_file

";


$DOCUMENTATION = " 

This perl script converts a net.bin file to big endian or little endian.
If the -d option is used, it  provides for output of the information in
the TIesr net.bin file so that the grammar network contents can be examined.
If no -o option is given, no file is output.

If a text file containing the ordered hmm list of all hmms used in the
grammar is available, which should be the case when models are created
using model2bin, then the -h option can be used to output names with
each hmm symbol when the -d option is specified.

If only the dictionary phone list is available, such as when TIesrFlex is
used to create the grammar network, then specify the phone list using
the  -p option.  In this case the hmm2phone.bin file must be in the same
directory as the net.bin file, and will be used to index from HMM to monophone.

If the net file is in a directory that contains the word.lis file, then
the word information will automatically be output with the -d option.

";

#----------------------------------------------------------------

use Getopt::Long;

use File::Basename;

use TIesrNet;

# read options
GetOptions('Help' => \$outdoc,
	   'l' => \$littleendian,
	   'd' => \$showdata,
	   'h=s' => \$hmmfile,
	   'p=s' => \$phonefile,
	   'o=s' => \$outfile );


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
    exit 1;
}


# Read the list of words from word.lis file
if( defined $showdata )
{
    $wordfile = dirname( $ARGV[0] );
    $wordfile .= "/word.lis";
}


if( defined $hmmfile or defined $phonefile )
{
    $hmm2phonefile = dirname( $ARGV[0] );
    $hmm2phonefile .= "/hmm2phone.bin";
}

# Create a net structure
$net = TIesrNet->new();

# Parse the network supplied
if( defined $hmmfile )
{
    $net->parsenet( $ARGV[0], $wordfile, $hmmfile, $hmm2phonefile, 0 );
}
elsif( defined $phonefile )
{
    $net->parsenet( $ARGV[0], $wordfile, $phonefile, $hmm2phonefile, 1 );
}
else
{
    $net->parsenet( $ARGV[0], $wordfile );
}


# Show all grammar network data if requested
if( $showdata )
{
    # Show header info
    print "Number of HMMs in network: ", $net->{NUMHMMS}, "\n";
    print "Number of symbols in network: ", $net->{NUMSYMS}, "\n";
    print "Number of words in network: ", $net->{NUMWORDS}, "\n";
    print "Number of HMM sets: ", $net->{NUMSETS}, "\n";
    print "Number of HMMs per set: ", $net->{NUMPERSET}, "\n";

    # Show start symbols
    $numsyms =  scalar( @{$net->{STARTSYMS}} );
    print "\n\nStart Symbols: ", $numsyms, "\n";
    for( $sym=0; $sym<$numsyms; $sym++ )
    {
	printf "%5d ", $net->{STARTSYMS}[$sym];

	# format 10 symbols per line
	print "\n" if( $sym > 0 and $sym < $numsyms-1 and ($sym+1) % 10 == 0 );
    }
    print "\n";

    # Show end symbols
    $numsyms =  scalar( @{$net->{ENDSYMS}} );
    print "\n\nEnd Symbols: ", $numsyms, "\n";
    for( $sym=0; $sym<$numsyms; $sym++ )
    {
	printf "%5d ", $net->{ENDSYMS}[$sym];

	# format 10 symbols per line
	print "\n" if( $sym > 0 and $sym < $numsyms-1 and ($sym+1) % 10 == 0 );
    }
    print "\n";


    # Show all symbol info
    print "\n\nSymbol Information\n";
    $numsyms =  scalar( @{$net->{SYMBOLS}} );
    for( $sym=0; $sym<$numsyms; $sym++ )
    {
	$r_sym = $net->{SYMBOLS}[$sym];
	print "\nSymbol: $sym  HMM index: $r_sym->{HMMCODE}";
	print "($r_sym->{HMMNAME})" if defined $r_sym->{HMMNAME};
	print "  Number transitions: $r_sym->{NUMTRANS}";
	print "  Word: $r_sym->{WORDINDEX}" if $r_sym->{WORDSYM};
	print "($r_sym->{WORDNAME})" if defined $r_sym->{WORDNAME};
	print "\n";
	$numtrans = $r_sym->{NUMTRANS};
	for( $trn=0; $trn < $numtrans; $trn++ )
	{
	    printf "%5d ", $r_sym->{TRANS}[$trn];

	    # format 10 symbols per line
	    print "\n" if( $trn > 0 and $trn < ($numtrans-1)  and ($trn+1) % 10 == 0 );
	}
	print "\n";
    }
}


if( $outfile )
{
  $net->output( $outfile, $littleendian );
}
