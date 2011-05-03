#================================================================
#  Module TIesrNet.pm

#  Copyright (C) 2010 Texas Instruments Incorporated -  http://www.ti.com/ 
#  ALL RIGHTS RESERVED


#  This module creates an object to encapsulate a TIESR fixed point
#  grammar network. The module creates the object and provides methods
#  to parse the network and expose the components of the network.  The
#  public interface to this object is given below.  Unlike many
#  modules, the output object structure is provided for use by
#  applications so that the network can be manipulated.
#  
#  In the network, symbols are nodes of the grammar that correspond to 
#  a specific hmm in that grammar context of the FSA.  The FSA has
#  HMMs tied to symbols (nodes) rather than transitions between nodes.


#  The object has the following structure
#  $net->{NUMHMMS} = $number of HMMs in grammar
#  $net->{NUMSYMS} = $number of symbols (nodes) in the grammar
#  $net->{NUMWORDS} = $number of words in the grammar
#  $net->{NUMSETS} = $number of HMM sets in grammar (male, female, etc)
#  $net->{NUMPERSET} = $number of HMMs per set
#  $net->{STARTSYMS} = @array of start symbols of the grammar
#  $net->{ENDSYMS} = @array of end symbols of the grammar
#  $net->{SYMBOLS} = @array of symbol structures representing symbols in the grammar

#  The symbol structure is as follows
#  $sym->{HMMCODE} = $code index of the HMM corresponding to this symbol
#  $sym->{HMMNAME} = $name of the HMM if the hmm list is supplied, see below
#  $sym->{NUMTRANS} = $number of transitions from symbol
#  $sym->{TRANS} = @array of transition symbols this symbol transitions TO
#  $sym->{WORDSYM} = $boolean indicating this symbol has a word tag
#  $sym->{WORDINDEX} = $index of the word this HMM corresponds to
#  $sym->{WORDNAME} = $name of the word if word list file supplied, see below


#  The object has the following available public interface methods

#  Constructor for a reference to a tmf object.
#  $net = TIesrNet->new() 

#  Parse a TIesrFlex output net.bin file file into the object structure. undef for the
#  $word_list_file, $hmm_list_file, and $hmm2phonefile is allowed.
#  If $hmm_list_file and $hmm2phonefile are supplied, and $addsil
#  is set to non-zero, then the _SIL model will be added at the
#  beginning of the phone list, as TIesrFlex does.
#  $net->parsenet( $net_file, $word_list_file, $hmm_list_file, $hmm2phonefile, $addsil )

#  Output a grammar network.  If $little_endian is true output will
#  be in little endian format.
#  $net->output( $netfile, $little_endian);


#  Parse a phrase according to the present grammar.  There must be
#  word information available when the network was parsed in order
#  to parse a phrase according to the network.  The returned value
#  is a an array of references to arrays that contain valid 
#  symbol index sequences making up the parse.  Note that there could
#  be multiple sequences, since there may be multiple pronunciations
#  for words.  If a symbol loops, only a single instance of the symbol
#  index will be included in the parse.
#  @sequences = $net->phraseparse( $phrase );

#----------------------------------------------------------------

#  Package header.  Now nothing exported.

package TIesrNet;

use strict qw( vars refs );

use Exporter ();
@TIesrNet::ISA = qw( Exporter );


#----------------------------------------------------------------
#  new

#  Construct a new TIesrNet hash data structure and return a reference
#  to it.  At this time the constructor does not provide for 
#  cloning by passing a reference to a blessed TIesrNet object.
#------------------------------------------------------------
sub new
{
    my $class = shift;
    my $this = {};
    return bless $this, $class;
}


#----------------------------------------------------------------

#  parsenet

#  This function parses a network into a structure.

