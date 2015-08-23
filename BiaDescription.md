# BIA: Bilingual Alignment at the Phrase level #

BIA is a discriminative alignment system, based on log-linear alignment models.

It uses a lexical model (based on relative link probabilities) and source and target fertility models trained on previously existing word alignments.

The '''lexical model''' contains the following information:

```
source phrase ||| target phrase ||| c(link|s,t) c(link|s) c(link|t) c(odds)
```

where
  * "link" refers to the link between source phrase s and target phrase t
  * the cost c=-log<sub>10</sub>(p) and the probabilities p are the following
  * p(link|s,t)= (occurrences(link)-discount)/co-occurrences(s,t)
  * p(link|s)=(occurrences(link)-discount)/occurrences(s)
  * p(link|t)=(occurrences(link)-discount)/occurrences(t)
  * p(odds)=(occurrences(link)+1.0)/(co-occurrences(s,t)-occurrences(link)+1.0);
  * co-occurrences(s,t)=min(occurrences(s),occurrences(t))

Note that to ensure that the relative probabilities are between 0 and 1, the number of links taken into account depends on the context. This is easier to understand with the following example. If a word w1 appears once, a word w2 appears twice and w1 is linked twice to w2, we have occurrences(w1)=1, occurrences(w2)=2, co-occurrences(w1,w2)=1. So if we count the 2 links from w1, p(link|w1,w2)=2 and p(link|w1)=2. To avoid this we keep different link counts in the relative link probabilities calculations for p(link|s,t), p(link|s) and p(link|t). The number of links taken into account is never more than the corresponding number of occurrences or co-occurrences. However, this does not affect the fertility calculation, for which the number of links is always the real one.

The '''fertility models''' contain the following information:

```
word c(0) c(1) c(2) c(3) c(>3) 
```

where c(i)=-log<sub>10</sub>(p(i)) and p(i) is the probability that the word is linked to i words

**References**:

Patrik Lambert, Rafael Banchs and Josep M. Crego. [Discriminative Alignment Training without Annotated Data for Machine Translation](http://aclweb.org/anthology-new/N/N07/N07-2022.pdf). In Proc. of NAACL-HLT, pp 85-88. Rochester, NY, USA, April 23-25.

Patrik Lambert and Rafael E. Banchs. Word association models and search strategies for discriminative word alignment. In Proc. of EAMT, pp 97--103. Hamburg, Germany.

## Running BIA ##

Before running BIA on a test set, you may want to filter the model given this test set:
```
$BIA_ROOTDIR/training/filter-model-given-input.pl model_file test.s test.t
```
The output is the file model\_file.filtered-test (where test is the prefix of the test file name)

You may run BIA with the following command:
```
    $BIA_ROOTDIR/bia -is bia/train.es -it bia/train.en \
     -isf bia/fertility.es -itf bia/fertility.en \
    -iwda bia/es-en.pha-d0-lnks.prnsum-9-20-200 \
    -n 3 -b 5 -t 5 -multiple-stacks number -m 1 -c 12 -ct 3 -first avge -exp word -dir as -doba 1 -aoba 2 \
    -wwda1 0.5 -wwda2 0.5 -wwda3 0.3 -wlb 1 -wcn 1 -wcl 0.2 -wrk 0.5 -wf 1 -whp 0.5
```
The parameters in the last line are the feature weights. You may tune them with an optimisation algorithm like fmin (downhill simplex), SPSA (simultaneous perturbation stochastic approximation ) or Condor.


[Training and Tuning Instructions](BIATraining.md)