#!/usr/bin/perl
#-------------------------------------------------------------------------

 #
 # c_to_char.pl
 #
 # Convert octal representation of character in a string to character.
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
#-----------------------------------------------------------------------
$USAGE = "
c_to_char [-Help] input_file output_file";

$DOCUMENTATION = "
This perl script converts occurrences of characters expressed in a character
string as  \"\ooo\" or \"\xhh\" to the actual 8-bit character within the
string.";

#----------------------------------------------------------------
use Getopt::Long;

GetOptions( Help => \help );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}

$infile = $ARGV[0];
$outfile = $ARGV[1];


open( IN, "<$infile" ) or die "Can not open $infile\n";
open( OUT, ">$outfile" ) or die "Can not open $outfile\n";

while( $lin = <IN> )
{
    chomp($lin);
    
    
    while( $lin =~ /((\\\\)*)(\\(x[0-9a-fA-F]{2}|[0-7]{3}))/ )
    {
	eval "\$c = \"$3\""; 
	$lin =  $` . $1 . $c . $';
    }

    print OUT "$lin\n";
}
