#!/usr/bin/perl -w

# $Id: filter-model-given-input.pl $
# Filters a bia model file given an input text

# Adapted by Patrik Lambert from Moses code by Philipp Koehn and Ondrej Bojar

use strict;
use Dumpvalue;
my $dumper = new Dumpvalue;

my $dir;
my $config;
my $modeldir;
my $input;
my $MAX_LENGTH = 10;

# utilities
my $ZCAT = "gzip -cd";

# consider phrases in input up to this length
# in other words, all phrase-tables will be truncated at least to 10 words per
# phrase

my $model = shift; 
my $source_input = shift;
my $target_input = shift;

if (!defined $model || !defined $source_input || !defined $target_input) {
  print STDERR "usage: filter-model-given-input.pl model source_input target_input\n";
  exit 1;
}

my @txt=split(/\//,$source_input);
my $suffix=pop @txt;
@txt=split(/\./,$suffix);
my $prefix=shift @txt;
#print "prefix:$prefix\n";

# already filtered? check if it can be re-used
if (-e "$model.filtered-$prefix") {
    print STDERR "The filtered model already exists, not doing anything.\n";
    exit 0;
}

# get tables to be filtered (and modify config file)
my %CONSIDER_FACTORS=(0,1);
my @TABLE=("$model");
my @TABLE_FACTORS=(0);
my @TABLE_NEW_NAME=("$model.filtered-$prefix");

# get the phrase pairs appearing in the input text, up to the $MAX_LENGTH
my %SOURCE_PHRASE_USED;
extract_phrases_used(\%SOURCE_PHRASE_USED,$source_input);
my %TARGET_PHRASE_USED;
extract_phrases_used(\%TARGET_PHRASE_USED,$target_input);
#$dumper->dumpValue(\%SOURCE_PHRASE_USED);

# filter files
for(my $i=0;$i<=$#TABLE;$i++) {
    my ($used,$total) = (0,0);
    my $file = $TABLE[$i];
    my $factors = $TABLE_FACTORS[$i];
    my $new_file = $TABLE_NEW_NAME[$i];
    print STDERR "filtering $file -> $new_file...\n";

    my $openstring;
    if ($file !~ /\.gz$/ && -e "$file.gz") {
      $openstring = "$ZCAT $file.gz |";
    } elsif ($file =~ /\.gz$/) {
      $openstring = "$ZCAT $file |";
    } else {
      $openstring = "< $file";
    }

    open(FILE,$openstring) or die "Can't open '$openstring'";
    open(FILE_OUT,">$new_file") or die "Can't write $new_file";

    while(my $entry = <FILE>) {
        my ($foreign,$english,$rest) = split(/ \|\|\| /,$entry);
        $foreign =~ s/ $//;
	$english =~ s/ $//;
	$english =~ s/^ //;
        if (defined($SOURCE_PHRASE_USED{$factors}{$foreign}) && defined($TARGET_PHRASE_USED{$factors}{$english})) {
    	print FILE_OUT $entry;
    	$used++;
        }
        $total++;
    }
    close(FILE);
    close(FILE_OUT);
    die "No phrases found in $file!" if $total == 0;
    printf STDERR "$used of $total phrases pairs used (%.2f%s) - note: max length $MAX_LENGTH\n",(100*$used/$total),'%';
}

#########################################################

sub extract_phrases_used {
    my $PHRASE_USED=shift;
    my $input=shift;
    open(INPUT,$input) or die "Can't read $input";
    while(my $line = <INPUT>) {
	chomp($line);
	my @WORD = split(/ +/,$line);
	for(my $i=0;$i<=$#WORD;$i++) {
	    for(my $j=0;$j<$MAX_LENGTH && $j+$i<=$#WORD;$j++) {
		foreach (keys %CONSIDER_FACTORS) {
		    my @FACTOR = split(/,/);
		    my $phrase = "";
		    for(my $k=$i;$k<=$i+$j;$k++) {
			my @WORD_FACTOR = split(/\|/,$WORD[$k]);
			for(my $f=0;$f<=$#FACTOR;$f++) {
			    $phrase .= $WORD_FACTOR[$FACTOR[$f]]."|";
			}
			chop($phrase);
			$phrase .= " ";
		    }
		    chop($phrase);
		    $PHRASE_USED->{$_}{$phrase}++;
		}
	    }
	}
    }
    close(INPUT);
}

sub safesystem {
  print STDERR "Executing: @_\n";
  system(@_);
  if ($? == -1) {
      print STDERR "Failed to execute: @_\n  $!\n";
      exit(1);
  }
  elsif ($? & 127) {
      printf STDERR "Execution of: @_\n  died with signal %d, %s coredump\n",
          ($? & 127),  ($? & 128) ? 'with' : 'without';
      exit(1);
  }
  else {
    my $exitcode = $? >> 8;
    print STDERR "Exit code: $exitcode\n" if $exitcode;
    return ! $exitcode;
  }
}
sub ensure_full_path {
    my $PATH = shift;
    return $PATH if $PATH =~ /^\//;
    my $dir = `pawd 2>/dev/null`;
    if (!$dir) {$dir = `pwd`;}
    chomp $dir;
    $PATH = $dir."/".$PATH;
    $PATH =~ s/[\r\n]//g;
    $PATH =~ s/\/\.\//\//g;
    $PATH =~ s/\/+/\//g;
    my $sanity = 0;
    while($PATH =~ /\/\.\.\// && $sanity++<10) {
        $PATH =~ s/\/+/\//g;
        $PATH =~ s/\/[^\/]+\/\.\.\//\//g;
    }
    $PATH =~ s/\/[^\/]+\/\.\.$//;
    $PATH =~ s/\/+$//;
    return $PATH;
}
