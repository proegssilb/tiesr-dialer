#!/usr/bin/perl
#---------------------------------------------------------------
#  odtcompress.pl

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


$USAGE = "
odtcompress -Help -D treedir -CF conflevel -o outdir [-B]";


$DOCUMENTATION = "
This script generates compressed binary decision trees based on the 
output of the C4.5 log files that contain the text version of
the trees.

The -D option specifies the directory which holds the c4.5 log files
needed for compression. The -CF option specifies the confidence level
used during c4.5 program processing, and this indicates which set of
*.CF*log files will be compressed. The -o option specifies the output
directory for the final compressed decision trees. This directory must
also contain the three files cAttType.txt, cAttValue.txt, and
onewphone.lst, which should have been created by prior steps of
processing. The compressed *.olmdtpm files will also be output to this
directory. If the -B option is selected, output will be in big-endian
format.";

#---------------------------------------------------------------

use Getopt::Long;

GetOptions( "Help" => \$help, 
            "D=s" => \$treedir,
            "o=s" => \$outdir,
            "CF=i" => \$conflevel,
	    "B" => \$bigendian );

if( $help )
{
    print "$USAGE\n\n";
    print "$DOCUMENTATION\n";
    exit 1;
}


if( ! -d $outdir or ! -d $treedir or 
    ($conflevel < 1 or $conflevel > 100 ) )
{
    print "$USAGE\n\n";
    print "$DOCUMENTATION\n";
    die "odtcompress: Invalid outdir\n";
}


# Read the decision tree attribute names
open( ATT, "<$outdir/cAttType.txt" ) or die "odtcompress: Missing file cAttType.txt\n";
@attributes = <ATT>;
close( ATT );
foreach $att (@attributes)
{
    chomp $att;
    $att =~ s/\s*//g;
}