#----------------------------------------------------------------
sub parsenet
{
    my( $net, $netfile, $wordfile, $hmmlist, $hmm2phone, $addsil ) = @_;

    open( NET, "$netfile") or die "Invalid net file $netfile";

    # Get heading data.  Determine if big/little endian and get header data 
    my( $buf, @netdata );
    my( $pks );
    read NET, $buf, 12;
    @netdata = unpack "n*", $buf;
    if( $netdata[4] & 0xff00 )
    {
	$pks = "v*";
    }
    else
    {
	$pks = "n*";
    }
    my $numwords;
    ($numwords, 
     $net->{NUMHMMS},
     $net->{NUMSYMS},
     $net->{NUMWORDS},
     $net->{NUMSETS},
     $net->{NUMPERSET} ) = unpack $pks, $buf;

    
    # skip past offsets
    read NET, $buf, ($net->{NUMSYMS} + 2)*2;
    # @offsets = unpack $pks, $buf;


    #read start symbols
    my( $hmmcode, $numsyms, @startsyms );
    read NET, $buf, 4;
    ($hmmcode, $numsyms) = unpack $pks, $buf;

    read NET, $buf, ($numsyms*2);
    @startsyms = unpack $pks, $buf;
    $net->{STARTSYMS} = \@startsyms;

    #read ending symbols
    my( @endsyms );
    read NET, $buf, 4;
    ($hmmcode, $numsyms) = unpack $pks, $buf;

    read NET, $buf, ($numsyms*2);
    @endsyms = unpack $pks, $buf;
    $net->{ENDSYMS} = \@endsyms;
    
    # if hmm list exists, build array of hmm names
    my @hmms;
    if( -f $hmmlist )
    {
	# If hmm2phone file exists, get the indices of the phones in the phone list
	my @hmmindices;
	if( -f $hmm2phone )
	{
	    open( LIST, $hmm2phone ) or die "Could not open hmm2phone.bin file\n";
	    read LIST, $buf, $net->{NUMPERSET}*2;
	    @hmmindices = unpack $pks, $buf;
	    close( LIST );

	    foreach my $hindex (@hmmindices)
	    {
		$hindex = $hindex - 2**16  if( $hindex >= 2**15 );
		$hindex++;
	    }
	}
	    
	open( LIST, $hmmlist ) or die "Could not open HMM list $hmmlist\n";

	@hmms = <LIST>;
	chomp @hmms;
	
	close( LIST );

	unshift @hmms, "_SIL" if ($addsil);

	@hmms = @hmms[@hmmindices] if( @hmmindices );
    }
    else
    {
	undef @hmms;
    }


    # if word list exists, build array of hmm names
    my @words;
    if( -f $wordfile )
    {
	open( LIST, $wordfile ) or die "Could not open word list $wordfile\n";

	@words = <LIST>;
	foreach my $wrd (@words)
	{
	    $wrd =~ s/\s+$//;
	}
	
	close( LIST );
    }
    else
    {
	undef @words;
    }


    # read all network symbol information
    my $sym;
    for( $sym=0; $sym < $net->{NUMSYMS}; $sym++ )
    {
	read NET, $buf, 4;
	($hmmcode, $numsyms) = unpack $pks, $buf;
	$net->{SYMBOLS}[$sym]{HMMCODE} = $hmmcode;
	$net->{SYMBOLS}[$sym]{HMMNAME} = $hmms[$hmmcode] if @hmms;

	$net->{SYMBOLS}[$sym]{NUMTRANS} = ($numsyms & 0x7fff);
	$net->{SYMBOLS}[$sym]{WORDSYM} = ($numsyms & 0x8000);

	read NET, $buf, ($numsyms & 0x7fff)*2;
	my $r_trans = [];
	@$r_trans = unpack $pks, $buf;
	$net->{SYMBOLS}[$sym]{TRANS} = $r_trans;

	if( $net->{SYMBOLS}[$sym]{WORDSYM} )
	{
	    my( $wordindex );
	    read NET, $buf, 2;
	    $wordindex =  unpack $pks, $buf;
	    $net->{SYMBOLS}[$sym]{WORDINDEX} = $wordindex;
	    $net->{SYMBOLS}[$sym]{WORDNAME} = $words[$wordindex] if @words;
	}
    }
	
    close(NET);
}


#----------------------------------------------------------------
# output

# This function outputs a network in either little or big endian 
# format.
#----------------------------------------------------------------
sub output
{
    my( $net, $netfile, $little ) = @_;
    
    open( NET, ">$netfile" ) or die "Could not open net file $netfile\n";
    
    my $outs = $little ? "v*" :  "n*";

    
    # Size of file data in short, dummy for now, filled in later
    my $fsize = 0;
    my $buf;
    $buf = pack $outs, 0;
    print NET $buf;


    # header information
    $buf = pack $outs, ( $net->{NUMHMMS},
			 $net->{NUMSYMS},
			 $net->{NUMWORDS},
			 $net->{NUMSETS},
			 $net->{NUMPERSET} );
    print NET $buf;
    $fsize += 5;

    
    # reserve space for offsets
    my $offidx = 0;
    my @offsets = (0) x ( $net->{NUMSYMS} + 2 );
    $buf = pack $outs, @offsets;
    print NET $buf;
    $fsize += $net->{NUMSYMS} + 2;


    # output start symbols
    $offsets[$offidx++] = $fsize;
    $buf = pack $outs, ( -2, scalar(@{$net->{STARTSYMS}}), @{$net->{STARTSYMS}} );
    print NET $buf;
    $fsize += scalar( @{$net->{STARTSYMS}} ) + 2;

    
    # output end symbols
    $offsets[$offidx++] = $fsize;
    $buf = pack $outs, ( -2, scalar( @{$net->{ENDSYMS}} ), @{$net->{ENDSYMS}} );
    print NET $buf;
    $fsize += scalar( @{$net->{ENDSYMS}} ) + 2;
    

    #output all symbol information
    my $sym;
    for( $sym=0; $sym < $net->{NUMSYMS}; $sym++ )
    {
	$offsets[$offidx++] = $fsize;

	my $r_sym;
	$r_sym = $net->{SYMBOLS}[$sym];
	
	my $numtrans;
	$numtrans = $r_sym->{NUMTRANS};
	$numtrans |= 0x8000 if $r_sym->{WORDSYM};

	$buf = pack  $outs, ( $r_sym->{HMMCODE}, $numtrans, @{$r_sym->{TRANS}} );
	print NET $buf;
	$fsize += scalar( @{$r_sym->{TRANS}} ) + 2;

	if( $r_sym->{WORDSYM} )
	{
	    $buf = pack  $outs, $r_sym->{WORDINDEX};
	    print NET $buf;
	    $fsize++;
	}
    }
    
    # Now write file size and offsets
    seek NET, 0, 0;
    $buf = pack $outs, $fsize;
    print NET $buf;

    seek NET, 5*2, 1;
    $buf = pack $outs, @offsets;
    print NET $buf;

    close(NET);
}



