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
#PARSING ARGUMENTS
my $optimizerConfigFile=shift;
open(CFG,$optimizerConfigFile) || die "Can't open condor config file $optimizerConfigFile!";

my ($_CONFIG, $_CORPUS, $_F, $_E, $_DEV_F,$_DEV_E, $_MERT_DEV_F,$_MERT_DEV_E, $__ROOT_DIR, $_TMPDIR, 
    $_JOBS, $_PARALMODE, $_MEM, $_WALLTIME, $_SMT_MEM, $_SENT_ALIGN_POSTPROCESS, $_SENT_ALIGN_WORDALIGN, $_SMT_WALLTIME, $_VERBOSE, $_SMT_SYSTEM, $_SMT_SYSTEM_CONFIG,
    $_METRIC, $_NO_MERT, $_MOSES_CMD, $_MOSES_SCRIPT_DIR, $_TRAIN_MOSES_OPTIONS, $_BIA_ROOTDIR, $_BIA_INPUT_DIVISIONS);
my @SORTED_FREEPAR_KEYS;
my $_KEEP_ROOT_DIR=0;
while (<CFG>){
    chomp;
    if (/ config="(.+)"/){$_CONFIG=$1;} # BIA configuration file
    if (/ tmpdir="(.+)"/){$_TMPDIR=$1;} # directory where to create temporary directories
    if (/ jobs="(.+)"/){$_JOBS=$1;}
    if (/ paralmode="(.+)"/){$_PARALMODE=$1;}
    if (/ mem="(.+)"/){$_MEM=$1;}
    if (/ walltime="(.+)"/){$_WALLTIME=$1;}
    if (/ smtmem="(.+)"/){$_SMT_MEM=$1;}
    if (/ smtwalltime="(.+)"/){$_SMT_WALLTIME=$1;}
    if (/ freepars="(.+)"/){@SORTED_FREEPAR_KEYS=split(" ",$1);}
    if (/ corpus="(.+)"/){$_CORPUS=$1;} # if not given, is extracted from BIA config file
    if (/ f="(.+)"/){$_F=$1;}           # if not given, is extracted from BIA config file
    if (/ e="(.+)"/){$_E=$1;}           # if not given, is extracted from BIA config file
    if (/ input="(.+)"/){$_DEV_F=$1;}
    if (/ ref="(.+)"/){$_DEV_E=$1;}
    if (/ mertinput="(.+)"/){$_MERT_DEV_F=$1;}
    if (/ mertref="(.+)"/){$_MERT_DEV_E=$1;}
    if (/ verbose="(.+)"/){$_VERBOSE=$1;}
    if (/ metric="(.+)"/){$_METRIC=$1;}
    if (/ nomert="(.+)"/){$_NO_MERT=$1;}
    if (/ sentalignpostprocess="(.+)"/){$_SENT_ALIGN_POSTPROCESS=$1;}
    if (/ sentalignwordalign="(.+)"/){$_SENT_ALIGN_WORDALIGN=$1;}
    if (/ smtsystem="(.+)"/){$_SMT_SYSTEM=$1;}
    if (/ smtsystemconfig="(.+)"/){$_SMT_SYSTEM_CONFIG=$1;}
    if (/ mosescmd="(.+)"/){$_MOSES_CMD=$1;}
    if (/ mosesscriptdir="(.+)"/){$_MOSES_SCRIPT_DIR=$1;}
    if (/ trainmosesoptions="(.+)"/){$_TRAIN_MOSES_OPTIONS=$1;}
    if (/ biarootdir="(.+)"/){$_BIA_ROOTDIR=$1;}
    if (/ biainputdivisions="(.+)"/){$_BIA_INPUT_DIVISIONS=$1;}
    # options to produce best alignment
    if (/ rootdir="(.+)"/){$__ROOT_DIR=$1;} # working directory
    if (/ keeprootdir/){$_KEEP_ROOT_DIR=1;} # do not remove working directory 
}
close(CFG);

my @SORTED_FREEPAR_VALUES;
my ($inputFile,$outputFile);
if ($optimizerConfigFile =~ /condor/){
    ($inputFile,$outputFile)=@ARGV;
}else{    
    for (my $i=0;$i<@ARGV;$i++){
	if ($i%2==0){
	    my $ii=$i/2;
	    if (substr($ARGV[$i],1) ne $SORTED_FREEPAR_KEYS[$ii]){die "ERROR: free parameter key order mismatch!\n";}
	}else{
	    push @SORTED_FREEPAR_VALUES,$ARGV[$i];
	}
    }
}

my $_ROOT_DIR="$_TMPDIR/$$";
$_ROOT_DIR = $__ROOT_DIR if $__ROOT_DIR;
my $cmd="mkdir -p $_ROOT_DIR";
safesystem($cmd);

