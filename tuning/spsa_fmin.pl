#! /usr/bin/perl

########################################################################
# Author:  Patrik Lambert (lambert@gps.tsc.upc.edu)
# Description:
#
#-----------------------------------------------------------------------
#
#  Copyright 2006 by Patrik Lambert
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
########################################################################

use lib `echo $USER_DIR`."/dev/perl/lib";
use strict;
use Getopt::Long;
use Pod::Usage;
use File::Temp;
#Debug:
use Dumpvalue;
my $dumper = new Dumpvalue;

my $TRUE = 1;
my $FALSE = 0;
my $INFINITY = 9999999999;
my $TINY = 1 - $INFINITY / ($INFINITY + 1);

#PARSING COMMAND-LINE ARGUMENTS
my %opts=();

# important default values:
$opts{seed}=35000;
$opts{alpha}=0.602;
$opts{gamma}=0.101;
$opts{bigA}=2;
$opts{a}=8;
$opts{c}=0.25;
$opts{grain}=0.0004;
$opts{mineval}=70;
$opts{temp}=0.005;

# not important default values (correspond to options tried):
$opts{maxak}=2*$opts{a}/($opts{bigA}+1)**$opts{alpha};
$opts{akincr}=0;
$opts{cc}=0.08;
$opts{cd}=0;
$opts{lc}=0;
$opts{cp}=0;
$opts{dIncr}=-10;
$opts{cIncr}=0;
# probability of a new theta vector to be accepted if y(theta) <= y(old_theta)+grain:
$opts{accept}=-1;
$opts{accpp}=0;
$opts{plusminus}=1;
$opts{orig}=0;
$opts{eco}=1;
$opts{norm}=0;
$opts{zero}=0;
$opts{maxAbsVariation}=0.4;
$opts{minAbsVariation}=0.005;
$opts{maxNotImp}=10; 
$opts{maxNotImp2}=10; 
$opts{dist}=1;
#parsing command-line:
GetOptions(\%opts,'verbose|v=i','task|t=s','configFile|f=s','c=f','a=f','bigA=i','grain=f','accept=f','accpp=i','temp=f','mineval=i','maxNotImp=i','maxNotImp2=i','akincr=i','plusminus=i','orig=i','maxak=f','alpha=f','gamma=f','eco=i','norm=i','zero=i','dist=i','dIncr=i','cIncr=i','cc=f','cd=f','lc=i','cp=i','seed=s','man','help|?') or pod2usage(1);
#$dumper->dumpValue(\%opts);

#check presence of all required arguments:
if ($opts{man}){
    pod2usage(-verbose=>2);
}elsif ($opts{"help"}){
    pod2usage(1);
}elsif( !(exists($opts{configFile})) ){
    pod2usage(-msg=>"Required arguments missing",-verbose=>0);
}
##END PARSING COMMAND-LINE ARGUMENTS