#----------------------------------------------------------------
# phraseparse
#
# This subroutine parses a phrase and returns an array or arrays
# giving all of the parse values.  If the returned reference is
# undef, then the phrase does not parse.
#----------------------------------------------------------------
sub phraseparse
{
    my( $net, $phrase) = @_;

    # list of valid sequences, a list of sequence lists
    my (@seqs);
    undef @seqs;

    # Each individual word of the phrase
    $phrase = uc $phrase;
    my @words = split( " ", $phrase );


    # Extend the parse for each word
    foreach my $word (@words)
    {
	# Extend the sequences for this word
	@seqs = expand_seqs($net, $word, @seqs);

	# If at any time no valid sequences remain, return 
	# the empty array
	if( scalar(@seqs) == 0 )
	{
	    return @seqs;
	}
    }


    # Final expansion to include ending silences and ensure
    # all ending symbols are stop symbols
    @seqs = expand_final($net, @seqs);


    # At the end of parsing, return the array of valid symbol sequences
    return @seqs;
}


#----------------------------------------------------------------
# expand_seqs 
#
# Expand the sequences that are presently valid until they get to the 
# next word.  This will create new sequences if multiple paths are 
# available from a symbol.  If the sequence ends up at a word that is
# not the word of interest, then the sequence is invalid, and is removed
# from the sequence list.  A symbol with _SIL is treated as a symbol
# that is not a word-ending symbol.
#----------------------------------------------------------------
sub expand_seqs
{
    my($net, $word, @seqs) = @_;

    my(@priorseqs);

    my $doexpand = 1;
    my $excount = 0;
    while( $doexpand )
    {
	$doexpand = 0;

	# if at start then initialize sequences
	if( scalar(@seqs) == 0 )
	{
	    foreach my $startidx ( @{$net->{STARTSYMS}} )
	    {
	        my $rsym = $net->{SYMBOLS}[$startidx];
		my $wordsym = $rsym->{WORDSYM};
		my $symname = $rsym->{WORDNAME};
		$symname =~ s/\+\d+// if $symname;
		$symname = uc $symname if $symname;

		# if starting symbol is not word symbol or is _SIL, then
		# the sequence is valid, and further expansion is necessary
		if( ! $wordsym or $symname eq "_SIL" )
		{
		    # start a new sequence list, and put it in list of sequences 
		    push @seqs, [$startidx];
		    $doexpand = 1;
		    next;
		}

		# if the starting symbol explains the current word then
		# the sequence is valid but expansion need not continue
		if( $wordsym and $symname eq $word )
		{
		    # start a new sequence list, and put it in list of sequences 
		    push @seqs, [$startidx];
		}
	    }
	}

	# expand existing sequences
	else
	{
	    # store prior sequences, rebuild expanded sequences
	    @priorseqs = @seqs;
	    undef @seqs;

	    # Expand prior sequences to the next word symbol
	    foreach my $rseq ( @priorseqs )
	    {
		my $lastsymidx = $$rseq[ $#{$rseq} ];
		my $rsym = $net->{SYMBOLS}[$lastsymidx];
		my $rtrans = $rsym->{TRANS};
		my $ntrans = $rsym->{NUMTRANS};
		my $wordsym = $rsym->{WORDSYM};
		my $symname = $rsym->{WORDNAME};
		$symname =~ s/\+\d+// if $symname;
		$symname = uc $symname if $symname;

		# if a sequence that explains the current word has been found,
		# maintain it but do not request further expansion
		if( $wordsym and  $symname eq $word and $excount > 0 )
		{
		    # remember this sequence as a current sequence
		    push @seqs, $rseq;
		    next;
		}

		# try to expand this sequence to explain the present word
		foreach my $transymidx ( @{$rtrans} )
		{
		    # do not expand loop transitions
		    next if( $transymidx == $lastsymidx);

		    my $rtsym = $net->{SYMBOLS}[$transymidx];
		    my $twordsym = $rtsym->{WORDSYM};
		    my $tsymname = $rtsym->{WORDNAME};
		    $tsymname =~ s/\+\d+// if $tsymname;
		    $tsymname = uc $tsymname if $tsymname;

		    # Expand if the transition symbol is a word symbol
		    # that does explain current word, but no need to
		    # continue expansion
		    if(  $twordsym and $tsymname eq $word )
		    {
			# add symbol to sequence list and save as new sequence
			push @seqs, [ @{$rseq}, $transymidx ];
			next;
		    }
		    		    
		    # Expand if the transition symbol is a non-word symbol,
		    # or is a silence symbol. Expansion should continue.
		    if( ! $twordsym or $tsymname eq "_SIL" )
		    {
			# add symbol to sequence list and save as new sequence
			push @seqs, [ @{$rseq}, $transymidx ];
			$doexpand = 1;
		    }

		} # foreach transsymidx

	    } # foreach $rseq (prior sequence)

	} # expand else

	# return immediately if expansion yields no sequences
	return @seqs if ( scalar(@seqs) == 0 );

	# another expansion cycle has taken place
	$excount++;

    } # while doexpand

    return @seqs;
}




#----------------------------------------------------------------
# final_expand
#
# All sequences input to final_expand explain the words in the
# phrase exactly.  final_expand checks these sequences to ensure that
# they ar valid stop sequences with a valid stop symbol.  Further searching
# is done to include sequences of _SIL symbols at the end of the
# phrase that end with a stop symbol.
#----------------------------------------------------------------
sub expand_final
{
    my($net, @seqs) = @_;

    my(@finalseqs);
    my(@toexpandseqs);

    # make a hash of end symbol indices
    my(%endsymhash);
    @endsymhash{ @{$net->{ENDSYMS}} } = ( @{$net->{ENDSYMS}} );


    undef @finalseqs;

    # Initial check of input sequences
    foreach my $rseq (@seqs)
    {
	my $lastsymidx = $$rseq[ $#{$rseq} ];
	my $rsym = $net->{SYMBOLS}[$lastsymidx];
	my $rtrans = $rsym->{TRANS};
	my $ntrans = $rsym->{NUMTRANS};
	my $wordsym = $rsym->{WORDSYM};
	my $symname = $rsym->{WORDNAME};
	$symname =~ s/\+\d+// if $symname;
	$symname = uc $symname if $symname;

	# if the input sequence ends at a stop symbol, then it is a 
	# valid final sequence explaining the phrase
	push @finalseqs, $rseq if( exists $endsymhash{$lastsymidx} );

	# All sequences should be checked for trailing _SIL
	push @toexpandseqs, $rseq;
    }


    # Expand all sequences to include final _SIL
    while( scalar(@toexpandseqs) )
    {
	@seqs = @toexpandseqs;
	undef @toexpandseqs;

	# Process each final symbol of sequences to expand
	foreach my $rseq ( @seqs )
	{
	    my $lastsymidx = $$rseq[ $#{$rseq} ];
	    my $rsym = $net->{SYMBOLS}[$lastsymidx];
	    my $rtrans = $rsym->{TRANS};
	    my $ntrans = $rsym->{NUMTRANS};
	    my $wordsym = $rsym->{WORDSYM};
	    my $symname = $rsym->{WORDNAME};
	    $symname =~ s/\+\d+// if $symname;
	    $symname = uc $symname if $symname;
		
	    # check if the sequence should be expanded by trailing _SIL
	    foreach my $transymidx ( @{$rtrans} )
	    {
		# do not expand loops
		next if $transymidx == $lastsymidx;
	    
		my $rtsym = $net->{SYMBOLS}[$transymidx];
		my $twordsym = $rtsym->{WORDSYM};
		my $tsymname = $rtsym->{WORDNAME};
		$tsymname =~ s/\+\d+// if $tsymname;
		$tsymname = uc $tsymname if $tsymname;

		# expand only if transition is to _SIL
		if( $twordsym and $tsymname eq "_SIL" )
		{
		    # new sequence with an ending _SIL
		    my $newseq = [ @{$rseq}, $transymidx ];
		    push @toexpandseqs, $newseq;

		    # if the _SIL is an ending _SIL, then this is a
		    # valid final sequence
		    push @finalseqs, $newseq if( exists $endsymhash{$transymidx} );
		}

	    } #foreach trans

	} #foreach rseq prior sequence
	
    } #while expansion cycle
    
    return @finalseqs;
}
