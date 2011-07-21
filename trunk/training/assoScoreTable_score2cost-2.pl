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
GetOptions(\%opts,'man','help|?','verbose|v=i','col=i@','max=f@') or pod2usage(0);
# check no required arg missing
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(0);
}elsif( ! (exists($opts{"col"}[0])) ){   #required arguments
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
#END PARSING COMMAND-LINE ARGUMENTS
my $verbose=$opts{"verbose"};
my $logTen=log(10);

#PARSE FILE
my %ranking;
while (<>){
    if ($_ !~ /^#/){
	chomp;
	my @toks=split / /;
	for (my $i=0;$i<@{$opts{"col"}};$i++){
	    my $index=$opts{"col"}[$i]+1;
	    my $prob=$toks[$index];
	    $ranking{$i}{$prob}=1;
	}
    }
}
my %max;
my %rank;
for (my $i=0;$i<@{$opts{"col"}};$i++){
    @{$rank{$i}}=keys (%{$ranking{$i}});
    @{$rank{$i}}=sort {$a <=> $b} (@{$rank{$i}});
    $max{$i}=@{$rank{$i}};
    #print join("\n",@rank),"\n";
}
%ranking=();
for (my $i=0;$i<@{$opts{"col"}};$i++){
    my $j=0;
    foreach my $prob (@{$rank{$i}}){
	$ranking{$i}{$prob}=$j/$max{$i};
	++$j;
    }
}

seek(STDIN, 0, 0);

#PARSE FILE
while (<>){
    if ($_ !~ /^#/){
	chomp;
	my @toks=split / /;
	for (my $i=0;$i<@{$opts{"col"}};$i++){
	    my $index=$opts{"col"}[$i]+1;
	    my $prob=$toks[$index];
	    $prob=$ranking{$i}{$prob};
	    if ($prob<$TINY){$prob=$TINY;}
	    my $cost=-1.0*log($prob)/$logTen;
	    $toks[$index]=sprintf "%6.5f", $cost;
	}
	print join(" ",@toks),"\n";
    }
}


__END__

=head1 NAME

assoScoreTable_score2cost.pl - normalises LLR score into a probability and converts into cost.

=head1 SYNOPSIS

perl assoScoreTable_score2cost.pl [options] {-col numCol -max maxScore} < table_scores > table_costs
    
    the sequence between {} can be repeated various times
    col    score column to be normalised (starting from 1 and without counting word indexes columns)
    max    maximum score, used in the normalisation

Options:

    -v 0|1    0:silent 1:verbose [default 0]
    -help|?    Prints the help and exits
    -man    Prints the manual and exits

=head1 COPYRIGHT AND LICENSE

Copyright 2006 by Patrik Lambert

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (version 2 or any later version).

=cut






