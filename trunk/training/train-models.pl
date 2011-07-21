#! /usr/bin/perl

use strict;
use Switch;
use Getopt::Long "GetOptions";
use Pod::Usage;
use FindBin qw($Bin);
use File::Spec::Functions;
use File::Basename;
#Debug:
use Dumpvalue;
my $dumper = new Dumpvalue;

$ENV{"LC_ALL"} = "C";

my $BIA_ROOTDIR = $Bin;
if ($BIA_ROOTDIR eq '') {
    $BIA_ROOTDIR = dirname(__FILE__);
}
$BIA_ROOTDIR =~ s/\/training$//;
$BIA_ROOTDIR = $ENV{"BIA_ROOTDIR"} if defined($ENV{"BIA_ROOTDIR"});

#DEFAULT VALUES
my ($man, $usage);

my($__ROOT_DIR, $__CORPUS, $__REDUCED_CORPUS, $__F, $__E, $__MODEL_DIR,
   $__ALIGNMENT_FILE, $__VERBOSE, $__TRAIN_MOSES_CMD, $__TRAIN_MOSES_OPTIONS,
   $__MATCH_TABLE, $__LINK_COST_DISCOUNT,$__LINK_COST_MINOCCURPHRASE, $__LINK_COST_AS_REDUCED, $__NO_FERTILITY,
   $__PRUNING_NBEST, $__PRUNING_UNBEST,$__PRUNING_SCORE,$__PRUNING_THRESHOLD,$__PRUNING_COLUMN_THRESHOLD);

my $debug = 0; # debug this script, do not delete any files in debug mode

#PARSING COMMAND-LINE ARGUMENTS
&GetOptions(
    'root-dir=s' => \$__ROOT_DIR,
    'corpus=s' => \$__CORPUS,
    'reduced-corpus=s' => \$__REDUCED_CORPUS,
    'f=s' => \$__F,
    'e=s' => \$__E,
    'model-dir=s' => \$__MODEL_DIR,
    'alignment-file=s' => \$__ALIGNMENT_FILE,
    'verbose|v=i' => \$__VERBOSE,
    'train-moses-cmd=s' => \$__TRAIN_MOSES_CMD,
    'train-moses-options=s' => \$__TRAIN_MOSES_OPTIONS,
    'linkCost-discount=f' => \$__LINK_COST_DISCOUNT,
    'linkCost-minOccurPhrase=i' => \$__LINK_COST_MINOCCURPHRASE,
    'linkCost-asReduced' => \$__LINK_COST_AS_REDUCED,
    'match-table' => \$__MATCH_TABLE,
    'no-fertility' => \$__NO_FERTILITY,
    'pruning-nbest=i' => \$__PRUNING_NBEST,
    'pruning-unbest=i' => \$__PRUNING_UNBEST,
    'pruning-score=s' => \$__PRUNING_SCORE,
    'pruning-threshold=f' => \$__PRUNING_THRESHOLD,
    'pruning-column-threshold=f' => \$__PRUNING_COLUMN_THRESHOLD,
    'debug' => \$debug,
    'man' => \$man,
    'help|h' => \$usage,
) or pod2usage(1);

if ($man){
    pod2usage(-verbose=>2);
}elsif ($usage){
    pod2usage(1);
}
elsif( ! ($__CORPUS && $__F && $__E) ){
    pod2usage(-msg=>'Required arguments missing',-verbose=>1);
}

# SET VARIABLES TO DEFAULTS OR FROM OPTIONS
my $_VERBOSE = 0;
$_VERBOSE = $__VERBOSE if $__VERBOSE;
my $_ROOT_DIR = ".";
$_ROOT_DIR = $__ROOT_DIR if $__ROOT_DIR;
my $_CORPUS      = $__CORPUS;
my $_MODEL_DIR = $_ROOT_DIR."/bia";
$_MODEL_DIR = $__MODEL_DIR if $__MODEL_DIR;
my $_ALIGNMENT_FILE = "$_ROOT_DIR/model/aligned.grow-diag-final";
$_ALIGNMENT_FILE = $__ALIGNMENT_FILE if $__ALIGNMENT_FILE;
my $_REDUCED_CORPUS = "";
$_REDUCED_CORPUS = $__REDUCED_CORPUS if $__REDUCED_CORPUS;

# foreign/English language extension
my $_F = $__F;
my $_E = $__E;

# supporting binaries from other packages
my $_TRAIN_MOSES_CMD="moses";
$_TRAIN_MOSES_CMD = $__TRAIN_MOSES_CMD if $__TRAIN_MOSES_CMD;
my $_TRAIN_MOSES_OPTIONS="";
$_TRAIN_MOSES_OPTIONS = $__TRAIN_MOSES_OPTIONS if $__TRAIN_MOSES_OPTIONS;

#die("ERROR: Cannot find moses") unless (-x $_MOSES);
# utilities
my $ZCAT = "gzip -cd";
my $BZCAT = "bzcat";

