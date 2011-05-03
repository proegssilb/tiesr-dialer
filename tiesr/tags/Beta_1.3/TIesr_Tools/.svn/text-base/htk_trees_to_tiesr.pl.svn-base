#!/usr/local/bin/perl
#-------------------------------------------------------------

 #
 # htk_trees_to_tiesr.pl
 #
 # Convert HTK acoustic trees to TIesr compatible binary format.
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
htk_trees_to_tiesr [-Help] -T treefile -C clusterdir  [-g genders] -le -d outdir";


$DOCUMENTATION = " 

This perl script is used to interface with an ASCII HTK Tree cluster
file.  It outputs binary files of questions and acoustic decision
trees that TIesrFlex can use along with the binary HMM file
information created using htk_mmf_to_tiesr to synthesize HMM models.

Use the -help option by itself to print this documentation and usage.

The -T option specifies the name of the HTK tree file.  

The -C option specifies the name of the directory containing all of
the fixed-point binary files, such as fxclust.list, that have been
created by the utility htk_mmf_to_tiesr.  Note that this implies that
htk_mmf_to_tiesr must be successfully run prior to using this tool.

If the trees will be gender-dependent then the -g option is used to
specify the gender tags separated by commas as a string, such as
':f,:m'.  This allows this tool to check for gender consistency.

The -le option specifies to output binary files in littleendian format.

The -d option specifies the output directory that will hold all of the
binary files.  It should normally be the same as the -C option. ";

#----------------------------------------------------------------

use Getopt::Long;

# This tool uses the HTKtrees module to parse the HTK trees file
use HTKtrees;

# read mmf files and a file containing a list of hmms to load
GetOptions('Help' => \$outdoc,
           'T=s' => \$treesfile,
	   'C=s' => \$clusterdir,
           'g=s' => \$genders,
	   'le' => \$littleendian,
           'd=s' => \$outdir );


if( defined $outdoc )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}


if( ! -f $treesfile or ! -d $clusterdir or ! defined $outdir )
{
    print "Invalid file/directory names\n";
    print "$USAGE\n";
    print "$DOCUMENTATION\n";
    exit 1;
}

if( ! -d $outdir )
{
    mkdir $outdir, 0770
	or die "Could not create $outdir\n";
}


# Parse the HTK trees file
$trees = HTKtrees->new();
$trees->parsetrees( $treesfile );


# Output binary tree question information
@questionlist = output_questions( $trees, $clusterdir, $genders, 
				  $littleendian, $outdir );


# Output binary trees information
@treelist = output_trees( $trees, $clusterdir, $genders,
			  \@questionlist, 
			  $littleendian, $outdir );


#----------------------------------------------------------------
# output_questions

# This subroutine outputs a set of questions files, and also a list of
# questions, so that the user knows the order of questions within the
# binary files.  The following files are output:

# _qs.list : This is a text listing of each question name as a text
# string on a line. The order of the questions output is the order in
# which the questions are coded in the binary files.

# _qs.bin : This is a binary file containing 1) 32-bit, number of
# bytes of data in the file. 2) Definitions of each question
# containing for each question a) an unsigned byte indicating number
# of phone contexts in the question, b) a byte representing each phone
# context consisting of the phone index (from the monophone list) ored
# with 0x00 if left context or 0x80 if right context.

# _qstbl.bin: This is a binary file containing 1) 32-bit, number of
# questions in the _qs.bin file.  2) For each question in _qs.bin an
# offset in number of data bytes in _qs.bin to the question (excluding
# the initial 32-bit number in _qs.bin).

