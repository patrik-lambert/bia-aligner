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
GetOptions(\%opts,'man','help|?','st=s','ts=s','us=s','ut=s','verbose|v=i') or pod2usage(0);
# check no required arg missing
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(0);
}elsif( !(exists($opts{"st"}) && exists($opts{"ts"})) ){   #required arguments
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
#END PARSING COMMAND-LINE ARGUMENTS
my $verbose=$opts{"verbose"};

#open input file
open(ST,"<$opts{st}") || die "$opts{st} file opening error !";
open(TS,"<$opts{ts}") || die "$opts{ts} file opening error !";
if (exists($opts{us})){
    open(US,">$opts{us}") || die "$opts{us} file opening error !";
}
if (exists($opts{ut})){
    open(UT,">$opts{ut}") || die "$opts{ut} file opening error !";
}
my $log10=log(10);
my %stprobs;
while (my $st=<ST>) {
    chomp $st;
    my ($trg,$src,$prob_st)=split / /,$st;
    $stprobs{"$src $trg"}=$prob_st;
    if ($src eq "NULL" && exists($opts{ut})){
	if ($prob_st<$TINY) {$prob_st=$TINY;}
	printf UT "$trg %6.5f\n",-1.0*log($prob_st)/$log10;
    }
}
while (my $ts=<TS>) {
    chomp $ts;
    my ($src,$trg,$prob_ts)=split / /,$ts;
    if ($prob_ts<$TINY) {$prob_ts=$TINY;}
    my $prob_st=$stprobs{"$src $trg"};
    if ($prob_st<$TINY) {$prob_st=$TINY;}
    if ($trg eq "NULL" && exists($opts{us})){
	printf US "$src %6.5f\n",-1.0*log($prob_ts)/$log10;
    }else{
	printf "$src $trg %6.5f %6.5f %6.5f\n",-1.0*log($prob_st)/$log10, -1.0*log($prob_ts)/$log10, -1.0*log(($prob_st+$prob_ts)/2.0)/$log10;
    }
}

__END__

=head1 NAME

name.pl - 

=head1 SYNOPSIS
    
perl assoScoreTable_ibm12wda_wd.pl [options] -st input_src2trg -ts input_trg2src > output_table
    
  input_src2trc: src2trg ibm1 file (trg src prob_st)
  input_trg2src: trg2src ibm1 file (src trg prob_ts)
  output_table: src trg -log10(prob_st) -log10(prob_ts) -log10((prob_st+prob_ts)/2)
    
Options:

    -us    Output source unlinked cost model file name
    -ut    Output target unlinked cost model file name
    -v 0|1    0:silent 1:verbose [default 0]
    -help|?    Prints the help and exits
    -man    Prints the manual and exits

=head1 DESCRIPTION



=head1 COPYRIGHT AND LICENSE

Copyright 2007 by Patrik Lambert

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (version 2 or any later version).

=cut