# END PARSING ARGUMENTS

### MAIN

print STDERR "*** BIALIGN-SMT-EVAL *** ";
my $node=`uname -n`; chomp $node;
my $date=`date`; chomp $date;
print STDERR "$node $date\n";

my ($corpus,$f,$e);
my ($al_corpus,$al_f,$al_e)=&align($_ROOT_DIR);
if ($_CORPUS){$corpus=$_CORPUS;}
else{$corpus=$al_corpus;}
if ($_E){$e=$_E;}
else {$e=$al_e;}
if ($_F){$f=$_F;}
else {$f=$al_f;}
print STDERR "corpus:$corpus f:$f e:$e\n";
my $score;
# alignment postprocessing for sentence alignment
if ($_SENT_ALIGN_POSTPROCESS){
    print STDERR "Performing sentence alignment postprocessing at `date`";
    $corpus=&sentence_alignment_postprocess($f,$e);
}
print STDERR "Running SMT system at `date`";
if ($_SMT_SYSTEM eq "phrase-based-moses"){
    $score=&phrase_based_moses_translate($_ROOT_DIR,$corpus,$f,$e);
} else { # executes the command entered as $_SMT_SYSTEM, which must return the score
    my $cmd=$_SMT_SYSTEM;
    $cmd =~ s/\[train\.s\]/$corpus.$f/;
    $cmd =~ s/\[train\.t\]/$corpus.$e/;
    print STDERR "Executing: $cmd\n";
    $score = `$cmd`;
}
print STDERR "SCORE: $score\n\n";
print $score;

unless ($_KEEP_ROOT_DIR){
    my $cmd="rm -fr $_ROOT_DIR";
    safesystem($cmd);
}

####################################################################################

