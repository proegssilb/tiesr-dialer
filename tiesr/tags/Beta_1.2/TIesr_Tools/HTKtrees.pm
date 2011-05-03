#----------------------------------------------------------------
#  HTKtrees.pm

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


# This module contains functions that parse an HTK trees file.  The
# HTK trees file consists of a set of questions and a set of trees
# based on those questions.  The result of parsing is a perl structure
# that contains all information about the questions and trees.  The
# functions and the structure of the resulting object are described
# below.

# $trees = HTKtree->new() 
# Creates a hash and and returns a reference to the hash object which
# is of the type HTKtree.  The hash will subsequently be filled by
# parsing the HTK trees file.  The structure contents are as follows:

# $trees->{QS}{$qsname} = [ @array_of_triphone_contexts ];

# $trees->{TREE}{$tree_name}{PHONE} = $phone_name;
# $trees->{TREE}{$tree_name}{STATE} = $emitting_state_number;
# $trees->{TREE}{$tree_name}{NODE}[$node]{QUESTION} = $qsname;
# $trees->{TREE}{$tree_name}{NODE}[$node]{NO} = $no_result;
# $trees->{TREE}{$tree_name}{NODE}[$node]{YES} = $yes_result;

# In the above, $node is a tree node number.  While in HTK node
# numbers are <= 0, this parser changes negative node numbers to
# positive node numbers.  The $no_result and $yes_result will either
# be a node number >=0, or will be a cluster context name indicating a
# terminal leaf node of $node.  It is possible for a tree to be a
# singleton tree.  In this case, $qsname of node 0 will be undefined,
# and the no and yes results will both be the terminal leaf cluster
# context name.

# $trees->parsetrees($filename)
# This function parses the tree, filling the object data structure.

#----------------------------------------------------------------
package HTKtrees;

use strict;

use Exporter;

@HTKtrees::ISA = qw( Exporter );


#--------------------------------
# new

# Construct a new trees structure and bless it into this class
sub new
{
    my $class = shift;
    my $trees = {};
    return bless $trees, $class;
}


#--------------------------------
# parsetrees

# This subroutine parses a tree file.

sub parsetrees
{
    my $treesref = shift;
    my $filename = shift;

    open(TREE, "$filename" ) or 
	die "HTKtree: invalid file $filename\n";

    while( my $line = <TREE> )
    {
	chomp $line;

	# Skip blank lines
	next if $line =~ /^\s*$/;

	# Determine if start of question or tree
	if( $line =~ /^\bQS\b/ )
	{
	    parsequestion( $treesref, $line );
	}
	elsif( $line =~ /^\s*[^\s]+\[\d+\]\s*$/ )
	{
	    parseonetree( $treesref, $line, *TREE );
	}
	else
	{
	    die "HTKtrees: parse fail for line\n$line\n";
	}
    }

    close(TREE);
}


#----------------------------------------------------------------
# parsequestion

# This subroutine parses a question and puts the data into the
# object structure.
sub parsequestion
{
    my $treesref = shift;
    my $line = shift;

    # The line contains the question. 
    # Parse the question into name and contexts.
    $line =~ /^\s*QS\s+\'([^\']+)\'\s+\{\s+(.*?)\s+\}\s*$/
	or die "HTKtrees: question parse fail\n$line\n";
    my $qsname = $1;
    my $contexts = $2;

    # Check format of the contexts
    $contexts =~ /^([^\s,]+\s*,\s*)*[^\s,]+$/
	or die "HTKtrees: question context parse fail\n$line\n";

    # Split out each context
    my @cxtarray = split( /\s*,\s*/, $contexts );
    foreach my $cxt (@cxtarray)
    {
	$cxt =~ s/\"//g;
    }

    # Put the result of the parse in the structure
    $treesref->{QS}{$qsname} = \@cxtarray;
}


#----------------------------------------------------------------
#parseonetree

# This subroutine parses a single tree and outputs the tree contents
# to the structure.

sub parseonetree
{
    use strict;

    # Arguments
    my $treesref = shift;
    my $line = shift;
    local *TREE = shift;

    # The first line contains the phone and state information
    $line =~ /^\s*(([^\s]+)\[(\d+)\])\s*$/;
    my $treename = $1;
    my $phone = $2;
    my $state = $3;

    $treesref->{TREE}{$treename}{STATE} = $state;
    $treesref->{TREE}{$treename}{PHONE} = $phone;
    my $tree = $treesref->{TREE}{$treename};


    # Parse the tree structure
    my $complete = 0;
    my $intree = 0;
    while( $line = <TREE> )
    {
	chomp $line;

	# Singleton tree
	if( ! $intree and 
	    $line =~ /^\s*(\"[^\"]+?\"|[^\"\s{}]+)\s*$/ )
	{
	    my $result = $1;
	    $result =~ s/\"//g;
	    $tree->{NODE}[0]{YES} = $result;
	    $tree->{NODE}[0]{NO} = $result;
	    $tree->{NODE}[0]{QUESTION} = undef;
	    $complete = 1;
	    last;
	}

	# Start of tree
	elsif( ! $intree and
	       $line =~ /^\s*\{\s*$/ )
	{
	    $intree = 1;
	}

	# tree node line
	elsif( $intree and 
	       $line =~  m{
		   ^\s*
		       -?(\d+)\s+   # Node number
		       \'([^\']+)\'\s+   # Question name
		       (\"[^\"]+?\"|[^\s\"]+|-\d+)\s+  #No result
		       (\"[^\"]+?\"|[^\s\"]+|-\d+)\s*$  #Yes result
		   }x
	       )
	{
	    my $node = $1;
	    my $question = $2;
	    my $noans = $3;
	    my $yesans = $4;
	    $noans =~ s/-// if $noans =~ /^-\d+$/;
	    $noans =~ s/\"//g;
	    $yesans =~ s/-// if $yesans =~ /^-\d+$/;
	    $yesans =~ s/\"//g;
	    $tree->{NODE}[$node]{QUESTION} = $question;
	    $tree->{NODE}[$node]{NO} = $noans;
	    $tree->{NODE}[$node]{YES} = $yesans;
	}
	
				       
	# End of tree, check for at least one node.
	# This is not a complete test for a valid tree.  In order
	# to do that you would have to parse the tree and ensure
	# that all nodes wind up in a terminal leaf node, etc.  Maybe
	# that could be added as a function later, but since 
	# trees are generated by HTK they should be ok, unless 
	# the user edits the trees file.

	elsif( $intree and
	       $line =~ /^\s*\}\s*$/ )
	{
	    die "HTKtrees: Invalid tree for $treename\n" 
		unless defined $tree->{NODE}[0];
	    
	    $complete = 1;
	    $intree = 0;
	    last;
	}
	
	# Line is not valid line for a tree
	else
	{
	    die "HTKtrees: Invalid tree for $treename\n";
	}
    }

    # A complete tree was not found
    die "HTKtrees: incomplete tree for $treename\n" unless $complete;
}
