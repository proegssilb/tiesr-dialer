#!/usr/bin/perl
#-------------------------------------------------------------

 #
 # train_trees.pl
 #
 # Build decision trees for letter data.
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
train_trees -Help -D datadir -CF  conflevel";


$DOCUMENTATION = "
This script builds decision trees for the letter data created using
ttp_dict_to_trees.pl.  The -D option specifies the directory that holds the
*.names and *.data files. The -CF option is the % confidence level parameter
(1-99) used by c4.5 to reduce the size of a decision tree by eliminating
branches of the tree. Smaller percentages remove more detailed branches, but
this may result in inaccurate trees. Larger values eliminate fewer branches, but
may result in many meaningless divisions of the tree due to overtraining.";

#---------------------------------------------------------------

use Getopt::Long;

GetOptions( "Help" => \$help,
	    "D=s" => \$datadir,
	    "CF=i" => \$conflevel );


if( $help )
{
    print "$USAGE\n\n";
    print "$DOCUMENTATION\n";
    exit 1;
}

die "train_trees: Invalid directory $datadir\n" if ! -d $datadir;

die "train_trees: Invalid confidence level $conflevel\n" 
    unless ( $conflevel > 0 and $conflevel <= 100 );



# Process each letter data file in working directory
foreach $fstem ( glob( "$datadir/*.data" ) )
{
    $fstem =~ s/\.data\s*$//;

    $letter = substr ($fstem, -1, 1);
    
    print "Training tree for letter: $letter\n";

    $cmd = "c4.5  -f $fstem -c $conflevel > ${fstem}.CF${conflevel}log";

    system( $cmd ) == 0 or die "train_trees: Could not run c4.5 on $fstem\n";
}
