#! /bin/bash

train(){
    
    _script=$wdir/train-align-$discount.bash
    log=$biadir/log
    if [ ! -e $biadir/${src}-${trg}.pha-d$discount.prn$pruningScore-9-0-20-200 ]; then
	echo -n ""> $_script; chmod +x $_script;
 	echo "
 	$BIA_ROOTDIR/training/train-models.pl --root-dir $wdir \
 	    --corpus $cdir/train --f $src --e $trg \
            --model-dir $biadir \
 	    --alignment-file $wdir/model/aligned.grow-diag-final-and \
 	    --linkCost-discount $discount \
            --pruning-score 1+2+3 >> $log 2>> $log
 " >> $_script;
	echo "BIA training" >> $log
	bash $_script;
    fi

}
tune(){
    fixedpars=( is  it  isf  itf  iwda  n  b  t  m  c  ct  first  exp  dir  doba  aoba  multiple-stacks );
    fixedvals=( $is $it $isf $itf $iwda $n $b $t $m $c $ct $first $exp $dir $doba $aoba $multstacks     );
    # WRITE BIA CONFIG FILE
    biaconfig=$tdir/bia.cfg
    echo -n "" > $biaconfig
    nFixedpars=${#fixedpars[@]}
    for i in `seq 0 $[nFixedpars-1]`
    do
	echo "-${fixedpars[$i]} ${fixedvals[$i]}" >> $biaconfig
    done
    nFreepars=${#freepars[@]}
    for i in `seq 0 $[nFreepars-1]`
    do
	echo "-${freepars[$i]} ${freevals[$i]}" >> $biaconfig
    done
  
    if [ ! -e $tdir/$optimizer.out ]; then
	_script=$tdir/launch-tuning.bash; mkdir -p $tdir
	echo "
$BIA_ROOTDIR/tuning/tune-model-weights.pl --config=$tdir/bia.cfg --input=$refdir/$decodeDev.$src --refs=$refdir/$decodeDev.$trg. \
    --mert-input $refdir/$mertDev.$src --mert-refs $refdir/$mertDev.$trg. \
    --corpus=$cdir/train --f $src --e $trg \
    --jobs=$jobs --tuning-dir=$tdir $mert \
    --optimizer=$optimizer --smt-system='$smtSystem' --spsa-gradient-factor=$spsa_a \
    --moses-script-dir=$MOSES_ROOTDIR/scripts --tmpdir=$tmpdir \
    --train-moses-options=\"-bin-dir /usr/local/bin -lm 0:4:$wdir/model/$trg.sblm:0 $memscore\" \
    --moses-cmd=\"$MOSES_CMD $mosesSB -ttl 20 -threads $jobs\" 2> $tdir/launch-tuning.log \
" > $_script; chmod +x $_script;
	bash $_script;
    fi
}
evaluate(){
    # get best weights
    bestweights=`grep BEST $tdir/$optimizer.out|tail -1| cut -d ' ' -f 5-`
    freevals=( $bestweights );
    nFreevals=${#freevals[@]}
    freeParSt=""
    for i in `seq 0 $[nFreevals-1]`
    do
	freeParSt=$freeParSt"-${freepars[$i]} ${freevals[$i]} " 
    done
    if [ -e $tdir/$optimizer.cfg ]; then
	finaldir=$tdir/final-$optimizer; mkdir -p $finaldir
	_config=$finaldir/$optimizer.cfg
	grep -v ' biarootdir=' $tdir/$optimizer.cfg | grep -v ' smtsystem=' | grep -v ' jobs=' | grep -v ' biainputdivisions=' > $_config
	echo "// smtsystem=\"\"
// jobs=\"$jobs\"
// keeprootdir
// rootdir=\"$finaldir\"
// biarootdir=\"$BIA_ROOTDIR\"">> $_config
    
	_script=$finaldir/align.bash
	echo "$BIA_ROOTDIR/tuning/bialign-smt-eval.pl $_config $freeParSt &> $finaldir/log;" > $_script; chmod +x $_script
	bash $_script;
    fi
}
reset(){
    is=$cdir/train.$src
    it=$cdir/train.$trg
    isf=$biadir/fertility.$src
    itf=$biadir/fertility.$trg
    iwda=$biadir/${src}-${trg}.pha-d$discount.prn1+2+3-9-0-20-200
    n=3; b=5; t=5; m=1; c=12; ct=3; first=avge; exp=word; doba=1;aoba=2; dir=asIat-asGDFAat; multstacks=number;
    optimizer=spsa; smtSystem=phrase-based-moses; mert="";  spsa_a=25; memscore=""; #memscore="--memscore";mert="--no-mert";
    mosesSB="-s 100 -b 0.5";
}
########################################################################################################################
########################################################################################################################
# The options for training, tuning and testing are the ones used to produce the results table in the PBML paper

bdir=`pwd`
BIA_ROOTDIR=/home/lambert/softmt/source/bia-aligner
MOSES_ROOTDIR=/home/lambert/soft/moses-2010-08-13
MOSES_CMD=$MOSES_ROOTDIR/moses-cmd/src/moses
tmpdir=$bdir/tmp    # directory for temporary files
jobs=8              # number of jobs for BIA (corpus is divided by the number of jobs) and number of threads for Moses
if [ "$jobs" = "" ]; then
    jobs=1;
fi

src=es
trg=en
mertDev=dev05                       # development set used for Moses MERT at each alignment optimisation iteration
decodeDev=dev                       # development set used to evaluate Moses output BLEU score at each alignment optimisation iteration
st="$src-$trg"
wdir=$bdir                          # working directory
cdir=$wdir/corpus; mkdir -p $cdir   # training corpus directory
refdir=$cdir                        # dev/test corpus directoy
biadir=$wdir/bia; mkdir -p $biadir

# TRAIN BIA MODELS
discount=1                          # to calculate BIA relative link frequencies, we ignore links occurring only once
pruningScore="1+2+3"                # we prune BIA model according to the sum of first 3 scores: p(link|s,t)+p(link|s)+p(link|t)
train

# TUNING ALIGNMENT ACCORDING TO BLEU SCORE
tdir=$wdir/tune-bia; mkdir -p $tdir
reset; freepars=( wwda1 wwda2 wwda3 wlb wf  wcn wcl whp ); freevals=( 0.5   0.1   0.1   1 0.5 0.5 0.3 0.1 ); 
tune; 

# ALIGNING WITH OPTIMUM WEIGHTS
evaluate