# Form hash used to look up index corresponding to attribute name
@attributes{@attributes} = (0..$#attributes);
$attributes{NULL_ATTRIBUTE} = 15;


# Read the values that the attributes can assume
open (VAL, "<$outdir/cAttValue.txt" ) or die "odtcompress: Missing file cAttValue.txt\n";
@values = <VAL>;
close( VAL );
foreach $val (@values)
{
    chomp $val;
    $val =~ s/\s*//g;
}

# Form hash to look up index to corresponding value
@values{@values} = (0..$#values);


# Read the decision tree phone classes
open( CLS, "<$outdir/onewphone.list" ) or die "odtcompress: Missing file onewphone.list\n";
@phones = <CLS>;
close( CLS );
foreach $phone (@phones)
{
    chomp $phone;
    ($phone) = $phone =~ /^\s*([^\s]+)\s/;
}

#Make a hash to look up index to phone class
@phones{@phones} = (0..$#phones);


# Ensure log file exists
foreach $logfile ( glob( "${treedir}/*.CF${conflevel}log" ) )
{

    die "odtcompress: Invalid file $logfile\n" unless -f $logfile;

    # Form empty hash to track mapping of attribute
    # values to each attribute index
    $attmap = {};


    #Parse the log file to produce a tree structure to be compressed
    $rtree = parse_file( $logfile );

    # Get the file name of the logfile, which will be used as the
    # filename for the compessed tree file
    $logfile =~ /^(.*\/)?([^.]+)\./;
    $lettername = $2;

    $treefile = "$outdir/${lettername}.olmdtpm";

    # Compress the tree and output to directory
    output_tree( $rtree, $treefile );

    # Delete the parsed tree structure
    undef $rtree;
    undef $attmap;
}



#----------------------------------------------------------
# parse_file
#
# Parse a c4.5 output log file and extract the simplified
# decision tree
#----------------------
sub parse_file
{
    my( $fname ) = shift @_;

    open( DT, $fname ) or die "odtcompress: Could not open $fname\n";

    # Read until the decision tree is found. This may be
    # the simplified decision tree, or if not, it may
    # indicate a single phone output tree.
    my $location = tell DT;
    while( $lin = <DT> )
    {
	if( $lin =~ /^\s*(Simplified )?Decision Tree:\s*$/ )
	{
	    $location = tell DT;
	}
    }

    # Go to start of decision tree
    seek DT, $location, 0;
    
    # Go past any blank lines
    while( $lin = <DT> )
    {
	last if $lin !~ /^\s*$/;
    }
    


    # Create the root node of the tree, with no parent
    # and at level 0
    my $rnode = init_node( undef, 0 );

    # Parse the root node.  This will recursively parse the
    # whole tree.
    $lin = parse_node( \*DT, $lin, $rnode );


    close( DT );

    return $rnode;
}


#--------------------------------------------------------
# init_node
#
# Initialize a node of the decision tree
#-------------------
sub init_node
{
    my $rparent = shift @_;
    my $level = shift @_;

    # Create a reference to a new anonymous hash for the node
    my $refnode = {};


    $$refnode{PARENT} = $rparent;
    $$refnode{LEVEL} = $level;

    return $refnode;
}



#---------------------------------------------------------
# parse_node
#
# Parse the simplified c4.5 generated decision tree node.
#-----------------
sub parse_node
{
    local $FH = shift;
    my $lin = shift;
    my $curnode = shift;
    my ($level, $curlevel, $type, $att, $val, $phone );


    # Parse the first line of the node
    ($level, $type, $att, $val, $phone ) = parse_lin( $lin );
    $curlevel = $level;

    # Save the index of the attribute of the node
    my $attidx = $attributes{$att};
    $$curnode{ATT} = $attidx;


    # Loop over all lines for this node level
    while( $lin !~ /^\s*$/ && $level == $curlevel )
    {
	# Enter value index mapped to the attribute into mapping hash
	$$attmap{$attidx}{$values{$val}} = 1;

	# Value is a branch value; decend into branch node and parse it
	if( $type eq "BRANCH" )
	{
	    # Create a new node for the branch at next level down
	    my $newnode = init_node( $curnode, $level+1);

	    # Save new node in branch value index hash of current node
	    $$curnode{BRANCH}{$values{$val}} = $newnode;

	    # Get the first line of the new node
	    $lin = <$FH>;
	    chomp $lin;

	    # Parse the new node, returning line after all lower level nodes
	    $lin = parse_node( $FH, $lin, $newnode );
	}


	# Value is a leaf value; save value index and its corresponding phone index
	elsif( $type eq "LEAF" )
	{
	    
	    # Save phone index to output corresponding to this value index
	    $$curnode{LEAF}{$values{$val}} = $phones{$phone};

	    # Get the next line
	    $lin = <$FH>;
	    chomp $lin;
	}


	# If not a blank line, parse it to determine if the line belongs
	# to current or higher level node. This will indicate end if the
	# line belongs to current node or ancestor node.
	if( $lin !~ /^\s*$/ )
	{	    
	    ($level, $type, $att, $val, $phone ) = parse_lin( $lin );
	}
    }

    return $lin;
}
   


#----------------------------------------------------------
# parse_lin
#
# Parse a line of the decision tree in the c4.5 log file
#---------------------
sub parse_lin
{
    my $lin = shift;
    my @plin;

    chomp $lin;

    my ($level, $att, $val, $phone, $type);
    my ( @parse );

    # Return undef if blank line
    return undef if $lin =~ /^\s*$/;


    # Determine the level of the node
    @parse = $lin =~ /^((\|\s+)*)(.*)$/;
    $level = scalar( split( " ", $1 ) );
    $lin = pop @parse;
    
    # Determine the attribute, value, and phone if leaf node
    $lin =~ /\s*([^\s]+)(\s*=\s*([^\s]+)\s*:\s*([^\s]*))?/;

    # Check if this is a single phone tree
    if( $2 eq "" )
    {
	# Null attribute and value
	$att = "NULL_ATTRIBUTE";
	$val = "NO_ATTRIBUTE";

	$phone = $1;
	$type = "LEAF";
    }
    else
    {

	$att = $1;

	$val = $3;

	# Special cases of values modified for c4.5  program
	if( $val eq "no" ) { $val = "NO_ATTRIBUTE" }
	elsif( $val eq "up" ) { $val = "'" }
	elsif( $val eq "dash" ) { $val = "-" }
	elsif( $val eq "dot" ) { $val = "." }


	$phone = $4;
	$type = ( $phone eq "" ) ?  "BRANCH" : "LEAF";
    }

    return ($level, $type, $att, $val, $phone );
}


#-----------------------------------------------------------
# encode_hash
#
# Encode a hash into a compressed list for linear processing efficiency.
# Each integer key in the hash has an associated
# integer value. There will often be many keys that have the
# same value. The value that occurs most often is the default value
# and the keys do not need to be encoded for the default value.
# The remaining keys are encoded by the value they share in order of 
# decreasing number of shared values. The integer values and keys
# are output in an array in compressed format.
#----------------------
sub encode_hash
{
    my $rhash = shift @_;


    # Build a list of keys corresponding to each value

    my ($key, $val );
    my %valkeys;
    foreach $key ( sort {$a <=> $b} keys %$rhash )
    {
	$val = $$rhash{$key};

	$valkeys{$val} = [] if ! defined $valkeys{$val};

	# The valkeys hash contains a list of keys for each value.
	push @{$valkeys{$val}}, $key;
    }


    # Determine the number of keys corresponding to each value
    my %numkeys;
    foreach $val ( keys %valkeys )
    {
	$numkeys{$val} = scalar( @{$valkeys{$val}} );
    }

    # Sort the values by the number of keys corresponding to the value.
    # used.
    my @sortvals = sort { $numkeys{$b} <=> $numkeys{$a} } keys %valkeys;


    # Default value is the value that occurs most often
    my $defaultval = shift @sortvals;


    # Compress the remaining keys corresponding to values in order
    # of most often occurring values
    my @outlist = ();
    foreach $val ( @sortvals )
    {
	# Output the value and number of keys having the value
	push @outlist, $val;
	push @outlist, $numkeys{$val};


	# Output compressed list of keys corresponding to this 
	# value. Note that @{$valkeys{$val}}
	# is a sorted list of keys corresponding to the value.
	my @encodekeys = encode_list( @{$valkeys{$val}} );
	
	push @outlist, @encodekeys;

    }

    # Output default value corresponding to all other keys
    push @outlist, $defaultval;


    # Output length of the list as first value
    unshift @outlist, ( scalar(@outlist) + 1 );

    return @outlist;
}


#----------------------------------------------------------
# encode_list
#
# Encode a list of integers into a run-length compressed list
# Input is a list. Encoding of the list encodes sequential 
# integers as the first integer, followed by the negative of
# the last sequential integer. For example, the list
# ( 1, 3, 5, 6, 7, 8, 10) would be encoded as:
# ( 1, 3, 5, -8, 10 )s
#------------------
sub encode_list
{
    my ( $item, $lastitem, $inrun );
    my @encodelist;



    # Run encode the sorted list
    my $lastitem = undef;
    my $inrun = 0;
    foreach $item ( sort {$a <=> $b} @_ )
    {
	# next integer or start of new run of sequential integers
	if( $lastitem == undef or $lastitem < $item-1 )
	    {
		if( $inrun )
		{
		    push @encodelist, ( $lastitem | 0x80 );
		    $inrun = 0;
		}

		push @encodelist, $item;
		$lastitem = $item;

	    }

	    # continuation of run of key integers
	    elsif( $lastitem == $item-1 )
	    {
		$lastitem = $item;
		$inrun = 1;
	    }
	    
	    else
	    {
		# Sanity check, should not happen
		die "odtcompress: encode_list items out of order\n";
	    }
	}
    
    # Ended in run of items, output end of run
    if( $inrun )
    {
	push @encodelist, ( $lastitem | 0x80);
    }

    return @encodelist;
}



#-----------------------------------------------------------
# output_tree
#
# Output the decision tree in compressed format for use by
# the TIesrDT API.
#------------------------
sub output_tree
{
    my( $rtree, $fname ) = @_;


    # Recursively encode the data of the nodes. This also allows
    # calculation of the size of each node in bytes. The encoded
    # data and the sizes are added to the tree hash
    encode_node( $rtree );


    # Open output file
    open( TRE, ">$fname" ) or die "odtcompress: Could not open $fname\n";


    # Output tree header information
    # Endian packing template
    my $temp =  $bigendian ?  "N" : "V";


    # Tree header information

    # Output size of top node plus node context info
    my $size = $$rtree{SIZE} + 2;
    print TRE pack( $temp, $size );


    # Output number of attributes in byte
    print TRE  pack( "C", scalar(@attributes) );

    # Output number of values mapped to each attribute index
    my $attidx;
    foreach $attidx ( (0 .. $#attributes) )
    {
	my @attvalues = sort keys %{ $$attmap{$attidx} };
	print TRE pack( "C", scalar(@attvalues) );
    }


    # Output the tree nodes depth first this function will
    # recursively output all nodes of the tree
    output_node( \*TRE, $rtree );

    close(TRE);
}

#---------------------------------------------------------------
# encode_node
#
# Encode the branch, leaf and header data for a node. This data is added
# to the tree hash. The size of each node is also calculated and 
# propagated up the tree so that node sizes (offsets) are included
# in the header encoding.
#--------------------------
sub encode_node
{
    my $node = shift;
    my $nodeflag;
    my @header = ();
    my @branchencode;
    my @branchvalues;
    my @leafencode;

    # Initialize node size in bytes
    $$node{SIZE} = 0;

    # If node contains a reference to a leaf hash, then it has
    # leaves, so they can be encoded.
    if( defined $$node{LEAF} )
    {
	# Encode the node attribute values and corresponding
	# output phones.
	@leafencode = encode_hash( $$node{LEAF} );
	
	# Put the result in an anonymous list of the leaf
	$$node{LEAFENCODE} = [@leafencode ];
	$$node{SIZE} += scalar( @leafencode );

	# Set node flag to indicate only leaf nodes
	$nodeflag = 0x80;
    }

    # If the node contains a reference to a branch hash, then
    # it has branch sub-nodes. Decend and encode the branch sub-nodes.
    if( defined $$node{BRANCH} )
    {
	foreach $branchnode ( values %{$node->{BRANCH}} )
	{
	    encode_node( $branchnode );

	    $$node{SIZE} += $$branchnode{SIZE};
	}

	# Encode compressed information regarding the 
	# branch attribute values that lead to sub-nodes.
	@branchvalues = sort {$a <=> $b} keys %{$node->{BRANCH}};
	@branchencode = encode_list( @branchvalues );

	# Override node flag to indicate node has branches
	$nodeflag = 0xc0;
    }


    # Node header information
    if( $nodeflag == 0x80 )
    {
	# If not a single phone tree, create leaf node only header byte
	if ( $$node{ATT} != $attributes{NULL_ATTRIBUTE} )
	{
	    # Node type info for a node that has only leaves.
	    # Inculdes flag and node attribute.
	    push @header,  $nodeflag | $$node{ATT};
	}
    }
    else
    {
	# Node type info for a node that has branches. Includes
	# flag and size of encoded attribute values.
	my $nodetype;
	$nodetype =  $nodeflag  | ( scalar(@branchencode) + 1 );

	push @header, $nodetype;

	# Encoded node attribute values that have branches
	push @header, @branchencode;
	push @header, scalar( keys %{$node->{BRANCH}} );

	# Header sub-node information for each attribute value
	# corresponding to a sub-node put in the header
	# in the order encoded in @branchencode
	my ( $val );
	foreach $val ( @branchvalues )
	{
	    my $childnode;
	    my $cnodeinfo;
	    $childnode = $$node{BRANCH}{$val};

	    $cnodeinfo = $$childnode{ATT};
	    $cnodeinfo = ($cnodeinfo << 12 ) | $$childnode{SIZE};
	    push @header, ($cnodeinfo >> 8 ) & 0xff;
	    push @header, ($cnodeinfo & 0xff );
	}
    }

    # Node size includes header
    $$node{SIZE} += scalar( @header );
    $$node{HEADER} = [@header];
}


#-----------------------------------------------------------
# output_node
#
# Output node for the tree, decending to other nodes as needed.
#--------------------------
sub output_node
{
    my( $FH, $node ) = @_;

    # If this is top node, output root branch  attribute context
    # and offset info. Even though the offset info will not be
    # used, it gives the size of the top node.
    if( $$node{LEVEL} == 0 )
    {
	my $cxt = $$node{ATT};
	$cxt = $cxt << 12;

	# Size not needed for top node
	# $cxt = $cxt + $$node{SIZE};

	print $FH pack "C",  ( ( $cxt >> 8 ) & 0xff );
	print $FH pack "C",  ( $cxt & 0xff );
    }    

    # Output node header information
    my $byte;
    foreach $byte ( @{ $$node{HEADER} } )
    {
	print $FH pack "C", ($byte & 0xff);
    }    


    # Output encoded leaf info for the node, if any
    if( defined $$node{LEAFENCODE} )
    {
	foreach $byte ( @{ $$node{LEAFENCODE} } )
	{
	    print $FH pack "C", ($byte & 0xff);
	}	    
    }

    # Output info for all sub-nodes recursively
    if( defined $$node{BRANCH} )
    {
	# Loop over all attribute values corresponding to sub-nodes
	# in the order in which they were encoded
	my $val;
	foreach $val ( sort {$a <=> $b} keys %{ $node->{BRANCH} } )
	{
 	    my $subnode = $$node{BRANCH}{$val};
	    output_node( $FH, $subnode );
	}
    }
}
