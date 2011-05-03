#!/usr/bin/perl

#----------------------------------------------------------------
#  htk_letter_mmf.pl

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


$USAGE = "
htk_letter_mmf -p phonefile -h letterhmmfile -o outputmmf -m modelmapfile";


$DOCUMENTATION = " 
This script creates an HTK MMF file for letters. The -p option is a
file containing the ordered list of phones in the language, the -h
option specifies a file containing a description of each letter hmm.
The format of the latter file is one line for each letter hmm.  The
line consists of the letter followed by a ':' followed by phone lists
and probabilities for the phone list separated by ','. Arbitrary
numbers of spaces may be included in the file.  Lines beginning with
'#' are comment lines.  The special phone '_sil' indicates a tee
transition directly from the starting state to the ending state. An
example is the letter X:

X : z 0.7 , k s 0.2 , _sil 0.1

This states that the letter hmm X should consist of a state z with
entry probability 0.7, a two state sequence k s with entry into the k
state of 0.2, and a tee probability of 0.1 (going directly from the
starting to ending state). The -o option specifies where to output the
mmf file. The -m option specifies where to output a map file that
defines the phone for each state of each letter HMM for use in subsequent
recognition to determine letter-to-phone mapping.";

#----------------------------------------------------------------
use Getopt::Long;


GetOptions( "Help" => \$help,
	    "p=s" => \$phonefile,
	    "h=s" => \$lethmmfile,
	    "o=s" => \$outputmmf,
	    "m=s" => \$modelmap
	    );

if( $help )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    exit 1;
}


if( ! -f $phonefile ||
    ! -f $lethmmfile ||
    ! $outputmmf ||
    ! $modelmap )
{
    print "$USAGE\n";
    print "$DOCUMENTATION\n\n";
    die "htk_letter_mmf: Invalid input\n";
}


