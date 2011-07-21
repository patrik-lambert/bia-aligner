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
$BIA_ROOTDIR =~ s/\/tuning$//;
$BIA_ROOTDIR = $ENV{"BIA_ROOTDIR"} if defined($ENV{"BIA_ROOTDIR"});

#DEFAULT VALUES
my ($man, $usage);

my ($__CONFIG, $_CORPUS, $_F,$_E, $__DEV_F, $__DEV_E, $__MERT_DEV_F, $__MERT_DEV_E,
   $__ROOT_DIR, $__TMPDIR, $__TUNING_DIR, $__VERBOSE, $_SENT_ALIGN_POSTPROCESS, $_SENT_ALIGN_WORDALIGN, $__SMT_SYSTEM, $__SMT_SYSTEM_CONFIG, 
   $__NO_MERT, $__MOSES_CMD, $__MOSES_SCRIPT_DIR, $__TRAIN_MOSES_OPTIONS,
   $__JOBS,$__PARALMODE,$__MEM,$__WALLTIME,$__SMT_MEM,$__SMT_WALLTIME,$__OPTIMIZER,$__SIMPLEX_CMD,$__SIMPLEX_FTOL,$__SEED,$__SPSA_GRADIENT_FACTOR,$__METRIC);

my $debug = 0; # debug this script, do not delete any files in debug mode

#PARSING COMMAND-LINE ARGUMENTS
&GetOptions(
    'config=s' => \$__CONFIG, # BIA configuration file
    'corpus=s' => \$_CORPUS, # could be taken from BIA config file, but the corpus for moses is not necessarily the same (e.g. you may align stems and train Moses on words).
    'e=s' => \$_E, 
    'f=s' => \$_F, 
    'input=s' => \$__DEV_F,
    'refs=s' => \$__DEV_E,
    'mert-input=s' => \$__MERT_DEV_F,
    'mert-refs=s' => \$__MERT_DEV_E,
    'root-dir=s' => \$__ROOT_DIR, # working directory
    'tuning-dir=s' => \$__TUNING_DIR, # tuning directory
    'tmpdir=s' => \$__TMPDIR, # directory to store temporary files
    'jobs=i' => \$__JOBS, # number of BIA jobs to be run at the same time 
    'paralmode=s' => \$__PARALMODE, # fork / qsub
    'mem=s' => \$__MEM, # memory to ask to scheduler
    'walltime=s' => \$__WALLTIME, # walltime to ask to scheduler
    'smt-mem=s' => \$__SMT_MEM, # memory to ask to scheduler
    'smt-walltime=s' => \$__SMT_WALLTIME, # walltime to ask to scheduler
    'optimizer=s' => \$__OPTIMIZER,
    'simplex-cmd=s' => \$__SIMPLEX_CMD,
    'simplex-ftol=f' => \$__SIMPLEX_FTOL,
    'spsa-gradient-factor=f' => \$__SPSA_GRADIENT_FACTOR, # small a factor in SPSA. The new lambdas are old*a(...)*gradient
    'seed=i' => \$__SEED,
    'metric=s' => \$__METRIC,
    'no-mert' => \$__NO_MERT,
    'sent-align-postprocess=s' => \$_SENT_ALIGN_POSTPROCESS,
    'sent-align-wordalign=s' => \$_SENT_ALIGN_WORDALIGN,
    'smt-system=s' => \$__SMT_SYSTEM,
    'smt-system-config=s' => \$__SMT_SYSTEM_CONFIG,
    'moses-cmd=s' => \$__MOSES_CMD,
    'moses-script-dir=s' => \$__MOSES_SCRIPT_DIR,
    'train-moses-options=s' => \$__TRAIN_MOSES_OPTIONS,
    'debug' => \$debug,
    'verbose|v=i' => \$__VERBOSE,
    'man' => \$man,
    'help|h' => \$usage,
) or pod2usage(1);

if ($man){
    pod2usage(-verbose=>2);
}elsif ($usage){
    pod2usage(1);
}
elsif( ! ($__CONFIG && $__DEV_F && $__DEV_E) ){
    pod2usage(-msg=>'Required arguments missing',-verbose=>1);
}

my ($_CONFIG, $_DEV_F, $_DEV_E)=($__CONFIG, $__DEV_F, $__DEV_E);