# training options
my $_MATCH_TABLE = 0;
$_MATCH_TABLE = $__MATCH_TABLE if $__MATCH_TABLE;

my $_LINK_COST_DISCOUNT = 0;
$_LINK_COST_DISCOUNT = $__LINK_COST_DISCOUNT if $__LINK_COST_DISCOUNT;

my $_LINK_COST_MINOCCURPHRASE = 3;
$_LINK_COST_MINOCCURPHRASE = $__LINK_COST_MINOCCURPHRASE if $__LINK_COST_MINOCCURPHRASE;

my $_LINK_COST_AS_REDUCED = "";
$_LINK_COST_AS_REDUCED = "-probsAsReduced" if $__LINK_COST_AS_REDUCED;
if ($_LINK_COST_AS_REDUCED ne "" && $_REDUCED_CORPUS eq ""){    
    die "If the option \"-linkCost-asReduced\" is set, you must provide a reduced corpus (for example: a stemmed version of the corpus)\n"; 
}

my $_NO_FERTILITY = 0;
$_NO_FERTILITY = $__NO_FERTILITY if $__NO_FERTILITY;

my $_PRUNING_NBEST = 200;
$_PRUNING_NBEST = $__PRUNING_NBEST if $__PRUNING_NBEST;

my $_PRUNING_UNBEST = 20;
$_PRUNING_UNBEST = $__PRUNING_UNBEST if $__PRUNING_UNBEST;

my $_PRUNING_SCORE = "1";
$_PRUNING_SCORE = $__PRUNING_SCORE if $__PRUNING_SCORE;
my $_PRUNING_THRESHOLD = 9;
$_PRUNING_THRESHOLD = $__PRUNING_THRESHOLD if $__PRUNING_THRESHOLD;
my $_PRUNING_COLUMN_THRESHOLD = 0;
$_PRUNING_COLUMN_THRESHOLD = $__PRUNING_COLUMN_THRESHOLD if $__PRUNING_COLUMN_THRESHOLD;


### MAIN

if ( ! -e $_ALIGNMENT_FILE ){
    &train_alignment($_TRAIN_MOSES_CMD,$_TRAIN_MOSES_OPTIONS);
}
&train_models();

####################################################################################