### ALIGN TRAINING CORPUS WITH BIA
sub align {
    my ($outputdir)=@_;
    my $output="$outputdir/aligned.bia";
    open(CFG,"<$_CONFIG") || die "$_CONFIG file opening error !";
    my $pars="";
    my %params=();
    my ($numSent,$corpus,$f,$e);
    my %freeparkeys;
    foreach my $key (@SORTED_FREEPAR_KEYS){
	$freeparkeys{"-$key"}=1;
    }
    my ($srcBiaInput,$trgBiaInput);
    while(<CFG>){
	chomp;
	if ($_ !~ /\/\//){
	    my ($key,$val)=split / /;
	    if (! exists($freeparkeys{$key})){
		# this is not a free parameter, so we add it to the params chain
		$params{$key}=$val;
		if ( $key ne "-n-best-list"){
		    $pars.="$key $val ";
		}
		if ($key eq "-is"){
		    $numSent=`wc -l $val|cut -d ' ' -f1`;chomp $numSent;
		    $srcBiaInput=$val;
		    if ($val =~ /(.+)\.([^\. ]+)$/){
			$corpus=$1;
			$f=$2;
		    }
		}
		if ($key eq "-it" && $val =~ /\.([^\. ]+)$/){
		    $trgBiaInput=$val;
		    $e=$1;
		}
	    }
	}
    }

    my $nbestOutputStr="";
    my $nbestOutput="";
    if (exists($params{"-n-best-list"})){
	$nbestOutput="$outputdir/nbest".$params{"-n-best-list"}.".bia";
	$nbestOutputStr="-n-best-list $nbestOutput ".$params{"-n-best-list"};
    }

    for (my $i=0;$i<@SORTED_FREEPAR_KEYS;$i++){
        $pars.="-$SORTED_FREEPAR_KEYS[$i] $SORTED_FREEPAR_VALUES[$i] ";
    }

    my $cmd;
    if ($_JOBS==1){
	$cmd="$_BIA_ROOTDIR/bia -o $output $nbestOutputStr $pars";
	safesystem("$cmd");
    }else{
	my $brange=1;
	my $erange=0;
	my $range="";
	my $rangeFileNames="";
	my $rangeNbestFileNames="";
	my $taskfile="$_ROOT_DIR/align.task";
	my $pids="";
	my @INPUT_DIVISIONS;
	if ($_BIA_INPUT_DIVISIONS){
	    @INPUT_DIVISIONS=split(/,/,$_BIA_INPUT_DIVISIONS);
	}else{
	    # WORD-NUMBER-BASED SPLITTING OF BIA INPUT FILES (GIVEN THE NUMBER OF PROCESSES, $_JOBS)
	    open(IS,"$srcBiaInput") || die "Cannot open source bia input ($srcBiaInput)\n";
	    open(IT,"$trgBiaInput") || die "Cannot open target bia input ($trgBiaInput)\n";
	    my $total=0;
	    my @wordNumbers;
	    while (<IS>){
		my $nswords=split (/ /);
		my $tline=<IT>; 
		my $ntwords=split(/ /,$tline);
		my $nwords=($nswords+$ntwords)/2;
		push @wordNumbers,log($nwords)*$nwords**2;
		$total+=log($nwords)*$nwords**2;
	    }
	    close(IS);
	    close(IT);
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
		@INPUT_DIVISIONS=@lines;
	    }
	}
	if ( $_PARALMODE eq "qsub"){
	    open(TASKF,">$taskfile"); system("chmod 755 $taskfile");
	}
	for (my $i=0; $i<$_JOBS; $i++){
	    if ($i == $_JOBS-1){
		$erange=$numSent;
	    }else{
		if (@INPUT_DIVISIONS > $i){
		    $erange=$INPUT_DIVISIONS[$i];
		}else{
		    $erange=$brange-1+int($numSent/$_JOBS);
		}
	    }
	    $range="$brange-$erange";
	    $rangeFileNames.="$output.$range ";
	    if (exists($params{"-n-best-list"})){
		$rangeNbestFileNames.="$nbestOutput.$range ";
		$nbestOutputStr="-n-best-list $nbestOutput.$range ".$params{"-n-best-list"};
	    }

	    $brange=$erange+1;
	    $cmd="$_BIA_ROOTDIR/bia -o $output.$range $nbestOutputStr $pars -range $range";
	    if ( $_PARALMODE eq "qsub"){
		print TASKF "$cmd\n";
	    }else{
		die "could not fork" unless defined(my $pid = fork);
		if (!$pid) { #child execs                                                                                                                            
		    print STDERR "$cmd\n";
		    exec "$cmd";
		    die "exec of $cmd failed";
		}
	    }
	}
	if ( $_PARALMODE eq "qsub"){
	    close(TASKF);
	    $cmd="$ENV{HOME}/scripts/qsub_taskfarm.pl -file $taskfile -mem $_MEM -walltime $_WALLTIME";
	    safesystem("$cmd") or die;
	}else{
	    while (wait() != -1) {}
	}
	safesystem("cat $rangeFileNames > $output");
	if (exists($params{"-n-best-list"})){
	    safesystem("cat $rangeNbestFileNames > $nbestOutput");
	}
    }
    return ($corpus,$f,$e);
}
sub sentence_alignment_postprocess {
    my ($f,$e)=@_;

    my $cdir="$_ROOT_DIR/corpus";
    my $cmd="mkdir -p $cdir";
    safesystem($cmd);
    my $train="$cdir/train";
    $cmd=$_SENT_ALIGN_POSTPROCESS;
    $cmd =~ s/\[train\.s\]/$train.raw.$f/;
    $cmd =~ s/\[train\.t\]/$train.raw.$e/;
    $cmd =~ s/\[alignment\]/$_ROOT_DIR\/aligned.bia/;
    print STDERR "# Creating parallel corpus from sentence-alignments\n";
    safesystem($cmd);

    print STDERR "# Filter out long sentences from parallel corpus";
    $cmd="$_BIA_ROOTDIR/tuning/clean-corpus-n-frac.perl $train.raw $f $e $train 1 100 3";
    safesystem($cmd);

    # align
    if ($_SENT_ALIGN_WORDALIGN eq "moses"){
	my $mdir="$_ROOT_DIR/model"; my $cmd="mkdir -p $mdir"; safesystem($cmd);
	my $smp=int($_JOBS/2); # number of CPUs for mgiza
	print STDERR "# Align parallel corpus with mgiza (threads:$smp)\n";
	$cmd="
    	$_MOSES_SCRIPT_DIR/training/train-model.perl \\
	    -first-step 1 -last-step 3 \\
	    -scripts-root-dir $_MOSES_SCRIPT_DIR \\
	    -root-dir $_ROOT_DIR -model-dir $mdir \\
	    -corpus $train -f $f -e $e \\
	    -mgiza -mgiza-cpus $smp -parallel $_TRAIN_MOSES_OPTIONS &> /dev/null";
	safesystem($cmd);
    }
    return $train;
}
sub phrase_based_moses_translate {
    my ($_ROOT_DIR,$corpus,$f,$e)=@_;
    my $mdir="$_ROOT_DIR/model"; my $cmd="mkdir -p $mdir"; safesystem($cmd);
    # TRAINING
    my ($alignment,$alignment_file);
    if ($_SENT_ALIGN_POSTPROCESS){
	$alignment="grow-diag-final";
	$alignment_file="$_ROOT_DIR/model/aligned";	
    }else{
	$alignment="bia";
	$alignment_file="$_ROOT_DIR/aligned";	
    }
    my $cmd="
    	$_MOSES_SCRIPT_DIR/training/train-model.perl \\
            --first-step 4 -last-step 9 \\
	    --root-dir $_ROOT_DIR -model-dir $mdir \\
	    --corpus $corpus --f $f --e $e \\
            --reordering msd-bidirectional-fe \\
            --alignment $alignment \\
	    --alignment-file $alignment_file $_TRAIN_MOSES_OPTIONS 1>&2";
    safesystem($cmd);
    
    # MERT
    my $tdir="$_ROOT_DIR/mert"; my $cmd="mkdir -p $tdir"; safesystem($cmd);
    if ($_NO_MERT){
	if ($_SMT_SYSTEM_CONFIG){
	    &replace_weights_in_moses_ini("$mdir/moses.ini",$_SMT_SYSTEM_CONFIG);
	    $cmd="cp $mdir/moses.ini $tdir";
	    safesystem($cmd);
	}
    }else{
	#mert
	my $mertseed=6894385819;
	my ($decoder,$decoder_flags)=split(/ /,$_MOSES_CMD,2);
	$cmd="
	$_MOSES_SCRIPT_DIR/training/mert-moses-new.pl \\
	    --rootdir $_MOSES_SCRIPT_DIR \\
	    --working-dir=\"$tdir\" \\
	    --mertdir=\"$_MOSES_SCRIPT_DIR/../mert\" \\
	    --mertargs=\" -r $mertseed\" \\
            --closest \\
	    --decoder-flags=\" $decoder_flags -v 0 \" \\
	    $_MERT_DEV_F $_MERT_DEV_E $decoder $mdir/moses.ini > $tdir/moses.log  2>&1"; 
	safesystem($cmd);
    }
    
    # TRANSLATE
    $cmd="$_MOSES_SCRIPT_DIR/training/filter-model-given-input.pl $_ROOT_DIR/filtered $tdir/moses.ini $_DEV_F 1>&2";
    safesystem($cmd);

    $cmd="$_MOSES_CMD -f $_ROOT_DIR/filtered/moses.ini  < $_DEV_F > $_ROOT_DIR/moses.output 2> /dev/null";
    safesystem($cmd);

    $cmd="$_MOSES_SCRIPT_DIR/generic/multi-bleu.perl $_DEV_E < $_ROOT_DIR/moses.output > $_ROOT_DIR/moses.output.eval";
    safesystem($cmd);
    
    my $score=`grep BLEU $_ROOT_DIR/moses.output.eval | sed -e 's/,//' | awk '{printf(\"%6.4f\\n\",\$3)}'`; chomp $score;
    $score=$score*-0.01; # for SPSA tuning, score should be < 1

    return $score;
}
sub replace_weights_in_moses_ini {
    my ($defaultWeightCfg,$newWeightCfg)=@_;
    
    open(NEW,$newWeightCfg) || die "Cannot open $newWeightCfg file\n";
    my @NEW_WEIGHT_CFG=<NEW>;
    close(NEW);

    my %NEW_WEIGHTS;
    my $i=0;
    while ($i<@NEW_WEIGHT_CFG){
	my $line=$NEW_WEIGHT_CFG[$i];chomp $line;
	if ($line =~ /\[(weight\-.)\]/){
	    my $wname=$1;
	    $i++; $line=@NEW_WEIGHT_CFG[$i];
	    while ($line =~ /^([\d\.\-]+)$/){
		push @{$NEW_WEIGHTS{$wname}},$1;
		$i++;
		$line=$NEW_WEIGHT_CFG[$i];chomp $line;
	    }
	    $i--;
	}else{
	    $i++;
	}
    }
    open(DEF,$defaultWeightCfg) || die "Cannot open $defaultWeightCfg file for reading\n";
    my @DEFAULT_WEIGHT_CFG=<DEF>;
    close(DEF);

    my $i=0;
    while ($i<@DEFAULT_WEIGHT_CFG){
	my $line=$DEFAULT_WEIGHT_CFG[$i];
	if ($line =~ /\[(weight\-.)\]/){
	    my $wname=$1;
	    for (my $j=0;$j<@{$NEW_WEIGHTS{$wname}};$j++){
		$i++;
		$DEFAULT_WEIGHT_CFG[$i]="$NEW_WEIGHTS{$wname}[$j]\n";
	    }
	}
	$i++
    }
    # writing new moses.ini
    open(DEF,">$defaultWeightCfg") || die "Cannot open $defaultWeightCfg file for writing\n";
    foreach my $line (@DEFAULT_WEIGHT_CFG){
	print DEF "$line";
    }
    close(DEF);
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