sub output_questions
{
    use strict;

    # Arguments
    my $treeref = shift;
    my $clusterdir = shift;
    my $genders = shift;
    my $littleendian = shift;
    my $outdir = shift;


    my $plong = $littleendian ? "V" : "N";
    my $pusbyte = "C";


    # If a gender string is defined, then determine genders needed
    my(@genderlist);
    my($genderre);
    if( $genders )
    {
	@genderlist = sort( split /\s+|\s*,\s*/ , $genders );
	$genderre = join '|', @genderlist ;
    }


    # Make hash containing monophones and their indices, include
    # "sp" as last phone
    open( MONO, "$clusterdir/monophone.list" ) or
	die "Can not open monophone list\n";

    my $phonect = 0;
    my %monophone = ();
    while( <MONO> )
    {
	chomp;
	$monophone{$_} = $phonect++;
    }
    $monophone{"sp"} = $phonect++ if ! exists $monophone{"sp"};
    close(MONO);


    # Read the configuration file, check for consistency and update it to output
    # whether GD trees are generated or not.
    open(CFG, "$clusterdir/fxconfig.bin" ) or die "Can not open fxconfig.bin\n";
    my $cfgstr;

    # slurp all bytes.  
    my $cread = read CFG, $cfgstr, 1e6;
    close(CFG);

    # Config file may now include dummy GD tree info and clustered weight info
    #die "Invalid fxconfig size\n" unless $cread == 32;
    die "Invalid fxconfig size\n" unless $cread >= 32;

    # Get number of sets of hmms 
    my @config = unpack "${plong}*", $cfgstr;
    my $configsets = $config[6];
    if( $genders && $configsets != scalar(@genderlist) )
    {
	die "Config file mismatch to trees\n";
    }

    # Write out the GD trees flag
    $config[8] = ( $genders ?  1 : 0 );


    open( CFG, ">$clusterdir/fxconfig.bin" ) or die "Can not open fxconfig.bin\n";
    print CFG pack "${plong}*", @config;
    close(CFG);


    # Open output files
    open( LIST, ">$outdir/_qs.list") or die "Can not open _qs.list\n";
    open( QS, ">$outdir/_qs.bin" ) or die "Can not open _qs.bin\n";
    open( TBL, ">$outdir/_qstbl.bin" ) or die "Can not open _qstbl.bin\n";


    # Pad sizes with dummy to start
    print QS pack $plong, 0;
    print TBL pack $plong, 0;


    # Output all questions in alphabetic order
    my $offset = 0;
    my $nquestions = 0;
    my @qlist;
    foreach my $qname ( sort keys %{ $treeref->{QS} } )
    {
	my $qssize = 0;


	# Determine gender consistency if using GD question names
	my $gender = "";
	if( $genders )
	{
	    my $gdname = $qname =~ /^.*?($genderre)$/;
	    $gender = $1;
	    
	    die "Invalid GD question $qname\n" unless $gdname;
	}
	else
	{
	    $gender = "";
	}

	print LIST "$qname\n";

	print QS pack $pusbyte, scalar( @{ $treeref->{QS}{$qname} } ); 
	$qssize++;


	# For each phonetic context, output the index.
	foreach my $context ( @{ $treeref->{QS}{$qname} } )
	{
	    $context =~ /^(?:(\w+?)-\*($gender)\+\*|\*($gender)\+(\w+))$/
		or die "Invalid context $context for $qname\n";
	    my $lphone = $1;
	    my $rphone = $4;

	    if( $lphone )
	    {
		die "Invalid phone in $context for $qname\n"
		    unless exists $monophone{$lphone};
		print QS pack $pusbyte, $monophone{$lphone} | 0x00;
	    }
	    elsif( $rphone )
	    {
		die "Invalid phone in $context for $qname\n"
		    unless exists $monophone{$rphone};
		print QS pack $pusbyte, $monophone{$rphone} | 0x80;
	    }		
	    else
	    {
		die "Invalid L/R context $context for $qname\n";
	    }
	    $qssize++;
	}

	print TBL pack $plong, $offset;
	$offset += $qssize;
	$qssize = 0;

	$nquestions++;
	push @qlist, $qname;
    }

    seek QS, 0, 0;
    print QS pack $plong, $offset;
    close(QS);

    seek TBL, 0, 0;
    print TBL pack $plong, $nquestions;
    close(TBL);

    close(LIST);

    return @qlist;
}


#----------------------------------------------------------------
# output_trees

# This subroutine outputs a set of trees making up the acoustic
# decision trees for all states of all phones.  Each decision tree
# uses the cluster indices as produced by htk_mmf_to_tiesr, and the
# question indices just produced by the output_questions subroutine.
# The subroutine outputs several files representing the trees.  Trees
# are output in a recursive method in order to know the structure of
# the tree.  The files are output as follows:

# _tree.bin: This file contains the tree information.  It contains 1)
# a 32-bit int size of the data in the file in bytes. 2) For each
# tree, a set of ordered 16-bit values indicating question indices
# (MSB=0) or leaf node pdf cluster indices (MSB=1).  The ordering of
# the values for the binary tree is such that "yes" paths are followed
# first, until a leaf node is found, then "no paths are followed.