### ALIGN CORPUS WITH MOSES SCRIPTS
sub train_alignment {
    print STDERR "(0) Not finding any alignment file: training alignment with moses scripts @ ".`date`;
    die("ERROR: Cannot find moses script for training models.\n") unless (-x $_TRAIN_MOSES_CMD);
    
    my $cmd="$_TRAIN_MOSES_CMD -first-step 1 -last-step 3 -root-dir $_ROOT_DIR -corpus $_CORPUS -f $_F -e $_E $_TRAIN_MOSES_OPTIONS";
    safesystem($cmd);
    #possible options            -scripts-root-dir $mscripts -mgiza -mgiza-cpus $smp -parallel 


    $cmd="
         for _dir in $_ROOT_DIR/giza.*
         do
	    rm -f \$_dir/*.A3.final.part* \$_dir/*.a3.* \$_dir/*.d3.* \$_dir/*.n3.* \$_dir/*.p0_3.* \$_dir/*.t3.*
	    gzip -f \$_dir/*.cooc
         done
";
    safesystem($cmd);
}

### TRAIN MODELS
sub train_models {
    print STDERR "(1) Training BIA models @ ".`date`;
    safesystem("mkdir -p $_MODEL_DIR") or die("ERROR: could not create BIA model dir $_MODEL_DIR");
    my $srcFile = "$_CORPUS."."$_F";
    my $trgFile = "$_CORPUS."."$_E";
    my $modelFile = "$_MODEL_DIR/$_F-$_E.pha-d$_LINK_COST_DISCOUNT";
    my $reducedCorpusString="";
    if ($_REDUCED_CORPUS ne ""){
	$reducedCorpusString="-sred $_REDUCED_CORPUS.$_F -tred $_REDUCED_CORPUS.$_E"
    }
    my $fertilityString="";
    if (! $_NO_FERTILITY){
	$fertilityString="-osf $_MODEL_DIR/fertility.$_F -otf $_MODEL_DIR/fertility.$_E";
    }
    
    # training lexical and fertility models
    if ( ! -e "$modelFile" ){	
	my $cmd="$BIA_ROOTDIR/training/linkCosts -s $srcFile -t $trgFile $reducedCorpusString -st $_ALIGNMENT_FILE $fertilityString -v 1 -d $_LINK_COST_DISCOUNT -minOccurPhrase $_LINK_COST_MINOCCURPHRASE $_LINK_COST_AS_REDUCED > $modelFile";
	safesystem($cmd) or die "Failed to train BIA models";
    }else{
	print STDERR "(1.1) Re-using existing model $modelFile\n";
    }
    
    # prune lexical model
    my $prunedModelFile = "$modelFile.prn$_PRUNING_SCORE-$_PRUNING_THRESHOLD-$_PRUNING_COLUMN_THRESHOLD-$_PRUNING_UNBEST-$_PRUNING_NBEST";
    if (! -e $prunedModelFile){
	my $cmd="$BIA_ROOTDIR/training/prune-assoScoreTable -th $_PRUNING_THRESHOLD -colth $_PRUNING_COLUMN_THRESHOLD -unbest $_PRUNING_UNBEST -nbest $_PRUNING_NBEST -score $_PRUNING_SCORE -i $modelFile > $prunedModelFile";
	safesystem($cmd) or die "Failed to pruned the lexical model";
    }else{
	print STDERR "(1.2) Lexical model $modelFile already pruned, reusing\n";
    }

    # compute match table
    my $mssFile="$_MODEL_DIR/$_F-$_E.mss";
    if ($_MATCH_TABLE && ! -e $mssFile){
	my $cmd="$BIA_ROOTDIR/training/detect-matchStemSyn -s $srcFile -t $trgFile -nostem -nosyn > $mssFile";
	safesystem($cmd) or die "Failed to build the match table";	
    }else{
	print STDERR "(1.3) Re-using existing match table $mssFile\n";
    }

    ### WRITE BIA CONFIGURATION FILE
    my $biacfg="$_MODEL_DIR/bia.cfg";
    my $cfgString="";
    $cfgString.="-is $srcFile\n";
    $cfgString.="-it $trgFile\n";
    if (! $_NO_FERTILITY){
	$cfgString.="-isf $_MODEL_DIR/fertility.$_F\n";
	$cfgString.="-itf $_MODEL_DIR/fertility.$_E\n";
    }
    $cfgString.="-iwda $prunedModelFile\n";
    if ($_MATCH_TABLE){
	$cfgString.="-imss $mssFile\n";
    }
    #default parameters:
    $cfgString.="-n 3
-b 5
-t 5
-m 1
-c 12
-ct 3
-first avge
-exp word
-dir at
-doba 1
-aoba 2
-multiple-stacks number
-cmatch 0
-wwda1 0.5 
-wwda2 0.5 
-wwda3 0.3 
-wlb 1 
-wcn 1 
-wcl 0.2 
-wrk 0.5 
-wf 1 
-whp 0.3
";
    open(CFG,">$biacfg") || die "Cannot open $biacfg file\n";
    print CFG "$cfgString";
}

######
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
    $PATH =~ s/\/nfsmnt//;
    return $PATH if $PATH =~ /^\//;
    my $dir = `pawd 2>/dev/null`; 
    if(!$dir){$dir = `pwd`;}
    chomp($dir);
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
    $PATH =~ s/\/nfsmnt//;
    return $PATH;
}

sub open_or_zcat {
  my $fn = shift;
  my $read = $fn;
  $fn = $fn.".gz" if ! -e $fn && -e $fn.".gz";
  $fn = $fn.".bz2" if ! -e $fn && -e $fn.".bz2";
  if ($fn =~ /\.bz2$/) {
      $read = "$BZCAT $fn|";
  } elsif ($fn =~ /\.gz$/) {
      $read = "$ZCAT $fn|";
  }
  my $hdl;
  open($hdl,$read) or die "Can't read $fn ($read)";
  return $hdl;
}

__END__

=head1 SYNOPSIS

perl train-models.pl --corpus corpus --f source_extension --e target_extension [options]

Parameters:

    corpus=STRING   Path of the corpus (without the final extension for the language)
    f=STRING   Source (foreign) language extension
    e=STRING   Target (English) language extension

    root-dir=STRING   Base working directory of your experiment
    model-dir=STRING   Directory for BIA models
    alignment-file=STRING   Path to the alignment file
    train-moses-cmd=STRING   Path to the moses training script (to train alignments)
    train-moses-options=STRING   Options of moses training script (for steps 1 to 3)
    linkCost-discount=FLOAT   Discounting parameter for relative link probabilities Prob=(links-discount)/cooccurrences [default 0]
    linkCost-minOccurPhrase=INT   Mininmum occurrence of a phrase to be included in the relative link model [default 3]
                                  This restriction only applies to phrases of length>1 ie all words are considered.
    pruning-nbest=INT   Number of target correspondants selected for each source word, and vice-versa [default 200]
	           The intersection of source-target and target-source lists is taken
    pruning-unbest=INT   (Union nbest): Minimum nb of trg correspondants selected for each src word, and vice-versa [default 20]
    pruning-score=STRING in (1|2|3|1+2|2+3|1+2+3) Score taken into account to rank {src,trg} word pairs [default 1]
		1,2 and 3 refer to first, second and third column's score
    pruning-threshold=FLOAT Cost threshold for the score taken into account [default 9]
    pruning-column-threshold=FLOAT Additional cost threshold which applies to each score column [default 0: not used]
    verbose|v=INT   Verbose level
    debug   Debug mode
    man   Prints this info
    help|h   Prints this info

=cut