# SET VARIABLES TO DEFAULTS OR FROM OPTIONS
my $_MERT_DEV_E = "";
$_MERT_DEV_E = $__MERT_DEV_E if $__MERT_DEV_E;
my $_MERT_DEV_F = "";
$_MERT_DEV_F = $__MERT_DEV_F if $__MERT_DEV_F;
my $_VERBOSE = 0;
$_VERBOSE = $__VERBOSE if $__VERBOSE;
my $_ROOT_DIR = `pwd`;chomp $_ROOT_DIR;
$_ROOT_DIR = $__ROOT_DIR if $__ROOT_DIR;
my $_TMPDIR = "./tmp";
$_TMPDIR = $__TMPDIR if $__TMPDIR;
my $_TUNING_DIR = "$_ROOT_DIR/tuning";
$_TUNING_DIR = $__TUNING_DIR if $__TUNING_DIR;
my $_JOBS = 1;
$_JOBS = $__JOBS if $__JOBS;
my $_PARALMODE = "fork";
$_PARALMODE = $__PARALMODE if $__PARALMODE;
my $_MEM = "5gb";
$_MEM = $__MEM if $__MEM;
my $_WALLTIME = "24:00:00";
$_WALLTIME = $__WALLTIME if $__WALLTIME;
my $_SMT_MEM = "15gb";
$_SMT_MEM = $__SMT_MEM if $__SMT_MEM;
my $_SMT_WALLTIME = "24:00:00";
$_SMT_WALLTIME = $__SMT_WALLTIME if $__SMT_WALLTIME;

# supporting binaries from other packages
my $_OPTIMIZER = "simplex";
$_OPTIMIZER = lc($__OPTIMIZER) if $__OPTIMIZER;
my $_SIMPLEX_CMD=`which fmin`; chomp $_SIMPLEX_CMD;
$_SIMPLEX_CMD = $__SIMPLEX_CMD if $__SIMPLEX_CMD;
my $_SIMPLEX_FTOL=0.005;
$_SIMPLEX_FTOL = $__SIMPLEX_FTOL if $__SIMPLEX_FTOL;
my $_SEED = "6894385819";
$_SEED = $__SEED if $__SEED;
my $_SPSA_GRADIENT_FACTOR = 8;
$_SPSA_GRADIENT_FACTOR = $__SPSA_GRADIENT_FACTOR if $__SPSA_GRADIENT_FACTOR;
my $_METRIC="bleu";
$_METRIC = lc($__METRIC) if $__METRIC;
my $_NO_MERT=0;
$_NO_MERT = $__NO_MERT if $__NO_MERT;
my $_SMT_SYSTEM="phrase-based-moses";
$_SMT_SYSTEM = lc($__SMT_SYSTEM) if $__SMT_SYSTEM;
my $_SMT_SYSTEM_CONFIG="";
$_SMT_SYSTEM_CONFIG = $__SMT_SYSTEM_CONFIG if $__SMT_SYSTEM_CONFIG;
my $_MOSES_CMD="mosesmt";
$_MOSES_CMD = $__MOSES_CMD if $__MOSES_CMD;
my $_MOSES_SCRIPT_DIR="train-model.perl";
$_MOSES_SCRIPT_DIR = $__MOSES_SCRIPT_DIR if $__MOSES_SCRIPT_DIR;
my $_TRAIN_MOSES_OPTIONS="";
$_TRAIN_MOSES_OPTIONS = $__TRAIN_MOSES_OPTIONS if $__TRAIN_MOSES_OPTIONS;


# utilities
my $ZCAT = "gzip -cd";
my $BZCAT = "bzcat";

### MAIN
my %FREE_PARAMS;
my %RANGES;
my @SORTED_PARS=("wwda1", "wwda2", "wwda3", "wwda4", "wwda5", "wwda6", "wtga", "wrk", "wrks", "wrkt", "wrkb", "wrkw", "wmatchb", "wstemb", "wsynb", "wlb", "wpp", "wup", "wus", "wut", "wum", "wums", "wumt", "wf", "wfs", "wft", "wcn", "wcl", "whp", "whs", "wht", "wzp", "w1to1","w1toN","w1toNsum","wNto1","wNtoM","wchfilt", "wchfilts", "wchfiltt","cmatch","cstem","csyn");
my @SORTED_FREEPAR_KEYS;
my $_BIA_INPUT_DIVISIONS="";
&init($_CONFIG);
&write_optimizer_config ($_OPTIMIZER);
&launch_optimizer ($_OPTIMIZER);

