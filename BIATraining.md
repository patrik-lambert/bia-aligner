# BIA Training #

If you have a word alignment file available, you may run the following command. The alignment file format is as in moses: links are represented by the source and target word positions (beginning at 0) separated by a dash, and links are separated by a whitespace.
```
$BIA_ROOTDIR/training/train-models.pl \
    --corpus $cdir/train --f es --e en \
    --alignment-file your/alignment/file
```

If you just have the parallel corpus, the script will invoke moses scripts to calculate the base alignment file
```
$BIA_ROOTDIR/training/train-models.pl \
    --corpus $cdir/train --f es --e en \
    --train-moses-cmd /home/lambert/soft/moses-2010-08-13/scripts/training/train-model.perl
```

Note that in the current version of the software, gzipped files are not managed.

# Tuning #

You may use the "tune-model-weights.pl" script as shown below. Note that this script reads the name of the free parameters in the BIA configuration file (--config switch), so make sure you have them all in your configuration file. The results appear in $wdir/tuning/simplex.out (for now only the downhill-simplex optimization is available), and the standard error in tuning/simplex.err. Note also that the initial simplex only appears after N+1 iterations are done, where N is the number of free parameters.
## Without MERT ##
At each tuning iteration, the corpus is aligned by BIA with the current free parameters, SMT models are extracted and some development set is translated with moses default weights or with the weights indicated by the "--smt-system-config" switch. The Bleu score for this evaluation is given to the optimizer.
```
$BIA_ROOTDIR/tuning/tune-model-weights.pl --config=$wdir/bia.cfg --input=$cdir/dev.$f --refs=$cdir/dev.$e. \
	--jobs=8 --root-dir=$wdir \
	--no-mert --smt-system-config=$wdir/model/moses.ini \
	--moses-script-dir=/home/lambert/soft/moses-2010-08-13/scripts --tmpdir=/tmp \
	--train-moses-options="-lm 0:4:$wdir/model/lm.$e:0 " \
	--moses-cmd="/home/lambert/soft/moses-2010-08-13/moses-cmd/src/moses -s 100 -b 0.5 -ttl 20 "
```
## With internal MERT ##
At each tuning iteration, the corpus is aligned by BIA with the current free parameters, SMT models are extracted, MERT is performed and some development set is translated. The Bleu score for this evaluation is given to the optimizer.
```
$BIA_ROOTDIR/tuning/tune-model-weights.pl --config=$wdir/bia.cfg --input=$cdir/dev.$f --refs=$cdir/dev.$e. \
	--jobs=8 --root-dir=$wdir \
	--mert-input=$cdir/dev2.$f --mert-refs=$cdir/dev2.$e \
	--moses-script-dir=/home/lambert/soft/moses-2010-08-13/scripts-flexible --tmpdir=/tmp \
	--train-moses-options="-lm 0:4:$wdir/model/lm.$e:0 " \
	--moses-cmd="(/home/lambert/soft/moses-2010-08-13/moses-cmd/src/moses -s 100 -b 0.5 -ttl 20 "
```

You can create a directory "scripts-flexible" in your moses directory, where you replace the mert-moses-new.pl script by a more flexible version of it. The idea is to go faster and never exceed a fixed amount of iterations, in order to control the time taken by internal MERT at each iteration of the BIA tuning.
You may do the following changes :
```
99c99,104
< my $minimum_required_change_in_weights = 0.00001;
---
> my $minimum_required_change_in_weights = 0.0001;  # increase threshold value for quicker convergence
>     # stop if no lambda changes more than this
> my $max_runs=12;      # stop after 12 iterations; this may have an impact on the result but avoids endless MERT runs
>     # stop if done that many runs
> my $mert_ntry=10;     # number of internal optimisations at each iteration; twice quicker and do not change the result
>     # -n parameter for mert
164a170,172
>   "maxruns=i" => \$max_runs,
>   "mert-ntry=i" => \$mert_ntry,
>   "min-change-in-weights=f" => \$minimum_required_change_in_weights,
215a224,226
>   --maxruns=N ... max number of runs that will be done [default 100]
>   --min-change-in-weights=N ... stops if no lambda is changed more that this value [default 0.00001]
>   --mert-ntry=N ... n parameter of mert [default 20]
607c618
< while(1) {
---
> while($run < $max_runs) {
702c713
<   $cmd = "$mert_mert_cmd -d $DIM $mert_mert_args -n 20";
---
>   $cmd = "$mert_mert_cmd -d $DIM $mert_mert_args -n $mert_ntry";
```