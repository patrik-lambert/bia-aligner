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
GetOptions(\%opts,'man','help|?','verbose|v=i','svcb=s','tvcb=s','i_s=s','i_t=s','o_s=s','o_t=s') or pod2usage(0);
# check no required arg missing
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(0);
}elsif( ! (exists($opts{"i_s"}) && exists($opts{"i_t"}) && exists($opts{"o_s"}) && exists($opts{"o_t"})) ){   #required arguments
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
#END PARSING COMMAND-LINE ARGUMENTS

my %svocab;
my %tvocab;
my $useVocTable=0;
if (exists($opts{svcb}) && exists($opts{tvcb})){
    $useVocTable=1;
    my $verbose=$opts{"verbose"};
    open(SVCB,"<$opts{svcb}") || die "ERROR opening vocabulary file $!";
    while (<SVCB>){
	chomp;
	my ($index,$word)= split /\t/;
	$svocab{$index}=$word;
    }
    close(SVCB);
    open(TVCB,"<$opts{tvcb}") || die "ERROR opening vocabulary file $!";
    while (<TVCB>){
	chomp;
	my ($index,$word)= split /\t/;
	$tvocab{$index}=$word;
    }
    close(TVCB);
}
#open input files
my $file = $opts{i_s};
my $froot;
if ($file =~ /(.*)\.gz$/){$froot=$1;}
else {$froot=$file;}
my $openfile;
if ( -e "$froot.gz" ){$openfile="zcat $froot.gz |";}
else {$openfile="< $froot";}
open(IS,"$openfile") || die "$opts{s} file opening error !";

$file = $opts{i_t};
if ($file =~ /(.*)\.gz$/){$froot=$1;}
else {$froot=$file;}
if ( -e "$froot.gz" ){$openfile="zcat $froot.gz |";}
else {$openfile="< $froot";}
open(IT,"$openfile") || die "$opts{t} file opening error !";

open(OS,">$opts{o_s}") || die "Can't open source output file $opts{o_s}\n";
open(OT,">$opts{o_t}") || die "Can't open source output file $opts{o_t}\n";

#PARSE FILE
while (<>){
    chomp;
    s/^\s+//;  #trim
    s/\s+$//;  #trim
    s/\s{2,}/ /g;  #remove multiple spaces
    my $sline=<IS>; chomp $sline; $sline =~ s/^\s+//; $sline =~ s/\s+$//; $sline =~ s/\s{2,}/ /g;
    my @swords=split(/ /,$sline);
    my $tline=<IT>; chomp $tline; $tline =~ s/^\s+//; $tline =~ s/\s+$//; $tline =~ s/\s{2,}/ /g;
    my @twords=split(/ /,$tline);

    my @clusters=split / /;
    foreach my $clust (@clusters){
	my ($srcSide,$trgSide)=split(/\-/,$clust);
	my @sPositions=split(/\,/,$srcSide);
	my @sSents;
	foreach my $posi (@sPositions){
	    my $sent;
	    if ($useVocTable){
		$sent=$svocab{$swords[$posi]};
	    }else{
		$sent=$swords[$posi];
	    }
	    push @sSents,$sent;
	}
	print OS join(" ",@sSents),"\n";

	my @tPositions=split(/\,/,$trgSide);
	my @tSents;
	foreach my $posi (@tPositions){
	    my $sent;
	    if ($useVocTable){
		$sent=$tvocab{$twords[$posi]};
	    }else{
		$sent=$twords[$posi];
	    }
	    push @tSents,$sent;
	}
	print OT join(" ",@tSents),"\n";
    }
}
close(OS);
close(OT);

__END__

=head1 NAME

linkCluster2sentence.pl - replaces link clusters by the corresponding words, with a return character after each cluster.

=head1 SYNOPSIS

perl linkCluster2sentence.pl [-svcb src_voc_file -tvcb trg_voc_file] -o_s output-source -o_t output-target < link_cluster_input
    
    -i_s     source corpus (text corresponding to alignment file)
    -i_t     target corpus (text corresponding to alignment file)
    -o_s     output source file
    -o_t     output target file

Options:

    -svcb    source vocabulary file name in TALPwhittle .vcb format (if present, indices in source are replaced by voc values)
    -tvcb    target vocabulary file name in TALPwhittle .vcb format (if present, indices in target are replaced by voc values)
    -v 0|1    0:silent 1:verbose [default 0]
    -help|?    Prints the help and exits
    -man    Prints the manual and exits

=head1 AUTHORS

Patrik Lambert

=cut