####################################################################################
sub init {
    my ($biacfg)=@_;
    my $cmd="mkdir -p $_TMPDIR"; 
    print STDERR "Creating temporary directory.\n";
    safesystem("$cmd");
    my $cmd="mkdir -p $_TUNING_DIR";
    print STDERR "Creating tuning directory.\n";
    safesystem("$cmd");

    # Triples are [lower bound, upper bound, variation in downhill simplex]
    %RANGES = (
	"wwda1" => [0.0, "", 0.5],
	"wwda2" => [0.0, "", 0.5],
	"wwda3" => [0.0, "", 0.5],
	"wwda4" => [0.0, "", 0.5],
	"wwda5" => [0.0, "", 0.5],
	"wwda6" => [0.0, "", 0.5],
	"wtga" => [0.0, "", 0.5],
	"wrks" => [0.0, "", 0.3],
	"wrkt" => [0.0, "", 0.3],
	"wrk" => [0.0, "", 0.3],
	"wrkb" => [0.0, "", 0.3],
	"wrkw" => [0.0, "", 0.3],
	"wum" => [0.01, "", 0.5],
	"wums" => [0.01, "", 0.5],
	"wumt" => [0.01, "", 0.5],
	"wf" => [0.01, "", 0.5],
	"wfs" => [0.01, "", 0.5],
	"wft" => [0.01, "", 0.5],
	"wchfilt" => [0.0, "", 0.3],
	"wchfilts" => [0.0, "", 0.3],
	"wchfiltt" => [0.0, "", 0.3],
	"wlb" => [0.0, "", 0.5],
	"wpp" => [0.0, "", 0.5],
	"wup" => [0.01, "", 0.5],
	"wus" => [0.01, "", 0.5],
	"wut" => [0.01, "", 0.5],
	"wcn" => [0.0, "", 0.5],
	"wcl" => [0.0, "", 0.3],
	"whp" => [0.0, "", 0.5],
	"whs" => [0.0, "", 0.5],
	"wht" => [0.0, "", 0.5],
	"wzp" => [0.0, "", 0.5],
	"w1to1" => ["", "", 0.5],
	"w1toN" => ["", "", 0.5],
	"wNto1" => ["", "", 0.5],
	"w1toNsum" => ["", "", 0.5],
	"wNtoM" => ["", "", 0.5],
	"wmatchb" => [0.0, "", 0.5],
	"wstemb" => [0.0, "", 0.5],
	"wsynb" => [0.0, "", 0.5],
	"cmatch" => [0.0, "", 0.5],
	"cstem" => [0.0, "", 0.5],
	"csyn" => [0.0, "", 0.5],
	);

    
    # GET FREE PARAMETERS FROM BIA CONFIGURATION FILE 
    open(CFG,"<$biacfg") || die "$biacfg file opening error !";
    my ($srcBiaInput);
    my @wdaParams;
    while(<CFG>){
	chomp; $_=~ s/^\s+//; $_ =~ s/\s{2,}/ /g;
	if ($_ =~ /^\-w/ || $_ =~ /^\-c(match|stem|syn)/){
	    my ($key,$val)=split / /;
	    $key=substr($key,1);
	    $FREE_PARAMS{$key}=$val;
	    if ($key =~ /^wwda/){push @wdaParams,$key;}
	}elsif ($_ =~ /\s*\-is /){
	    my $key; 
	    ($key,$srcBiaInput)=split / /;
	}
    }
    close(CFG);
    
    # at least one wda param must be always >0
    @wdaParams=sort {$a cmp $b} @wdaParams;
    $RANGES{"$wdaParams[0]"}[0]=0.001;
    print STDERR "At least one of wwdaN params must be always >0: setting $wdaParams[0] range to begin with 0.001\n";

    # WORD-NUMBER-BASED SPLITTING OF BIA INPUT FILES (GIVEN THE NUMBER OF PROCESSES, $_JOBS)
    if ($_JOBS>0){
	open(IS,"$srcBiaInput") || die "Cannot open source bia input ($srcBiaInput)\n";
	my $total=0;
	my @wordNumbers;
	while (<IS>){
	    my $nwords=split (/ /);
	    push @wordNumbers,log($nwords)*$nwords**2;
	    $total+=log($nwords)*$nwords**2;
	}
	close(IS);
	if ($_JOBS<@wordNumbers){
	    my @lines;
	    my $subtotal=0;
	    my $task=($total/$_JOBS);
	    for (my $i=0;$i<@wordNumbers-1;$i++){
		$subtotal+=$wordNumbers[$i];
		my $target=$task*(scalar(@lines)+1);
		my $nextSubtotal=$subtotal+$wordNumbers[$i+1];
		if ($nextSubtotal > $target){
		    if ( $target-$subtotal < $nextSubtotal-$target){
			# $line $i is closer from target than line $i+1
			push @lines,$i;
		    }else{
			push @lines,$i+1;
		    }
		}
	    }
	    $_BIA_INPUT_DIVISIONS=join(",",@lines);
# 	    print "Total:$total\nLines:\n";
# 	    my $previous=0;
# 	    for (my $i=0;$i<@lines;$i++){
# 		print "$i: line $lines[$i] (".($lines[$i]-$previous).")\n";
# 		$previous=$lines[$i];
# 	    }
# 	    print scalar(@lines).": line ".scalar(@wordNumbers)." (".(scalar(@wordNumbers)-$previous).")\n";
	}
    }
}

