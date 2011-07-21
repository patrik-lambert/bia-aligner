#! /usr/bin/perl

###########################################################################################
# Author:  Patrik Lambert (lambert@gps.tsc.ucp.es)
# Description: cf -h or -man options
############################################################################################

#use lib "";
use strict;
use Getopt::Long;
use Pod::Usage;
#Debug:
use Dumpvalue;
my $dumper = new Dumpvalue; 

my $TRUE = 1;
my $FALSE = 0;
my $INFINITY = 9999999999;
my $TINY = 1 - $INFINITY / ($INFINITY + 1);

#PARSING COMMAND-LINE ARGUMENTS
my %opts=();
$opts{verbose}=1;
# parse command line
GetOptions(\%opts,'man','help|?','verbose|v=i','svcb=s','tvcb=s') or pod2usage(0);
# check no required arg missing
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(0);
}elsif( ! (exists($opts{"svcb"}) && exists($opts{"tvcb"})) ){   #required arguments
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
#END PARSING COMMAND-LINE ARGUMENTS
my $verbose=$opts{"verbose"};
open(SVCB,"<$opts{svcb}") || die "ERROR opening vocabulary file $!";
my %svocab;
while (<SVCB>){
    chomp;
    my ($index,$word)= split /\t/;
    $svocab{$index}=$word;
}
close(SVCB);
open(TVCB,"<$opts{tvcb}") || die "ERROR opening vocabulary file $!";
my %tvocab;
while (<TVCB>){
    chomp;
    my ($index,$word)= split /\t/;
    $tvocab{$index}=$word;
}
close(TVCB);

my $smallest=0;
my $largest=0;
#PARSE FILE
while (<>){
    chomp;
    s/^\s+//;  #trim
    s/\s+$//;  #trim
    s/\s{2,}/ /g;  #remove multiple spaces

    my @indices=split / \|\|\| /;
    my @sWords=split / /,"$indices[0]";
    for (my $i=0; $i<@sWords;$i++){
	$sWords[$i]=$svocab{"$sWords[$i]"};
    }
    my @tWords=split / /,"$indices[1]";
    for (my $i=0; $i<@tWords;$i++){
	$tWords[$i]=$tvocab{"$tWords[$i]"};
    }
    print join(" ",@sWords)," ||| ",join(" ",@tWords)," ||| ",@indices[2],"\n";
}

#print STDERR "smallest:$smallest largest:$largest\n";
__END__

=head1 NAME

assoScoreTable_int2word.pl - replaces each word by its index in a .vcb file passed as argument.

=head1 SYNOPSIS

perl word2int.pl [options] -svcb src_voc_file -tvcb trg_voc_file < table_indices > table_words
    
    svcb    source vocabulary file name in TALPwhittle .vcb format
    tvcb    target vocabulary file name in TALPwhittle .vcb format

Options:

    -v 0|1    0:silent 1:verbose [default 0]
    -help|?    Prints the help and exits
    -man    Prints the manual and exits

=head1 COPYRIGHT AND LICENSE

Copyright 2006 by Patrik Lambert

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (version 2 or any later version).

=cut
