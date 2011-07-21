#include "ClientSocket.h"
#include "SocketException.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

int read_params(int argc, const char **argv);
void error_params(string);

string request,reply;
vector<string> b;
string sInputType, tInputType,nLevels,t,nms,n,d,f,hr,m,c,ct,init,first,phLinkWeight,phas,exp,direction;
bool accu,oneToOne,backTrack;
string multipleStacks,doba,avgeOnlyBestAsso;
vector<string> wlb,wpp,wup,wus,wut,wcl,wcn,whp,whs,wht,wzp;
vector<string> wwda1;
string wtga,wwda2,wwda3,wwda4,wwda5,wwda6,wrk,wrks,wrkt,wrkb,wrkw,wum,wums,wumt,wf,wfs,wft,wchfilt,wchfilts,wchfiltt;
string w1to1,w1toN,wNto1,w1toNsum,wNtoM;
string wmatchb,wstemb,wsynb,cmatch,cstem,csyn,meshLinkCost;
vector<string> isfile,itfile;
string ofile,ostfile,otsfile,itgsfile,itgtfile,nbestFile;
int nbestSize;
string verbose,range,outputType;
int PORT;

int main ( int argc, const char ** argv ){
  PORT=read_params(argc,argv);

  if (PORT<10000){
    string err("PORT<1000");
    error_params(err);
  }

  try {
    ClientSocket client_socket ( "localhost", PORT );
    try	{
      client_socket << request;
      client_socket >> reply;
    }
    catch ( SocketException& ) {}

    cerr << "Client: " << reply << endl;
  }
  catch ( SocketException& e ){
    cerr << "Client: Exception was caught:" << e.description() << "\n";
  }
  return 0;
}
void error_params(string err){
  cerr << "usage: bia -is inputSourceFile -it inputTargetFile -iwda chi-squareTableFile  [ optional ]" << endl;
  cerr << "   -is    f : input source (sentences) file" << endl;
  cerr << "   -it    f : corresponding target file" << endl;
  cerr << "   -o   f : output file for src-to-trg alignments       [def stdout]" << endl;
  cerr << "   -n-best-list   f n: writes nbest-list of size n (so far only 1-best implemented) to file indicated" << endl;
  cerr << "   -ost   f : output file for alignments (search along source side)      [def '-': not used]" << endl;
  cerr << "   -ots   f : output file for alignments (search along target side)      [def '-': not used]" << endl;
  cerr << "   -itgs  f : input source tags file" << endl;
  cerr << "   -itgt  f : corresponding target file" << endl;
  cerr << "   -isu  f : source unlinked model table file" << endl;  
  cerr << "   -itu  f : target unlinked model table file" << endl;  
  cerr << "   -iwda  f : word association scores table file" << endl;  
  cerr << "   -imss  f : match, stem, synonym table file" << endl;  
  cerr << "   -itga  f : tag association scores table file" << endl;  
  cerr << "   -lev     n : number of levels in multi-level search                [def 1: only words]" << endl<<endl;
  cerr << "   -sinputtype n: source input type: text (0) or confusion network (1) [default 0]" << endl<<endl;
  cerr << "   -tinputtype n: target input type: text (0) or confusion network (1) [default 0]" << endl<<endl;

  cerr << "   -b[i]     n : hypothesis histogram pruning for decoding iteration i       [def 50]" << endl;
  cerr << "   -t     f : hypothesis threshold pruning                [def 0:NOT USED]" << endl;
  cerr << "   -nms     n : maximum number of substacks when using multiple stacks      [def 0: all]" << endl;
  cerr << "   -n     n : number of possible links for each word      [def 5]" << endl;
  cerr << "   -d     n : maximum iterations in decoding loop (trying to change all links from initial alignment)      [def 1]" << endl;
  cerr << "   -f     n : number of forward words to estimate future costs      [def 0: not used]" << endl;
  cerr << "   -hr     n : number of last decision identical for hypothesis recombination      [def 0: not used]" << endl;
  cerr << "   -m     n : maximum length for link combinations in the selection of links to be expanded (for eexp=word)       [def 1]" << endl;
  cerr << "   -c     n : maximum number of link combinations to be expanded (in each stack)     [def 0: not used]" << endl;
  cerr << "   -ct     f : link combinations to be expanded threshold pruning     [def 0: not used]" << endl;
  cerr << "   -init     empty|diag|full : initial alignment in first decoding iteration [def 'empty']" << endl;
  cerr << "   -first    avge|none : links between words involving less links than average are inserted first [def 'none']" << endl;
  cerr << "   -phlink-count    Weighting scheme for phrase-links (number of links considered in link bonus and zigzap penalty): [def 'weighted']" << endl;
  cerr << "                 one: one link is counted" << endl;
  cerr << "                 weighted: number of links considered is ns*nc*weight, weight=0.5(1/ns+1/nt), where ns, nt are number of source and target words" << endl;
  cerr << "                 word-links: number of links considered is ns*nc" << endl;
  cerr << "   -one-to-one   i: Only one-to-one (and one-to-NULL) alignments are allowed (one-to-many and many-to-many are not allowed) [def 0: no restriction]" << endl;
  cerr << "   -backtrack   i: Hypothesis links not compatible with the newly inserted link are first removed from the hypothesis [def 0: not removed]" << endl;
  cerr << "   -phas    possl|imsum|imbest|imworst : method to calculate phrase association score [def 'possl']" << endl;
  cerr << "                 possl: average lexical cost of the selected n-1-level links in n-level link"<<endl;
  cerr << "                 imsum: sum of src-trg ant trg-src ibm1 scores (with maximum approx:1/J* prod_j max_i p(ij) )"<<endl;
  cerr << "                 imbest: best of src-trg ant trg-src ibm1 scores (with maximum approx:1/J* prod_j max_i p(ij) )"<<endl;
  cerr << "                 imworst: worst of src-trg ant trg-src ibm1 scores (with maximum approx:1/J* prod_j max_i p(ij) )"<<endl;
  cerr << "   -dir     a|as|at|st|ts|stts|stIts|asIat[-X]: search direction: order in which links are added to hypothesis during search [def 'stts']" << endl;
  cerr << "            X-Y[-...]: direction X best alignment is initial alignment for direction Y parse"<<endl;
  cerr << "                 a: according to association score only, independently of position"<<endl;
  cerr << "                 as: reordering source sentence according to association score (exp=word)"<<endl;
  cerr << "                 at: reordering target sentence according to association score (exp=word)"<<endl;
  cerr << "                 st: parsing along the source sentence"<<endl;
  cerr << "                 ts: parsing along the target sentence"<<endl;  
  cerr << "                 stts: parsing along source sentence, then along target sentence, then merging both N-best lists"<<endl;
  cerr << "                 stIts: intersection of 'st' and 'ts' parse "<<endl;
  cerr << "                 asIat: intersection of 'as' and 'at' parse"<<endl;
  cerr << "   -multiple-stacks    none|number|coverage|target-number: uses multiple stacks during decoding to expand states ([def none])" << endl;
  cerr << "                 none: uses one stack for each entry in the link expanding list (one entry is one word if exp=word)"<<endl;  
  cerr << "                 number: uses one stack for each different number of words (source+target) in the hypothesis"<<endl;
  cerr << "                 coverage: uses one stack for each different word coverage in the hypothesis"<<endl;
  cerr << "                 target-number: uses one stack for each different number of target words in the hypothesis"<<endl;
  cerr << "   -accu     b: n-best alignments of all searches are accumulated in final n-best list ([def not used])" << endl;
  cerr << "   -doba     n: in all but last search iterations, only best asso score is considered ([def 0: not used])" << endl;
  cerr << "   -aoba     n: for words involving more links than average, only best asso score is considered ([def 0: not used])" << endl;
  cerr << endl;
  cerr << "   -wwdaN    w : weight for the Nth word association model [def wwa1:1]" << endl;
  cerr << "                                                           [def wwaN,N>1:0]" << endl;
  cerr << "   -wtga    w : weight for TAG association model [def 0: NOT USED]" << endl;
  cerr << "   -wrks     w : weight for source association rank model (rank of target given source) [def 0: NOT USED]" << endl;
  cerr << "   -wrkt     w : weight for target association rank model (rank of source given target) [def 0: NOT USED]" << endl;
  cerr << "   -wrk     w : weight for association rank sum model (rks+rkt) [def 0: NOT USED]" << endl;
  cerr << "   -wrkb     w : weight for best association rank model (min(rks,rkt)) [def 0: NOT USED]" << endl;
  cerr << "   -wrkw     w : weight for worst association rank model (max(rks,rkt)) [def 0: NOT USED]" << endl;
  cerr << "   -wum    w : weight for the unlinked model [def 0]" << endl;
  cerr << "   -wums    w : weight for the unlinked SOURCE model [def 0]" << endl;
  cerr << "   -wumt    w : weight for the unlinked TARGET model [def 0]" << endl;
  cerr << "   -wf     w : weight for fertility models [def 0: NOT USED]" << endl;
  cerr << "   -wfs     w : weight for source fertility model [def 0: NOT USED]" << endl;
  cerr << "   -wft    w : weight for target fertility model [def 0: NOT USED]" << endl;
  cerr << "   -wchfilt    w : weight for the chunk-based Intersection-guided filtering feature [def 0:NOT USED]" << endl;
  cerr << "   -wchfilts    w : weight for the source-chunks based Intersection-guided filtering feature [def 0:NOT USED]" << endl;
  cerr << "   -wchfiltt    w : weight for the target-chunks based Intersection-guided filtering feature [def 0:NOT USED]" << endl;
  cerr << "   -wlb    w : weight for the link bonus [def 0:NOT USED]" << endl;
  cerr << "   -wpp    w : weight for the phrase-link penalty (pp=wpp*#source_words*#target_words in phrase-based links) [def 0:NOT USED]" << endl;
  cerr << "   -wup    w : weight for the unlinked word penalty [def 0]" << endl;
  cerr << "   -wus    w : weight for the unlinked SOURCE word penalty [def 0]" << endl;
  cerr << "   -wut    w : weight for the unlinked TARGET word penalty [def 0]" << endl;
  cerr << "   -wcn    w : weight for the crossing number [def 0:NOT USED]" << endl;
  cerr << "   -wcl    w : weight for the accumulated crossing lengthlink bonus [def 0:NOT USED]" << endl;
  cerr << "   -whp    w : weight for the hole penalty [def 0:NOT USED]" << endl;
  cerr << "   -whs    w : weight for the SOURCE hole penalty [def 0:NOT USED]" << endl;
  cerr << "   -wht    w : weight for the TARGET hole penalty [def 0:NOT USED]" << endl;
  cerr << "   -wzp    w : weight for the zigzag penalty [def 0:NOT USED]" << endl;
  cerr << "   -w1to1  w : weight for the number of 1-to-1 links [def 0:NOT USED]" << endl;
  cerr << "   -w1toN  w : weight for the number of 1-to-many links (one source word connecting several target words) [def 0:NOT USED]" << endl;
  cerr << "   -wNto1  w : weight for the number of many-to-1 links (several source words connecting one target word) [def 0:NOT USED]" << endl;
  cerr << "   -w1toNsum  w : weight for the sum of 1-to-many and many-to-1 links [def 0:NOT USED]" << endl;
  cerr << "   -wNtoM  w : weight for the number of many-to-many links [def 0:NOT USED]" << endl;
  cerr << "   -wmatchb  w : weight for the match bonus [def 0: NOT USED]" << endl;
  cerr << "   -wstemb  w : weight for the stem match bonus [def 0: NOT USED]" << endl;
  cerr << "   -wsynb  w : weight for the synonym match bonus [def 0: NOT USED]" << endl;
  cerr << "   -cmatch  w : cost of a match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -cstem  w : cost of a stem match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -csyn  w :  cost of a synonym match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -meshlc best|interp : each of the costs (association, rank, match, etc.) of a link between two meshes is calculated as: [def best]" << endl;
  cerr << "                  best: the best cost between the mesh words" << endl;
  cerr << "                  interp: the interpolation of the costs of the mesh words (weighted by the normalised probability of the words in the mesh)" << endl;

  cerr << "   -port    i : port to place BIA server     [def 0:NOT server]" << endl;
  cerr << "   -outputtype     'word-links' (word-to-word links, ex 1-1), 'phrase-links' (phrase-to-phrase links, ex 1,2-1), or 'clusters' [def word-links]" << endl;
  cerr << "    (Cluster: minimal set of words such that all source (target) words are linked only to the target (source) words in the same set.)" << endl;
  cerr << "   -range     begin-end: limits range between sentence pair numbers 'begin' and 'end' [def '-' (not used)]" << endl;
  cerr << "   -verbose     l : verbosity level (0|1|2)                        [def 0]" << endl;
  cerr << "                  l=0: (no additional information)" << endl;
  cerr << "                  l>0: verbosity in outputfile.VERBOSE" << endl;
  cerr << endl;
  cerr << "   [to kill a server, use:] bia-[version].client -die port" << endl;

  //cerr << endl << "   -h         : a detailed help" << endl;
  //cerr << "   -warranty         : warranty and conditions" << endl;
  if (err.size()>0) cerr << endl << "ERROR " << err << " while parsing BIA parameters !!" << endl;
  exit(0);
}
int read_params(int argc, const char **argv){
  //servermode=false;

  if (argc==1) error_params("");
  PORT=0;
  verbose="0";
  isfile.assign(10,"-");isfile[0]="-";
  itfile.assign(10,"-");itfile[0]="-";
  ofile="-";ostfile="-";otsfile="-";itgsfile="-";itgtfile="-";
  sInputType="1"; tInputType="1";
  nLevels="1";
  outputType="word-links"; nbestFile="-"; nbestSize=1; range="1-";
  b.assign(10,"50");
  t="0";nms="0";n="5";d="1";f="0";hr="0";m="3";c="0";ct="0";init="empty";first="none";phLinkWeight="weighted"; oneToOne=false; backTrack=false; phas="possl";exp="word";direction="as";multipleStacks="target-number";
  accu=false;
  doba="0";avgeOnlyBestAsso="0";
  wlb.assign(10,"-");wpp.assign(10,"-");wcn.assign(10,"-");wcl.assign(10,"-");
  wup.assign(10,"-"); wus.assign(10,"-");wut.assign(10,"-"); 
  whp.assign(10,"-");whs.assign(10,"-");wht.assign(10,"-");wzp.assign(10,"-");
  wwda1.assign(10,"-");
  wwda2="0";wwda3="0";wwda4="0";wwda5="0";wwda6="0";wtga="0";wrk="0";wrks="0";wrkt="0";wrkb="0";wrkw="0";wum="0";wums="0";wumt="0"; wf="0";wfs="0";wft="0";wchfilt="0";wchfilts="0";wchfiltt="0";
  w1to1="0";w1toN="0";wNto1="0";w1toNsum="0";wNtoM="0";
  wmatchb="0";wstemb="0";wsynb="0"; cmatch="-1"; cstem="-1"; csyn="-1"; meshLinkCost="best";
  string list("");
  for (int i=1;i<argc;i++){
   list.append(argv[i]).append(" ");
  }
  istringstream inStr(list);

  while (inStr){
    string wrd;
    inStr >> wrd;

    if (!inStr) continue;

    /*   
    if (wrd.compare("-h")==0){
      detailed_help();
      continue;
      }

    if (wrd.compare("-warranty")==0){
      warranty_conditions();
      continue;
      }
    */

    if (wrd.compare("-die")==0){
      inStr >> wrd;
      PORT=atoi(wrd.data());
      request="die";
      return PORT;
    }

    if (wrd.compare("-itgs")==0){
      inStr >> wrd;
      itgsfile=wrd;
      continue;
    }

    if (wrd.compare("-itgt")==0){
      inStr >> wrd;
      itgtfile=wrd;
      continue;
    }

    // here a parameter beginning with -is or -it can ONLY be -isn or -itn with n an integer or ""
    if (wrd.substr(0,3)=="-is"){
      int index;
      if (wrd.substr(3).compare("")==0) index=0;
      else index=atoi(wrd.substr(3).c_str());
      inStr >> wrd;
      isfile[index]=wrd;
      continue;
    }
    if (wrd.substr(0,3)=="-it"){
      int index;
      if (wrd.substr(3).compare("")==0) index=0;
      else index=atoi(wrd.substr(3).c_str());
      inStr >> wrd;
      itfile[index]=wrd;
      continue;
    }

    if (wrd.compare("-lev")==0){
      inStr >> wrd;
      nLevels=wrd;
      continue;
    }
    if (wrd.compare("-sinputtype")==0){
      inStr >> wrd;
      sInputType=wrd;
      continue;
    }
    if (wrd.compare("-tinputtype")==0){
      inStr >> wrd;
      tInputType=wrd;
      continue;
    }
    if (wrd.substr(0,2)=="-b"){
      int index;
      if (wrd.substr(2).compare("")==0) index=0;
      else index=atoi(wrd.substr(2).c_str());
      inStr >> wrd;
      b[index]=wrd;
      continue;
    }

    if (wrd.compare("-t")==0){
      inStr >> wrd;
      t=wrd;
      continue;
    }
    if (wrd.compare("-nms")==0){
      inStr >> wrd;
      nms=wrd;
      continue;
    }
    if (wrd.compare("-n")==0){
      inStr >> wrd;
      n=wrd;
      continue;
    }
    if (wrd.compare("-d")==0){
      inStr >> wrd;
      d=wrd;
      continue;
    }
    if (wrd.compare("-f")==0){
      inStr >> wrd;
      f=wrd;
      continue;
    }
    if (wrd.compare("-hr")==0){
      inStr >> wrd;
      hr=wrd;
      continue;
    }
    if (wrd.compare("-m")==0){
      inStr >> wrd;
      m=wrd;
      continue;
    }
    if (wrd.compare("-c")==0){
      inStr >> wrd;
      c=wrd;
      continue;
    }
    if (wrd.compare("-ct")==0){
      inStr >> wrd;
      ct=wrd;
      continue;
    }
    if (wrd.compare("-init")==0){
      inStr >> wrd;
      init=wrd;
      continue;
    }
    if (wrd.compare("-first")==0){
      inStr >> wrd;
      first=wrd;
      continue;
    }
    if (wrd.compare("-phlink-count")==0){
      inStr >> wrd;
      phLinkWeight=wrd;
      continue;
    }
    if (wrd.compare("-one-to-one")==0){
      inStr >> wrd;
      if (wrd != "0") oneToOne=true;
      continue;
    }
    if (wrd.compare("-backtrack")==0){
      inStr >> wrd;
      if (wrd != "0") backTrack=true;
      continue;
    }
    if (wrd.compare("-phas")==0){
      inStr >> wrd;
      phas=wrd;
      continue;
    }
    if (wrd.compare("-exp")==0){
      inStr >> wrd;
      exp=wrd;
      continue;
    }
    if (wrd.compare("-dir")==0){
      inStr >> wrd;
      direction=wrd;
      continue;
    }
    if (wrd.compare("-multiple-stacks")==0){
      inStr >> wrd;
      multipleStacks=wrd;
      continue;
    }
    if (wrd.compare("-accu")==0){
      accu=true;
      continue;
    }
    if (wrd.compare("-doba")==0){
      inStr >> wrd;
      doba=wrd;
      continue;
    }
    if (wrd.compare("-aoba")==0){
      inStr >> wrd;
      avgeOnlyBestAsso=wrd;
      continue;
    }

    if (wrd.substr(0,6)=="-wwda1"){
      int index;
      if (wrd.substr(6).compare("")==0) index=0;
      else index=atoi(wrd.substr(6).c_str());
      inStr >> wrd;
      wwda1[index]=wrd;
      continue;
    }
    if (wrd.compare("-wwda2")==0){
      inStr >> wrd;
      wwda2=wrd;
      continue;
    }
    if (wrd.compare("-wwda3")==0){
      inStr >> wrd;
      wwda3=wrd;
      continue;
    }
    if (wrd.compare("-wwda4")==0){
      inStr >> wrd;
      wwda4=wrd;
      continue;
    }
    if (wrd.compare("-wwda5")==0){
      inStr >> wrd;
      wwda5=wrd;
      continue;
    }
    if (wrd.compare("-wwda6")==0){
      inStr >> wrd;
      wwda6=wrd;
      continue;
    }
    if (wrd.compare("-wtga")==0){
      inStr >> wrd;
      wtga=wrd;
      continue;
    }
    if (wrd.compare("-wrk")==0){
      inStr >> wrd;
      wrk=wrd;
      continue;
    }
    if (wrd.compare("-wrks")==0){
      inStr >> wrd;
      wrks=wrd;
      continue;
    }
    if (wrd.compare("-wrkt")==0){
      inStr >> wrd;
      wrkt=wrd;
      continue;
    }
    if (wrd.compare("-wrkb")==0){
      inStr >> wrd;
      wrkb=wrd;
      continue;
    }
    if (wrd.compare("-wrkw")==0){
      inStr >> wrd;
      wrkw=wrd;
      continue;
    }
    if (wrd.compare("-wum")==0){
      inStr >> wrd;
      wum=wrd;
      continue;
    }
    if (wrd.compare("-wums")==0){
      inStr >> wrd;
      wums=wrd;
      continue;
    }
    if (wrd.compare("-wumt")==0){
      inStr >> wrd;
      wumt=wrd;
      continue;
    }
    if (wrd.compare("-wf")==0){
      inStr >> wrd;
      wf=wrd;
      continue;
    }
    if (wrd.compare("-wfs")==0){
      inStr >> wrd;
      wfs=wrd;
      continue;
    }
    if (wrd.compare("-wft")==0){
      inStr >> wrd;
      wft=wrd;
      continue;
    }
    if (wrd.compare("-wchfilt")==0){
      inStr >> wrd;
      wchfilt=wrd;
      continue;
    }
    if (wrd.compare("-wchfilts")==0){
      inStr >> wrd;
      wchfilts=wrd;
      continue;
    }
    if (wrd.compare("-wchfiltt")==0){
      inStr >> wrd;
      wchfiltt=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wlb"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wlb[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wpp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wpp[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wup"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wup[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wus"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wus[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wut"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wut[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wcn"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wcn[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wcl"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wcl[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-whp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      whp[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-whs"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      whs[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wht"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wht[index]=wrd;
      continue;
    }
    if (wrd.substr(0,4)=="-wpzp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wzp[index]=wrd;
      continue;
    }
    if (wrd.compare("-w1to1")==0){
      inStr >> wrd;
      w1to1=wrd;
      continue;
    }
    if (wrd.compare("-w1toN")==0){
      inStr >> wrd;
      w1toN=wrd;
      continue;
    }
    if (wrd.compare("-wNto1")==0){
      inStr >> wrd;
      wNto1=wrd;
      continue;
    }
    if (wrd.compare("-w1toNsum")==0){
      inStr >> wrd;
      w1toNsum=wrd;
      continue;
    }
    if (wrd.compare("-wNtoM")==0){
      inStr >> wrd;
      wNtoM=wrd;
      continue;
    }
    if (wrd.compare("-wmatchb")==0){
      inStr >> wrd;
      wmatchb=wrd;
      continue;
    }
    if (wrd.compare("-wstemb")==0){
      inStr >> wrd;
      wstemb=wrd;
      continue;
    }
    if (wrd.compare("-wsynb")==0){
      inStr >> wrd;
      wsynb=wrd;
      continue;
    }
    if (wrd.compare("-cmatch")==0){
      inStr >> wrd;
      cmatch=wrd;
      continue;
    }
    if (wrd.compare("-cstem")==0){
      inStr >> wrd;
      cstem=wrd;
      continue;
    }
    if (wrd.compare("-csyn")==0){
      inStr >> wrd;
      csyn=wrd;
      continue;
    }
    if (wrd.compare("-meshlc")==0){
      inStr >> wrd;
      meshLinkCost=wrd;
      continue;
    }
    if (wrd.compare("-o")==0){
      inStr >> wrd;
      ofile=wrd;
      continue;
    }
    if (wrd.compare("-n-best-list")==0){
      inStr >> wrd;
      nbestFile=wrd;
      inStr >> wrd;
      nbestSize=atoi(wrd.data());
      if (nbestSize!=1) error_params("N-best lists are not implemented yet. This switch only allows to output the best hypothesis scores.");
      continue;
    }
    if (wrd.compare("-ost")==0){
      inStr >> wrd;
      ostfile=wrd;
      continue;
    }
    if (wrd.compare("-ots")==0){
      inStr >> wrd;
      otsfile=wrd;
      continue;
    }

    if (wrd.compare("-range")==0){
      inStr >> wrd;
      range=wrd;
      string::size_type loc = range.find( "-", 0 );
      if (loc == string::npos){
	error_params("range must have the format 'begin-end' were 'begin' and 'end' can be either integer of empty string");
      }
      continue;
    }
    if (wrd.compare("-verbose")==0){
      inStr >> wrd;
      verbose=wrd;
      continue;
    }

    if (wrd.compare("-port")==0){
      inStr >> wrd;
      PORT=atoi(wrd.data());
      //servermode=true;
      continue;
    }
    if (wrd.compare("-outputtype")==0){
      inStr >> wrd;
      outputType=wrd;
      continue;
    }

    string err("param ");
    err.append(wrd).append (" UNKNOWN,");
    error_params(err);
  }
  request="";
  
  for (vector<string>::size_type cnt=0; cnt<isfile.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (isfile[cnt].compare("-")!=0) request.append(" -is").append(oss.str()).append(" ").append(isfile[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<itfile.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (itfile[cnt].compare("-")!=0) request.append(" -it").append(oss.str()).append(" ").append(itfile[cnt]);
  }
  if (ofile.compare("-")!=0) request.append(" -o ").append(ofile);
  if (ostfile.compare("-")!=0) request.append(" -ost ").append(ostfile);
  if (otsfile.compare("-")!=0) request.append(" -ots ").append(otsfile);
  if (itgsfile.compare("-")!=0) request.append(" -itgs ").append(itgsfile);
  if (itgtfile.compare("-")!=0) request.append(" -itgt ").append(itgtfile);
  request.append(" -lev ").append(nLevels);
  request.append(" -sinputtype ").append(sInputType);
  request.append(" -tinputtype ").append(tInputType);
  for (vector<string>::size_type cnt=0; cnt<atoi(d.c_str());++cnt){
    ostringstream oss("");oss<<cnt;    
    request.append(" -b").append(oss.str()).append(" ").append(b[cnt]);
  }
  request.append(" -t ").append(t);
  request.append(" -nms ").append(t);
  request.append(" -n ").append(n);
  request.append(" -d ").append(d);
  request.append(" -f ").append(f);
  request.append(" -hr ").append(hr);
  request.append(" -m ").append(m);
  request.append(" -c ").append(c);
  request.append(" -ct ").append(ct);
  request.append(" -init ").append(init);
  request.append(" -first ").append(first);
  request.append(" -phlink-count ").append(phLinkWeight);
  if (oneToOne==true) request.append(" -one-to-one 1 ");
  if (backTrack==true) request.append(" -backtrack 1 ");
  request.append(" -phas ").append(phas);
  request.append(" -exp ").append(exp);
  request.append(" -dir ").append(direction);
  if (accu==true)  request.append(" -accu ");
  request.append(" -multiple-stacks ").append(multipleStacks);
  request.append(" -doba ").append(doba);
  request.append(" -aoba ").append(avgeOnlyBestAsso);
  for (vector<string>::size_type cnt=0; cnt<wwda1.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wwda1[cnt].compare("-")!=0) request.append(" -wwda1").append(oss.str()).append(" ").append(wwda1[cnt]);
  }
  request.append(" -wwda2 ").append(wwda2);
  request.append(" -wwda3 ").append(wwda3);
  request.append(" -wwda4 ").append(wwda4);
  request.append(" -wwda5 ").append(wwda5);
  request.append(" -wwda6 ").append(wwda6);
  request.append(" -wtga ").append(wtga);
  request.append(" -wrk ").append(wrk);
  request.append(" -wrks ").append(wrks);
  request.append(" -wrkt ").append(wrkt);
  request.append(" -wrkb ").append(wrkb);
  request.append(" -wrkw ").append(wrkw);
  request.append(" -wum ").append(wum);
  request.append(" -wums ").append(wums);
  request.append(" -wumt ").append(wumt);
  request.append(" -wf ").append(wf);
  request.append(" -wfs ").append(wfs);
  request.append(" -wft ").append(wft);
  request.append(" -wchfilt ").append(wchfilt);
  request.append(" -wchfilts ").append(wchfilts);
  request.append(" -wchfiltt ").append(wchfiltt);
  for (vector<string>::size_type cnt=0; cnt<wlb.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wlb[cnt].compare("-")!=0) request.append(" -wlb").append(oss.str()).append(" ").append(wlb[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wpp.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wpp[cnt].compare("-")!=0) request.append(" -wpp").append(oss.str()).append(" ").append(wpp[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wup.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wup[cnt].compare("-")!=0) request.append(" -wup").append(oss.str()).append(" ").append(wup[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wus.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wus[cnt].compare("-")!=0) request.append(" -wus").append(oss.str()).append(" ").append(wus[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wut.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wut[cnt].compare("-")!=0) request.append(" -wut").append(oss.str()).append(" ").append(wut[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wcn.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wcn[cnt].compare("-")!=0) request.append(" -wcn").append(oss.str()).append(" ").append(wcn[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wcl.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wcl[cnt].compare("-")!=0) request.append(" -wcl").append(oss.str()).append(" ").append(wcl[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<whp.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (whp[cnt].compare("-")!=0) request.append(" -whp").append(oss.str()).append(" ").append(whp[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<whs.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (whs[cnt].compare("-")!=0) request.append(" -whs").append(oss.str()).append(" ").append(whs[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wht.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wht[cnt].compare("-")!=0) request.append(" -wht").append(oss.str()).append(" ").append(wht[cnt]);
  }
  for (vector<string>::size_type cnt=0; cnt<wzp.size();++cnt){
    ostringstream oss("");oss<<cnt;    
    if (wzp[cnt].compare("-")!=0) request.append(" -wzp").append(oss.str()).append(" ").append(wzp[cnt]);
  }
  request.append(" -w1to1 ").append(w1to1);
  request.append(" -w1toN ").append(w1toN);
  request.append(" -wNto1 ").append(wNto1);
  request.append(" -w1toNsum ").append(w1toNsum);
  request.append(" -wNtoM ").append(wNtoM);
  request.append(" -wmatchb ").append(wmatchb);
  request.append(" -wstemb ").append(wstemb);
  request.append(" -wsynb ").append(wsynb);
  request.append(" -cmatch ").append(cmatch);
  request.append(" -cstem ").append(cstem);
  request.append(" -csyn ").append(csyn);
  request.append(" -meshlc ").append(meshLinkCost);
  request.append(" -outputtype ").append(outputType);
  request.append(" -n-best-list ").append(nbestFile).append(" ").append(nbestSize);
  request.append(" -range ").append(range);
  request.append(" -verbose ").append(verbose);

  return PORT;
 
}