# WRITE OPTIMIZER CONFIGURATION FILE
sub write_optimizer_config{
    my $optimizer=shift;
    my $optimConfig="$_TUNING_DIR/$optimizer.cfg";
    open(CFG,">$optimConfig") || die "Could not open file optimizer config file ($optimConfig)\n";
    print STDERR "Writing optimizer configuration file in $_TUNING_DIR/$optimizer.cfg\n";
    foreach my $param (@SORTED_PARS){
	if (exists($FREE_PARAMS{$param})){push @SORTED_FREEPAR_KEYS,$param;}
    }
    my $nDim=scalar(keys %FREE_PARAMS);
    
    if ($optimizer eq "simplex"){
	my @initConstVec=();
	my $freeParStr="";
	foreach my $key (@SORTED_FREEPAR_KEYS){
	    if (exists($RANGES{$key})){
		push @initConstVec,$RANGES{$key}->[2];
	    }else{
		push @initConstVec,[0.0, "", 0.5];		
	    }
	    $freeParStr.="freeParam -$key $FREE_PARAMS{$key} [".$RANGES{$key}->[0].",".$RANGES{$key}->[1]."]\n";
	}
	my $initConstVecStr=join(" ",@initConstVec);
	print CFG "
nDim $nDim
fTol $_SIMPLEX_FTOL
cTol 0.1
maxIter 200
maxRestarts 0
initConstVec $initConstVecStr
errorMeasure class
mtSystem $BIA_ROOTDIR/tuning/bialign-smt-eval.pl 
src $_DEV_F
fixedParams $optimConfig
$freeParStr
";	
    } elsif ($optimizer eq "spsa"){
	my $freeParStr="";
	foreach my $key (@SORTED_FREEPAR_KEYS){
	    $freeParStr.="freeParam -$key $FREE_PARAMS{$key} [".$RANGES{$key}->[0].",".$RANGES{$key}->[1]."]\n";
	}
	print CFG "
nDim $nDim
cTol 0.1
maxIter 200
errorMeasure class
mtSystem $BIA_ROOTDIR/tuning/bialign-smt-eval.pl 
src $_DEV_F
fixedParams $optimConfig
$freeParStr
";	
    }

    # WHATEVER THE OPTIMIZER, PRINT THIS INFO TO THE CONFIG FILE
    print CFG "
// config=\"$_CONFIG\"
// tuningdir=\"$_TUNING_DIR\"
// tmpdir=\"$_TMPDIR\"
// jobs=\"$_JOBS\"
// paralmode=\"$_PARALMODE\"\n";
    if ($_PARALMODE eq "qsub"){
	print CFG "// mem=\"$_MEM\"\n";
	print CFG "// walltime=\"$_WALLTIME\"\n";
	print CFG "// smtmem=\"$_SMT_MEM\"\n";
	print CFG "// smtwalltime=\"$_SMT_WALLTIME\"\n";
    }
    print CFG "// freepars=\"",join(" ",@SORTED_FREEPAR_KEYS),"\"
// input=\"$_DEV_F\"
// ref=\"$_DEV_E\"
// mertinput=\"$_MERT_DEV_F\"
// mertref=\"$_MERT_DEV_E\"
// verbose=\"$_VERBOSE\"
// metric=\"BLEU\"
// nomert=\"$_NO_MERT\"
// smtsystem=\"$_SMT_SYSTEM\"
// smtsystemconfig=\"$_SMT_SYSTEM_CONFIG\"
// mosescmd=\"$_MOSES_CMD\"
// mosesscriptdir=\"$_MOSES_SCRIPT_DIR\"
// trainmosesoptions=\"$_TRAIN_MOSES_OPTIONS\"
// biarootdir=\"$BIA_ROOTDIR\"
";
    if ($_CORPUS){
	print CFG "// corpus=\"$_CORPUS\"\n";
    }
    if ($_E){
	print CFG "// e=\"$_E\"\n";
    }
    if ($_F){
	print CFG "// f=\"$_F\"\n";
    }
    if ($_SENT_ALIGN_POSTPROCESS){
	print CFG "// sentalignpostprocess=\"$_SENT_ALIGN_POSTPROCESS\"\n";
    }
    if ($_SENT_ALIGN_WORDALIGN){
	print CFG "// sentalignwordalign=\"$_SENT_ALIGN_WORDALIGN\"\n";
    }
    if ($_BIA_INPUT_DIVISIONS ne ""){
	print CFG "// biainputdivisions=\"$_BIA_INPUT_DIVISIONS\"\n";
    }
    close(CFG);
}
sub launch_optimizer {
    my $optimizer=shift;
    if ($optimizer eq "simplex"){
        my $cmd="$_SIMPLEX_CMD -t mt-class -f $_TUNING_DIR/$optimizer.cfg -v 3 > $_TUNING_DIR/$optimizer.out 2> $_TUNING_DIR/$optimizer.err";
	safesystem("$cmd");
	die("ERROR: Cannot find fmin (implementation of the downhill-simplex algorithm") unless (-x $_SIMPLEX_CMD);
    }elsif ($optimizer eq "spsa"){
	my $cmd="perl $BIA_ROOTDIR/tuning/spsa_fmin.pl -t mt-class -f $_TUNING_DIR/$optimizer.cfg -seed $_SEED -a $_SPSA_GRADIENT_FACTOR -maxNotImp2 20 -v 2 > $_TUNING_DIR/$optimizer.out 2> $_TUNING_DIR/$optimizer.err";
	safesystem("$cmd");
    }
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

    config=STRING   Path of BIA configuration file
    input=STRING   Path of source (foreign) development file
    refs=STRING   Path of reference development file

    root-dir=STRING   Base working directory of your experiment [default .]
    tuning-dir=STRING   Directory to store files used for tuning [default ./tuning]
    tmpdir=STRING   Directory to store temporary files [default ./tmp]
    jobs=STRING   Number of BIA jobs to be run at the same time
    paralmode=STRING   Parallelisation technique used (fork, qsub) [default fork]
    mem=STRING   Memory to ask to scheduler [default 15gb]
    walltime=STRING   Walltime to ask to scheduler [default 24:00:00]
    optimizer=STRING   Optimization algorithm: (downhill-) simplex, spsa or condor [default simplex]
    simplex-cmd=STRING   Path to the fmin binary
    simplex-ftol=float   Objective function relative threshold for convergence in simplex (if for all i,j: |f_i-f_j|/|f_i+f_j|<fTol => convergence)
    spsa-gradient-factor=float   Small "a" factor in SPSA. The new lambdas are: old*a(...)*gradient
    metric   Metric used to tune the weights [default bleu]
    no-mert   Don't do Minimum Error Rate Training before translating the DEV corpus [default: not activated]
    sent-align-postprocess   Command to post-process bia output to build parallel corpus, 
                             where [train.s], [train.t] and [alignment] refer to source and target corpus and bia output.
    smt-system   SMT system used in tuning [default phrase-based-moses]
    smt-system-config   Configuration file for SMT system used in tuning
    moses-cmd=STRING   Path to the moses binary 
    moses-script-dir=STRING   Path to the moses scripts directory 
    train-moses-options=STRING   Options of moses training script (for steps 1 to 3)
    verbose|v=INT   Verbose level
    debug   Debug mode
    man   Prints this info
    help|h   Prints this info

=cut