# Get the ordered phone listing file
open(PHN, "$phonefile") or die "htk_letter_mmf: Can not open $phonefile\n";
while( <PHN> )
{
    chomp;

    # Bypass empty or comment lines
    next if( /^\s*$|^\s*\#/ );

    # Remove spaces in phone name
    s/\s+//g;

    push(@phones, $_);
}
close(PHN);


# Make a phone index hash
$nphones = @phones;
@pindex{@phones} = ( 1 .. $nphones );


# Open the modelmap file that tracks phone/state mapping
open(MMAP, ">$modelmap" ) or die "htk_letter_mmf: Can not open $modelmap\n";


# Construct the mmf file header
open( MMF, ">$outputmmf" ) or die "htk_letter_mmf: Can not open $outputmmf\n";

print MMF "~o <DISCRETE> <STREAMINFO> 1 1\n";


# Create state macros for all phones.  Each state macro has the
# name of the phone, and that state has probability of 1.0 of that
# phone VQ index occurring.  All other indices have lowest probability.
foreach $phone ( @phones )
{
    print MMF "~s \"$phone\"\n";
    print MMF "<NUMMIXES> $nphones\n";

    $idx = $pindex{$phone};

    if( $idx == 1 )
    {
	$nafter = $nphones - 1;
	print MMF "<DProb> 0 32767\*$nafter\n";
    }
    elsif( $idx == $nphones )
    {
	$nbefore = $nphones - 1;
	print MMF "<DProb> 32767\*$nbefore 0\n";
    }
    else
    {
	$nbefore = $idx - 1;
	$nafter = $nphones - $idx;
	print MMF "<DProb> 32767\*$nbefore 0 32767\*$nafter\n";
    }
}


# Output the non-tee null phone model
print MMF "~h \"_NUL\"\n";
print MMF "\<BeginHMM\>\n";
print MMF "\<NumStates\> 3\n";
print MMF "\<State\> 2\n";
print MMF "~s \"_nul\"\n";
printf MMF "\<TransP\> 3\n";
print MMF " 0.0 1.0 0.0\n";
print MMF " 0.0 0.9 0.1\n";
print MMF " 0.0 0.0 0.0\n";
print MMF "<ENDHMM>\n";

print MMAP "_NUL _nul\n";

# Read in all HMM descriptions and write out the hmm macros
open( HMM, "$lethmmfile" ) or die "htk_letter_mmf: Can not open $lethmmfile\n";

while( <HMM> )
{

    next if /^(\s*|\s*\#.*)$/;


    $rlethash = parse_letter($_);
    
    # Begin the letter hmm
    print MMF "~h \"$rlethash->{LETTER}\"\n";
    print MMF "\<BeginHMM\>\n";
    print MMF "\<NumStates\> $rlethash->{NUMSTATES}\n";
    
    print MMAP "$rlethash->{LETTER}";

    # Print emitting state phone state macros 
    for( $st = 2; $st < $rlethash->{NUMSTATES}; $st++ )
    {
	$refst = $rlethash->{STATE}[$st];

	die "htk_letter_mmf: Invalid phone $refst->{NAME}\n" unless exists $pindex{$refst->{NAME}};

	print MMF "\<State\> $st\n";
	print MMF "~s \"$refst->{NAME}\"\n";

	print MMAP " $refst->{NAME}";
    }

    print MMAP "\n";


    # Transition probabilities
    printf MMF "\<TransP\> $rlethash->{NUMSTATES}\n";
    for($st=1; $st<= $rlethash->{NUMSTATES}; $st++ )
    {
	$refst = $rlethash->{STATE}[$st];
	for( $pidx = 1; $pidx <= $rlethash->{NUMSTATES}; $pidx++ )
	{
	    print MMF " $refst->{TRPROB}[$pidx]";
	}
	print MMF "\n";
    }

    print MMF "\<EndHMM\>\n";
}


close( HMM );

close( MMF );

close( MMAP );

exit 0;



#----------------------------------------------------------------
sub parse_letter
{
    my($line) = $_[0];

    # The letter hash
    my(%lethash);

    # Get letter and its corresponding phone sequences
    ($let, $seqinfo) = ( $line =~ /^\s*([^\s]+)\s*:\s*(.*?)\s*$/ );

    $lethash{LETTER} = $let;


    # Default tee probability
    $teeprob = 0.0;

    # Initialize total entry probability to first emitting states
    $totalprob = 0;

    # Define the starting state
    $state = 1;
    $lethash{STATE}[$state]{NAME} = "STARTSTATE";
    $lethash{STATE}[$state]{TRPROB}[1] = 0.0;


    # Extract each sequence as a string and put it as a path on the HMM
    # from starting to ending state
    @seqs = split( /\s*,\s*/, $seqinfo );

SEQS:
    foreach $seq (@seqs)
    {

	# extract sequence elements that make a path from start to end state
	@path = split( " ", $seq );
	die "htk_letter_mmf: Invalid path sequence $seq\n" unless $#path > 0;
	
	# last path element is the entry probability from start state to first phone
	$prob = pop @path;

	die "htk_letter_mmf: Invalid path sequence $seq\n" 
	    unless $prob =~ /^(\d*\.\d+|\d+\.?\d*)((E|e)(\+|-)(\d){1,3})?$/;
	$totalprob += $prob;

	# process each phone in the path
	$first = 1;
	foreach $phname (@path)
	{
	    # handle _sil uniquely; it is not a state, but indicates tee probability
 	    if( $phname =~ /_sil/ )
	    {
		die "htk_letter_mmf: Invalid tee spec\n" if( $#path > 0 );
		$teeprob = $prob;
		next SEQS;
	    }

	    # handle forced null state that is not a skip
	    if( $phname =~ /_nul/ )
	    {
	    	$phname = "_nul";
	    }

	    # First phone in the path has entry probability from start state
	    if( $first )
	    {
		$state++;
		$lethash{STATE}[$state]{NAME} = $phname;

		#Transition from start state to this first state of path
		$lethash{STATE}[1]{TRPROB}[$state] = $prob;
		$first = 0;
	    }

	    # all other phones in sequence chain from the preceding path phone
	    # and need to explain possible intervening non-tee observations
	    else
	    {
		# Transition from prior state to a new non-tee state
		$state++;
		$lethash{STATE}[$state]{NAME} = "_nul";
		$lethash{STATE}[$state-1]{TRPROB}[$state] = 0.5;

		# Non-tee state loop probability
		$lethash{STATE}[$state]{TRPROB}[$state] = 0.5;
		

		# Next phone state in path
		$state++;
		$lethash{STATE}[$state]{NAME} = $phname;

		# Transition from prior phone state in sequence to this state
		$lethash{STATE}[$state-2]{TRPROB}[$state] = 0.5;

		#Transition from new non-tee state to this state
		$lethash{STATE}[$state-1]{TRPROB}[$state] = 0.5;
	    }
	}

	# last phone on the sequence path ties to end state.  Since we don't 
	# know end state yet, tag this by a self transition of -1.0
	$lethash{STATE}[$state]{TRPROB}[$state] = -1.0;
    }

    die "htk_letter_mmf: Invalid probabilities for $let\n" 
	if( $totalprob < .9999 || $totalprob > 1.0001 );


    # Add the ending state
    $state++;

    $lethash{NUMSTATES} = $state;

    $lethash{STATE}[$state]{NAME} = "ENDSTATE";
    for( $st = 1; $st <= $state; $st++ )
    {
	$lethash{STATE}[$state]{TRPROB}[$st] = 0.0;
    }	


    # Complete the transition probabilities

    # The tee probability from first to last state
    $lethash{STATE}[1]{TRPROB}[$state] = $teeprob;

    # Transitions from last path state to ending state
    for( $st = 2; $st < $state; $st++ )
    {
	if( $lethash{STATE}[$st]{TRPROB}[$st] == -1.0 )
	{
	    $lethash{STATE}[$st]{TRPROB}[$state] = 1.0;
	    $lethash{STATE}[$st]{TRPROB}[$st] = 0.0;
	}	
    }

    # All other undefined trans prob. are zero 
    for( $srcst = 1; $srcst < $state; $srcst++ )
    {
	for( $dstst = 1; $dstst <= $state; $dstst++ )
	{
	    if( ! defined $lethash{STATE}[$srcst]{TRPROB}[$dstst] )
	    {
		$lethash{STATE}[$srcst]{TRPROB}[$dstst] = 0.0;
	    }
	}
    }

    return \%lethash;
}