# Read configuration file
open(CONF,"<$opts{configFile}") || die "Problem opening config file ($opts{configFile})\n";
my @params;
my @theta;
my @params_infLim; # params minimum allowed value
my @params_supLim; # params maximum allowed value
while (my $line=<CONF>){
    chomp $line;
    $line =~ s/^\s+//g;
    $line =~ s/\s+$//g;
    $line =~ s/\s{2,}/ /g;	
    if ($line !~ m/^\/\// && $line !~ m/^$/){
	my @vars=split / /,$line;
	my $varname=shift @vars;
	if ($varname ne "freeParam"){
	    $opts{$varname}=join(" ",@vars);
	}else{
	    push @params,$vars[0];
	    push @theta,$vars[1];
	    my $range=$vars[2];
	    $range=~s/\[//;$range=~s/\]//;
	    my ($inflim,$suplim)=split /,/,$range;
	    if ($inflim eq ""){$inflim=-1.0*$INFINITY;}
	    push @params_infLim,$inflim;
	    if ($suplim eq ""){$suplim=$INFINITY;}
	    push @params_supLim,$suplim;
	}
    }
}
#$dumper->dumpValue(\@params_infLim);
#$dumper->dumpValue(\@params_supLim);
my $fixedParams=$opts{fixedParams};
my $nDim=scalar(@params);
my $verbose=$opts{verbose};
my $refstr;
foreach my $ref (@{$opts{r}}){
    $refstr.="-r $ref ";
}
my $type="w"; #wer
$type="b"; #bleu

# Initialization and coefficient selection
my ($a)=($opts{a});
my $c=$opts{c};
if ($verbose>2){print "SEED:$opts{seed}\n";}
srand($opts{seed});
my $maxEval = $opts{maxIter};
my $minEval = $opts{mineval};
my $numRnb = $maxEval*$nDim*5;
#to avoid interferences with random filename generation, generate all random numbers now.
my @rnbs=();
for (my $i=0;$i<$numRnb;$i++){
    my $rnb=rand;
    push @rnbs,$rnb;
}

my $A=$opts{bigA};
my $alpha=$opts{alpha};
my $gamma=$opts{gamma};
my $cTol = $opts{cTol};
my $grain = $opts{grain};
my $accpp=$opts{accpp};

my $numIter=0;
my $numEval=0;
my $convergeNow=0;
my $disturbed=0;
my $numSuccessiveLittleChange=0;
my $maxSuccessiveLittleChange=3;
my $numRejected=0;
my $numAllminNotImproved=0; #nb of iterations without improving minimum

my $y_min=mtErr(\@theta,\@params,$type);
$numEval++;
my $y_allmin=$y_min;
my @theta_allmin=@theta;
print "Verbose:$verbose\n";
if ($verbose>0){
    print " \nLambdas:\n=> y(",joinf('%7.6f',\@theta),")=$y_min\n\n"; 
}
while ($numSuccessiveLittleChange<$maxSuccessiveLittleChange && $numEval<$maxEval){
    if ($verbose>0){
	print "evals $numEval / $maxEval (iters:$numIter ; Allmin not improved:$numAllminNotImproved)\n";
    }
    # STEP 1 Initialization and coefficient selection
    ########
    my $a_k=$a/($A+$numIter+1)**$alpha;
    if ($opts{akincr}==1){
	my $rejected_a_k=$a_k + $a_k*$numRejected/2;
	if ($rejected_a_k<$opts{maxak}){
	    $a_k=$rejected_a_k;
	}else{
	    $a_k=$opts{maxak};
	}
	if ($verbose>1){print "numrejected:$numRejected a_k=min($opts{maxak},$a(1+$numRejected/2)/($A+$numIter+1)**$alpha) =  $a_k\n";}
    }else{
	if ($verbose>1){print "a_k=$a/($A+$numIter+1)**$alpha = $a_k\n";}
    }
    my $c_k=$c/($numIter+1)**$gamma;
    if ($opts{cp}){
	$c_k=$c/($numIter+1)**$gamma-0.12*(1-exp(-0.0001*$numIter**2));
	if ($verbose>1){print "c_k=$c/($numIter+1)**$gamma-0.12*(1-exp(-0.0001*$numIter**2)) = $c_k\n";}
    }elsif ($opts{lc}){
	$c_k=-0.0017*$numIter+0.25;
	if ($verbose>1){print "c_k= -0.0017*$numIter+0.25 = $c_k\n";}
    }else{
	if ($verbose>1){print "c_k=$c/($numIter+1)**$gamma = $c_k\n";}
    }


    # STEP 2 delta vector
    ########
    my @delta=();
    for (my $p=0;$p<$nDim;$p++){
        # random number in [0,1[
	my $rnb=shift @rnbs;
	if ($opts{zero}){
	    if ($rnb>=0.66){push @delta,1}
	    elsif ($rnb >=0.33){push @delta,0}
	    else {push @delta,-1}
	}else{
	    if ($rnb>=0.5){push @delta,1}
	    else {push @delta,-1}
	}
    }
    if ($verbose>1){print "delta: ",join(" ",@delta),"\n";}

    # STEP 3 Loss Function Evaluations
    ########
    my @thetaPlus =();
    my @thetaMinus=();
    for (my $p=0;$p<$nDim;$p++){
	$thetaPlus[$p]=$theta[$p]+$c_k*$delta[$p];
	if ($thetaPlus[$p]<$params_infLim[$p]){$thetaPlus[$p]=$params_infLim[$p];}
	elsif ($thetaPlus[$p]>$params_supLim[$p]){$thetaPlus[$p]=$params_supLim[$p];}
	if ($opts{eco}){
	    $thetaMinus[$p]=$theta[$p];
	}else{
	    $thetaMinus[$p]=$theta[$p]-$c_k*$delta[$p];
	    if ($thetaMinus[$p]<$params_infLim[$p]){$thetaMinus[$p]=$params_infLim[$p];}
	    elsif ($thetaMinus[$p]>$params_supLim[$p]){$thetaMinus[$p]=$params_supLim[$p];}
	}
    }
    my $yPlus=mtErr(\@thetaPlus,\@params,$type);
    $numEval++;
    if (($numEval % 1) == 0 && $verbose > 1){
	if ($yPlus<$y_allmin){print "BEST AFTER $numEval EVALS:$yPlus ",joinf("%7.6f",\@thetaPlus),"\n";}
	else {print "BEST AFTER $numEval EVALS:$y_allmin ",joinf("%7.6f",\@theta_allmin),"\n";}
    }
    if ($verbose>1){print "Theta+ = theta + c_k*delta: ",joinf("%7.6f",\@thetaPlus)," =>y+ = y(theta+): $yPlus\n";}
    my $yMinus;
    if ($opts{eco}){
	$yMinus=$y_min;
    }else{
	$yMinus=mtErr(\@thetaMinus,\@params,$type);
	$numEval++;
    }
    if ($verbose>1){print "Theta- = theta - c_k*delta: ",joinf("%7.6f",\@thetaMinus)," =>y- = y(theta-): $yMinus\n";}

    # STEP 4 Gradient approximation
    ########
    my @grad=();
    my $ydiff=$yPlus-$yMinus;
    my $denom;
    if ($opts{eco}){$denom=$c_k;}
    else {$denom=2*$c_k;}
    for (my $p=0;$p<$nDim;$p++){
	$grad[$p]=$ydiff*$delta[$p]/$denom;
    }
    if ($verbose>1){print "Gradient approx: (y+ - y-)/2*c_k [",join(",",@delta),"]: ",joinf("%7.6f",\@grad),"\n";}
    # If |a_k*grad_k| is too low or too high, this point won't be a really better one => don't calculate error
    my $variation=$ydiff*$a_k/$denom;
    if (!$opts{orig} && $numIter>$nDim){
	if (abs($variation) > $opts{maxAbsVariation} || abs($variation)<$opts{minAbsVariation}){
	    $numAllminNotImproved++;
	    if ($opts{plusminus} && $yPlus<$y_min){
		$y_min=$yPlus;
		@theta=@thetaPlus;
		if ($y_min < $y_allmin-$grain){
		    $numAllminNotImproved=0;
		}
		if ($y_min<$y_allmin){
		    $y_allmin=$y_min;
		    @theta_allmin=@theta;
		}
	    }
	    $numRejected=0;
	    if ($verbose>1){printf "|a_k*grad_k|=%5.4f outside allowed range: Try another Delta Vector\n\n",$variation;}
	    $numIter++;
	    next;
	}
    }

    # STEP 5 Updating theta estimate
    ########
    my @oldTheta=@theta;
    my $withinCTol=1;
    if ($verbose>1){print "a_k*grad_k:";}
    for (my $p=0;$p<$nDim;$p++){
	my $variation_p;
	if ($opts{norm}){
	    $variation_p = $a_k*$grad[$p]*0.1/abs($grad[$p]);
	}else{
	    $variation_p = $a_k*$grad[$p];
	}
	if ($verbose>1){
	    printf " %7.6f",$variation_p;
	}
	$theta[$p]=$oldTheta[$p]-$variation_p;
	if ($theta[$p]<$params_infLim[$p]){$theta[$p]=$params_infLim[$p];}
	elsif ($theta[$p]>$params_supLim[$p]){$theta[$p]=$params_supLim[$p];}
	if ($theta[$p]-$oldTheta[$p]>$cTol){
	    $withinCTol=0;
	}
    }
    if ($verbose>1){print "\n";}
    if ($withinCTol==1){
	$numSuccessiveLittleChange++;
    }else{
	$numSuccessiveLittleChange=0;
    }
    if ($numSuccessiveLittleChange==$maxSuccessiveLittleChange){
	if ($numEval>$minEval){
	    print "CONVERGED !\n";
	    print "minimum encountered:\ny_allmin(",joinf("%7.6f",\@theta_allmin),")=$y_allmin\n";
	}else{
	    $numIter=0;
	    $numSuccessiveLittleChange=0;
	}
    }

    if (!$opts{orig}){
	# look if we accept theta
	my $y=mtErr(\@theta,\@params,$type);
	$numEval++;
	if (($numEval % 1) == 0 && $verbose > 1){
	    if ($y<$y_allmin){print "BEST AFTER $numEval EVALS:$y ",joinf("%7.6f",\@theta),"\n";}
	    else {print "BEST AFTER $numEval EVALS:$y_allmin ",joinf("%7.6f",\@theta_allmin),"\n";}
	}

	if ($verbose>0){
	    print " y(",joinf("%7.6f",\@theta),")=$y\n";
	}
	if ($verbose>2){print "y:$y  y_min-grain:".($y_min-$grain)."\n";}
	my $toImprove=$y_min-$grain;
	if ( ($opts{plusminus} && $y>=$toImprove && $yPlus>=$toImprove && $yMinus>=$toImprove) || (!$opts{plusminus} && $y>=$toImprove) ){
	    # FAILURE. See whether we accept it or not
	    $numAllminNotImproved++;
	    print "FAILURE\n";
	    my $failureAcceptanceProb;
	    #acceptance probability
	    if ($opts{accept}>=0){
		$failureAcceptanceProb=$opts{accept};
	    }else{
		$failureAcceptanceProb=acceptanceProb($numRejected,$numAllminNotImproved,$opts{temp},($y-$y_min)/$y_min,$accpp);
	    }
	    my $rnb=shift @rnbs;
	    if ($verbose>2){
		print STDERR "rnb=$rnb\n";
		print "rnb=$rnb failure acceptance:$failureAcceptanceProb\n";
	    }
	    if ($rnb>=$failureAcceptanceProb){
		# we don't accept this new theta vector
		@theta=@oldTheta;
		if ($verbose>2){print "Didn't accept\n";}
		$numRejected++;
	    }else{
		$numRejected=0;
		$y_min=$y;
	    }
	    # if we improve y_allmin, we change y_allmin and @theta_allmin
	    if ($y<$y_allmin || ($opts{plusminus} && ( $yPlus < $y_allmin || $yMinus<$y_allmin))){
		if (!$opts{plusminus}){
		    $y_allmin=$y;
		    @theta_allmin=@theta;
		}else{
		    if ($y<=$yPlus && $y<=$yMinus){
			$y_allmin=$y;
			@theta_allmin=@theta;
		    }elsif ($yPlus<=$yMinus){
			#the best is yPlus
			$y_allmin=$yPlus;
			@theta_allmin=@thetaPlus;
		    }else{
			#the best is yMinus
			$y_allmin=$yMinus;
			@theta_allmin=@thetaMinus;
		    }
		}
	    }
	}else{
	    #SUCCESS. Keep the best one.
	    if (!$opts{plusminus}){
		$y_min=$y;
	    }else{
		if ($y<=$yPlus && $y<=$yMinus){
		    $y_min=$y;
		}elsif ($yPlus<=$yMinus){
		    #the best is yPlus
		    $y_min=$yPlus;
		    @theta=@thetaPlus;
		}else{$c/($numIter+1)**$gamma;
		    #the best is yMinus
		    $y_min=$yMinus;
		    @theta=@thetaMinus;
		}
	    }
	    if ($y_min<$y_allmin-$grain){
		$numAllminNotImproved=0;
	    }else{
		$numAllminNotImproved++;
	    }
	    if ($y_min<$y_allmin){
		$y_allmin=$y_min;
		@theta_allmin=@theta;
	    }
	    $numRejected=0;
	}
    }else{ # if $opts{orig}==1
	# we keep in memory best lambda seen
	if ($yMinus<$y_allmin || $yPlus<$y_allmin){
	    if ($yMinus<$yPlus){
		$y_allmin=$yMinus;
		@theta_allmin=@thetaMinus;
	    }else{
		$y_allmin=$yPlus;
		@theta_allmin=@thetaPlus;
	    }
	    $numAllminNotImproved=0;
	}
    }
    if ($verbose>0){
	print " \n Current Lambdas: ",joinf("%7.6f",\@theta),"\n"; 
    }
    if ($verbose>1){
	print " Minimum: y_allmin(",joinf("%7.6f",\@theta_allmin),")= $y_allmin\n\n"; 
    }
    if ($opts{dist} && $numAllminNotImproved >= $opts{maxNotImp2} && $disturbed && !$convergeNow){
	print "\nCONVERGE NOW mode triggered: we try to converge close to minimum found up to now\n\n";
	# trigger "convergeNow" mode: try to converge 
	$convergeNow=1;
	$numIter+=$opts{cIncr};
	$numAllminNotImproved=0;
	@theta=@theta_allmin;
	$y_min=$y_allmin;
	$opts{accept}=0;
	if ($opts{cc}>0){
	    $c=$opts{cc};
	}
    }elsif($opts{dist} && $numAllminNotImproved >= $opts{maxNotImp} && !$disturbed){
	print "\nSYSTEM DISTURBED: we now accept 0 in Delta vector\n\n";
	$disturbed=1;
	# if numIter is not too high, we don't decrement it too much
	my $diffI=$numIter+$opts{dIncr};
	if ($diffI<0){$diffI=0;}
	if ($diffI<abs($opts{dIncr})){
	    $numIter -= $diffI;
	}else{
	    $numIter += $opts{dIncr}; 
	}
	$numAllminNotImproved=0;
	$opts{zero}=1;
	if ($opts{cd}>0){
	    $c=$opts{cd};
	}
    }else{
	$numIter++;
    }
}

sub mtErr {
    my ($lambdas,$params)=@_;
    my $lambstr="";
    for (my $i=0;$i<@$params;$i++){
	$lambstr.=" ".$params->[$i]." ".$lambdas->[$i];
    }
    if ($opts{task} eq "mt-class"){
	my $err=`$opts{mtSystem} $fixedParams $lambstr`;
	return $err;
    }
    # task is "mt-error"
    my ($fh, $tmptran) = mkstemp( "tranXXXXXX" );
    if (substr($opts{mtSystem},0,7) eq "rescore"){
	if ($verbose>2){
	    print STDERR "$opts{mtSystem} $fixedParams $lambstr -fileout $tmptran\n";
	}
	system("$opts{mtSystem} $fixedParams $lambstr -fileout $tmptran");
    }elsif (substr($opts{mtSystem},0,5) eq "marie"){
	if ($verbose>2){
	    print STDERR "$opts{mtSystem} $fixedParams $lambstr -o $tmptran\n";
	}
	system("$opts{mtSystem} $fixedParams $lambstr -o $tmptran");
    }else{
	if ($verbose>2){
	    print STDERR "$opts{mtSystem} $fixedParams $lambstr < $opts{src} > $tmptran\n";
	}
	system("$opts{mtSystem} $fixedParams $lambstr < $opts{src} > $tmptran");
    }
    my $evalStr = $opts{evalSystemCall};
    $evalStr =~ s/\[transl\]/$tmptran/;
    if ($verbose>2){
	print STDERR "$evalStr\n";
    }
    my $err=`$evalStr`;
    if ($opts{worstScore}==0){
	$err=-1.0*$err;
    }
    system("rm -f $tmptran");
    chomp $err;
    return $err;
}

sub acceptanceProb {
    my ($numIter,$numAllminNotImproved,$T,$deltaErr,$accpp)=@_;
    if ($verbose>2){
	print "acceptance prob: |deltaErr/Err|:$deltaErr T:$T iters:$numIter numNotImproved:$numAllminNotImproved exponent:".($deltaErr/($T*($numIter+1)))."\n";
    }
    my $help=sqrt($numAllminNotImproved)-3;
    if ($help < 0 || !$accpp){
	return exp(-abs($deltaErr)/($T*($numIter+1)));
    }else{
	return exp(-abs($deltaErr)/($T*($numIter+1+$help)));  #acc+++
    }
#    return exp(-abs($deltaErr)/($T*($numIter+1+sqrt($numAllminNotImproved*2))));
#    return exp(-abs($deltaErr)/($T*($numIter+1+0.2*$numAllminNotImproved))); #acc++

}

sub joinf {
    my $format=shift;
    my $arr=shift;
    my $str;
    foreach my $val (@$arr){
#		printf($fileHandle "   Recall    = %5.4f\n",$this->{possibleRecall});
	$str.=sprintf "$format ",$val;
    }
    return $str;
}
print STDERR "$0 done.\n";
# DEBUG: print object structure:
#$dumper->dumpValue($refToObject);
#$t0 = (times)[0];
#$elapsed = (times)[0]-$t0;


__END__

=head1 NAME

spsa_fmin.pl

=head1 SYNOPSIS

perl spsa_fmin.pl [options] required_arguments

Required arguments
    
    -f configuration_file

Main Options

    -c c constant (default 0.25)
    -a a constant (default 8)
    -A A constant (default 2)
    -maxeval    Max number of function evaluations
    -mineval    Don't accept convergence until this number of function evaluations
    -temp Function to give acceptance prob=exp[-((y-y_min)/y) / (temp*(numIter+1))]   (default 0.005)
    -grain new function value must be less than f_min-grain (default 0.0004)
    -seed INT    Seed for ran function (default 35000)
    -verbose 0|1|2|3 verbose level (default 0)
    -man
    -help

Other Options (some of them possibly not working any more)

    -accept -1|[0,1] Prob to accept a value lower than min. -1: prob given by a function (default -1)
    -orig 0|1 Set the original SPSA alg. (eval of f only to approx gradient) (default 0)
    -eco grad_k=y+ - y_k instead of y+ - y- (saves an evaluation at each iteration) (default 1)
    -norm gradient is normalized to 1 so that only its direction counts (default 0)
    -plusminus: y+ and y- (gradient values) are kept if they improve actual best (default 1)
    -zero Delta values are chosen in [-1,0,1] instead of [-1,1]
    -maxNotImp    # of iters without improving best minimum when "disturbed" is triggered (if dist=0)
    -maxNotImp2    # of iters without improving best minimum when "convergeNow" is triggered (if dist=1)
    -dist    Allows to disturb the system after maxNotImp iters without improvement, and trigger convergence now
    -dIncr   Nb of iters the system is incremented with when system is disturbed
    -cIncr   Nb of iters the system is incremented with when entering "converge now" mode
    -accpp    adds a dependency in num iters without improving minimum in acceptance prob
    -cc    In "converge now" mode, the value of c is changed to this value (if >0)
    -cd    In "system disturbed" mode, the value of c is changed to this value (if >0)
    -lc    pseudo-linear c_k formula
    -cp    corrected formula to calculate c
    -akincr
    -maxak
    

=head1 ARGUMENTS

=over 8

=item B<-f,--filename=name>

File name

=head1 OPTIONS

=item B<--help, --?>

Prints a help message and exits.

=item B<--man>

Prints the manual and exits.

=head1 DESCRIPTION


=head1 AUTHOR

Patrik Lambert <lambert@gps.tsc.upc.edu>

=head1 COPYRIGHT AND LICENSE

Copyright 2006 by Patrik Lambert

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (version 2 or any later version).

=cut