# _treetbl.bin: This file contains a table for finding the tree
# associated with each phone, state, and possibly gender.  The file
# contains 1) a 32-bit int defining the number of bytes in the file
# (even though everything in the file is written as 4-byte ints). 2)
# For each phone, and possibly each gender, in monophone order with
# male/female interlaced with female first and male second, a 32-bit
# int value representing a 32-bit value offset in this _treetbl.bin
# file to the offset information to locate tree information for each
# phone and gender. 3) For each, possibly gender-dependent, phone the
# tree information data consisting of, a) a 32-bit int containing the
# number of emitting states in the phone, b) for each emitting state,
# a 32-bit value representing the offset in 16-bit ints to the tree
# information assocated with each state of this possibly
# gender-dependent phone in _tree.bin.

sub output_trees
{
    use strict;

    # Arguments
    my $treesref = shift;
    my $clusterdir = shift;
    my $genders = shift;
    my $qlistref = shift;
    my $littleendian = shift;
    my $outdir = shift;

    my $plong = $littleendian ? "V" : "N";
    my $pshort = $littleendian ? "v" : "n";

    my @treelist = ();

    # Make a hash containing questions in question files
    # and their respective indices
    my %qhash;
    @qhash{ @{$qlistref} } = ( 0 .. $#{$qlistref} );


    # Make a list of the monophones.
    # Make a monophone regular expression for matching.
    open( MONO, "$clusterdir/monophone.list" ) or
	die "Can not open monophone list\n";
    my $phonect = 0;
    my @monolist = ();
    while( <MONO> )
    {
	chomp;
	push @monolist, $_;
    }
    my $monore = join "|", @monolist;
    close(MONO);


    # Make hash containing cluster pdf names and their indices
    open( CLUST, "$clusterdir/fxclust.list" )
	or die "Can not open fxclust.list\n";

    my $clcount = 0;
    my %cluster = ();
    while( <CLUST> )
    {
	chomp;
	$cluster{$_} = $clcount++;
    }
    close(CLUST);


    # If a gender string is defined, then determine genders needed
    my(@genderlist) = ("");
    my($genderre) = "";
    if( $genders )
    {
	@genderlist = sort( split /\s+|\s*,\s*/ , $genders );
	$genderre = join '|', @genderlist ;
    }


    # Make a listing of all phones in the trees
    # Determine alphabetic index
    # Ensure all phones match phone list
    my %treephones;
    foreach my $treename ( keys %{ $treesref->{TREE} } )
    {
	my $phname = $treesref->{TREE}{$treename}{PHONE};
	$treephones{ $phname } = -1;

	die "Invalid tree phone $phname\n" 
	    unless $phname =~ /^($monore)($genderre)$/;
    }


    # Check that all phones in the monophone list and genders
    # are represented by trees
    foreach my $phone ( @monolist )
    {
	foreach my $gender (@genderlist)
	{
	    die "Missing tree phone for $phone$gender\n"
		unless exists $treephones{"$phone$gender"};
	}
    }
	    

    # Open the tree binary files
    open(TREE, ">$outdir/_tree.bin" ) or die "Can not open _tree.bin\n";
    open(TBL, ">$outdir/_treetbl.bin" ) or die "Can not open _treetbl.bin\n";
    open(LIST, ">$outdir/_tree.list" ) or die "Can not open _tree.list\n";
    open(PLIST, ">$outdir/_treetbl.list" ) or die "Can not open _treetbl.list\n";

    # Dummy sizes to start    
    print TREE pack $plong, 0;
    print TBL pack $plong, 0;

    # Placeholder for each phone in table
    my $tbloffset = 0;
    foreach my $phname ( keys %treephones )
    {
	print TBL pack $plong, 0;
	$tbloffset++;
    }


    # Output each tree in monophone and gender list order.
    # Output the list of trees so user can know order of trees.
    my $treeoffset = 0;
    my $ntrees = 0;
    my $phcount = 0;
    foreach my $phone (@monolist)
    {
	foreach my $gender (@genderlist)
	{
	    my $phname = "$phone$gender";
	    $phcount++;
	    print PLIST "$phname\n";

	    # Offset of phone in table in 32-bit offset
	    seek TBL, $phcount*4, 0;
	    print TBL pack $plong, $tbloffset;
	    # Go to end of table
	    seek TBL, 0, 2;

	    # Find number of emitting states for this phone
	    my $nemit = 0;
	    my $state = 2;
	    my $treename = $phname . "[${state}]";
	    while( exists $treesref->{TREE}{$treename} )
	    {
		$nemit++;
		$state++;
		$treename = $phname . "[${state}]";
	    }

	    die "Invalid tree $treename\n" if $nemit == 0;

	    # Number of emitting states in the phone
	    print TBL pack $plong, $nemit;
	    $tbloffset++;
	    
	    foreach $state (2 .. $nemit+1)
	    {
		# size is in 16-bit words written to TREE file
		$treename = $phname . "[${state}]";
		my $size = output_tree( $treesref, $treename, 
					\%qhash, \%cluster, 
					$littleendian, *TREE );
	    
		print TBL pack $plong, $treeoffset;
		$tbloffset++;

		$treeoffset += $size;

		print LIST "$treename\n";
		push @treelist, $treename;
		$ntrees++;
	    }
	}
    }

    close(LIST);
    close(PLIST);

    # Size of data in TBL, in bytes.  $tbloffset is in 32-bit longs.
    seek TBL, 0, 0;
    print TBL pack $plong, $tbloffset*4;
    close(TBL);

    # Size of data in TREE in bytes.  $treeoffset is in 16-bit words.
    seek TREE, 0, 0;
    print TREE pack $plong, $treeoffset*2;
    close(TREE);

    return @treelist;

}


#----------------------------------------------------------------
# output_tree

# This subroutine outputs a single tree to a file and returns the
# number of shorts written.  The tree is written recursively, following
# the "yes" answers first until a terminal node is found then, following
# no answers.

sub output_tree
{
    use strict;

    my $treesref = shift;
    my $treename = shift;
    my $questref = shift;
    my $clustref = shift;
    my $littleendian = shift;
    local *TREE = shift;

    # Reference to the node array structure of this tree
    my $tnodes = $treesref->{TREE}{$treename}{NODE};

    # Recursively call output_nodes to output all nodes for the tree
    # starting with node 0 and 0 tree size.
    my $tsize = output_nodes( $tnodes, $questref, $clustref, 0, 0,
			      $littleendian, *TREE );

    return $tsize;
}


#----------------------------------------------------------------
# output_nodes

# This subroutine is a recursive call to output all nodes of a tree to
# the tree file.  First, the question index is output for the node, if
# this node is not singleton.  Then "Yes" child non-terminal nodes are
# followed first, followed by "no" child non-terminal nodes.  Cluster
# indices are output upon finding terminal leaf child nodes for the
# "Yes" or "No" child nodes.  This is a depth-first arrangement of 
# nodes and clusters with yes nodes followed first.

sub output_nodes
{
    use strict;

    # Arguments
    my $tnodes = shift;
    my $questref = shift;
    my $clustref = shift;
    my $node = shift;
    my $tsize = shift;
    my $littleendian = shift;
    local *TREE = shift;

    my $pshort = $littleendian ? "v" : "n";

    # Check if this node indicates a singleton tree,
    # that is, no question for node.  If so,
    # yes and no child nodes are the cluster name,
    # and there exists no tree, so we are done.
    my $question = $tnodes->[$node]{QUESTION};
    if( ! defined $question )
    {
	my $clustername = $tnodes->[$node]{YES};
	exists $clustref->{$clustername} or 
	    die "Invalid tree terminal node $clustername\n";
	
	my $clusterindex = $clustref->{$clustername};
	print TREE pack $pshort, ($clusterindex | 0x8000);
	$tsize++;

	return $tsize;
    }
    else
    {
	# This node has a question, and is a non-terminal
	# question node.  Output the question.
	exists $questref->{$question} or
	    die "Invalid question $question in trees\n";

	my $questindex = $questref->{$question};
	print TREE pack $pshort, $questindex;
	$tsize++;
    }


    # Now we must handle the child nodes for this question node.
    # These are either terminal cluster names or question node indices.
    my $yesnode =  $tnodes->[$node]{YES};
    my $nonode = $tnodes->[$node]{NO};


    # Process "Yes" child node first, followed by "No" child node
    foreach my $childnode ( $yesnode, $nonode )
    {
	# Check if child node is not a terminal.  If so, follow it.
	if( ! exists $clustref->{ $childnode } )
	{
	    $childnode =~ /^\d+$/ or 
		die "Invalid tree node $childnode in trees\n";
	    $tsize = output_nodes( $tnodes, $questref, $clustref, 
			       $childnode, $tsize, $littleendian, *TREE );
	}
	else
	{
	    # Child node is a terminal, output it
	    my $childindex =  $clustref->{ $childnode };
	    print TREE pack $pshort, ( $childindex | 0x8000 );
	    $tsize++;
	}
    }

    return $tsize;
}
