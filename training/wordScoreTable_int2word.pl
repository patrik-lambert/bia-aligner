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
GetOptions(\%opts,'man','help|?','verbose|v=i','vcb=s') or pod2usage(0);
# check no required arg missing
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(0);
}elsif( ! (exists($opts{"vcb"})) ){   #required arguments
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
#END PARSING COMMAND-LINE ARGUMENTS
my $verbose=$opts{"verbose"};
open(VCB,"<$opts{vcb}") || die "ERROR opening vocabulary file $!";
my %vocab;
while (<VCB>){
    chomp;
    my ($index,$word)= split /\t/;
    $vocab{$index}=$word;
}
close(VCB);

my $smallest=0;
my $largest=0;
#PARSE FILE
while (<>){
    chomp;
    s/^\s+//;  #trim
    s/\s+$//;  #trim
    s/\s{2,}/ /g;  #remove multiple spaces

    my @indices=split / /;
    if (exists($vocab{"$indices[0]"})){
	$indices[0]=$vocab{"$indices[0]"};
	print join(" ",@indices),"\n";
    }
    
}

__END__

=head1 NAME

wordScoreTable_int2word.pl - replaces each word by its index in a .vcb file passed as argument.

=head1 SYNOPSIS

perl wordScoreTable_int2word.pl [options] -vcb voc_file < table_indices > table_words
    
    vcb    vocabulary file name in TALPwhittle .vcb format

Options:

    -v 0|1    0:silent 1:verbose [default 0]
    -help|?    Prints the help and exits
    -man    Prints the manual and exits

=head1 COPYRIGHT AND LICENSE

Copyright 2006 by Patrik Lambert

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (version 2 or any later version).

=cut
