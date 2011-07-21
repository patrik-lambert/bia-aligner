/*
Copyright (C) TALP Research Center - Universitat Politècnica de Catalunya (UPC)
Written by Josep M. Crego
Barcelona, September 2005

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "params.h"

/************************************************************************/
/**** PARAMS ************************************************************/
/************************************************************************/

/*void param::warranty_conditions(){
  cerr << "GNU General Public License\n \
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n \
\n \
0. This License applies to any program or other work which contains a notice placed by the copyright holder saying it may be distributed under the terms of this General Public License. The \"Program\", below, refers to any such program or work, and a \"work based on the Program\" means either the Program or any derivative work under copyright law: that is to say, a work containing the Program or a portion of it, either verbatim or with modifications and/or translated into another language. (Hereinafter, translation is included without limitation in the term \"modification\".) Each licensee is addressed as \"you\".\n \
\n \
Activities other than copying, distribution and modification are not covered by this License; they are outside its scope. The act of running the Program is not restricted, and the output from the Program is covered only if its contents constitute a work based on the Program (independent of having been made by running the Program). Whether that is true depends on what the Program does.\n \
\n \
1. You may copy and distribute verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice and disclaimer of warranty; keep intact all the notices that refer to this License and to the absence of any warranty; and give any other recipients of the Program a copy of this License along with the Program.\n \
\n \
You may charge a fee for the physical act of transferring a copy, and you may at your option offer warranty protection in exchange for a fee.\n \
\n \
2. You may modify your copy or copies of the Program or any portion of it, thus forming a work based on the Program, and copy and distribute such modifications or work under the terms of Section 1 above, provided that you also meet all of these conditions:\n \
\n \
    a) You must cause the modified files to carry prominent notices stating that you changed the files and the date of any change. \n \
\n \
    b) You must cause any work that you distribute or publish, that in whole or in part contains or is derived from the Program or any part thereof, to be licensed as a whole at no charge to all third parties under the terms of this License. \n \
\n \
    c) If the modified program normally reads commands interactively when run, you must cause it, when started running for such interactive use in the most ordinary way, to print or display an announcement including an appropriate copyright notice and a notice that there is no warranty (or else, saying that you provide a warranty) and that users may redistribute the program under these conditions, and telling the user how to view a copy of this License. (Exception: if the Program itself is interactive but does not normally print such an announcement, your work based on the Program is not required to print an announcement.) \n \
\n \
These requirements apply to the modified work as a whole. If identifiable sections of that work are not derived from the Program, and can be reasonably considered independent and separate works in themselves, then this License, and its terms, do not apply to those sections when you distribute them as separate works. But when you distribute the same sections as part of a whole which is a work based on the Program, the distribution of the whole must be on the terms of this License, whose permissions for other licensees extend to the entire whole, and thus to each and every part regardless of who wrote it.\n \
\n \
Thus, it is not the intent of this section to claim rights or contest your rights to work written entirely by you; rather, the intent is to exercise the right to control the distribution of derivative or collective works based on the Program.\n \
\n \
In addition, mere aggregation of another work not based on the Program with the Program (or with a work based on the Program) on a volume of a storage or distribution medium does not bring the other work under the scope of this License.\n \
\n \
3. You may copy and distribute the Program (or a work based on it, under Section 2) in object code or executable form under the terms of Sections 1 and 2 above provided that you also do one of the following:\n \
\n \
    a) Accompany it with the complete corresponding machine-readable source code, which must be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or, \n \
\n \
    b) Accompany it with a written offer, valid for at least three years, to give any third party, for a charge no more than your cost of physically performing source distribution, a complete machine-readable copy of the corresponding source code, to be distributed under the terms of Sections 1 and 2 above on a medium customarily used for software interchange; or, \n \
\n \
    c) Accompany it with the information you received as to the offer to distribute corresponding source code. (This alternative is allowed only for noncommercial distribution and only if you received the program in object code or executable form with such an offer, in accord with Subsection b above.) \n \
\n \
The source code for a work means the preferred form of the work for making modifications to it. For an executable work, complete source code means all the source code for all modules it contains, plus any associated interface definition files, plus the scripts used to control compilation and installation of the executable. However, as a special exception, the source code distributed need not include anything that is normally distributed (in either source or binary form) with the major components (compiler, kernel, and so on) of the operating system on which the executable runs, unless that component itself accompanies the executable.\n \
\n \
If distribution of executable or object code is made by offering access to copy from a designated place, then offering equivalent access to copy the source code from the same place counts as distribution of the source code, even though third parties are not compelled to copy the source along with the object code.\n \
\n \
4. You may not copy, modify, sublicense, or distribute the Program except as expressly provided under this License. Any attempt otherwise to copy, modify, sublicense or distribute the Program is void, and will automatically terminate your rights under this License. However, parties who have received copies, or rights, from you under this License will not have their licenses terminated so long as such parties remain in full compliance.\n \
\n \
5. You are not required to accept this License, since you have not signed it. However, nothing else grants you permission to modify or distribute the Program or its derivative works. These actions are prohibited by law if you do not accept this License. Therefore, by modifying or distributing the Program (or any work based on the Program), you indicate your acceptance of this License to do so, and all its terms and conditions for copying, distributing or modifying the Program or works based on it.\n \
\n \
6. Each time you redistribute the Program (or any work based on the Program), the recipient automatically receives a license from the original licensor to copy, distribute or modify the Program subject to these terms and conditions. You may not impose any further restrictions on the recipients' exercise of the rights granted herein. You are not responsible for enforcing compliance by third parties to this License.\n \
\n \
7. If, as a consequence of a court judgment or allegation of patent infringement or for any other reason (not limited to patent issues), conditions are imposed on you (whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License. If you cannot distribute so as to satisfy simultaneously your obligations under this License and any other pertinent obligations, then as a consequence you may not distribute the Program at all. For example, if a patent license would not permit royalty-free redistribution of the Program by all those who receive copies directly or indirectly through you, then the only way you could satisfy both it and this License would be to refrain entirely from distribution of the Program.\n \
\n \
If any portion of this section is held invalid or unenforceable under any particular circumstance, the balance of the section is intended to apply and the section as a whole is intended to apply in other circumstances.\n \
\n \
It is not the purpose of this section to induce you to infringe any patents or other property right claims or to contest validity of any such claims; this section has the sole purpose of protecting the integrity of the free software distribution system, which is implemented by public license practices. Many people have made generous contributions to the wide range of software distributed through that system in reliance on consistent application of that system; it is up to the author/donor to decide if he or she is willing to distribute software through any other system and a licensee cannot impose that choice.\n \
\n \
This section is intended to make thoroughly clear what is believed to be a consequence of the rest of this License.\n \
\n \
8. If the distribution and/or use of the Program is restricted in certain countries either by patents or by copyrighted interfaces, the original copyright holder who places the Program under this License may add an explicit geographical distribution limitation excluding those countries, so that distribution is permitted only in or among countries not thus excluded. In such case, this License incorporates the limitation as if written in the body of this License.\n \
\n \
9. The Free Software Foundation may publish revised and/or new versions of the General Public License from time to time. Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns.\n \
\n \
Each version is given a distinguishing version number. If the Program specifies a version number of this License which applies to it and \"any later version\", you have the option of following the terms and conditions either of that version or of any later version published by the Free Software Foundation. If the Program does not specify a version number of this License, you may choose any version ever published by the Free Software Foundation.\n \
\n \
10. If you wish to incorporate parts of the Program into other free programs whose distribution conditions are different, write to the author to ask for permission. For software which is copyrighted by the Free Software Foundation, write to the Free Software Foundation; we sometimes make exceptions for this. Our decision will be guided by the two goals of preserving the free status of all derivatives of our free software and of promoting the sharing and reuse of software generally.\n \
\n \
NO WARRANTY\n \
\n \
11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n \
\n \
12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. \n \
" << endl;
  exit(0);
}
*/
//first letter conventions: i=input file o=output file w=weight
//other letters: s=source t=target wd=word tg=tag
void param::error_params(string err) const {
  cerr << "usage: bia -is inputSourceFile -it inputTargetFile -iwda chi-squareTableFile  [ optional ]" << endl;
  cerr << "   -is    f : input source (sentences) file" << endl;
  cerr << "   -it    f : corresponding target file" << endl;
  cerr << "   -o     f : output file for alignments       [def stdout]" << endl;
  cerr << "   -n-best-list   f n: writes nbest-list of size n (so far only 1-best implemented) to file indicated" << endl;
  cerr << "   -ost   f : output file for alignments (search along source side)      [def '-': not used]" << endl;
  cerr << "   -ots   f : output file for alignments (search along target side)      [def '-': not used]" << endl;
  cerr << "   -itgs  f : input source tags file" << endl;
  cerr << "   -itgt  f : corresponding target file" << endl;
  cerr << "   -iwda  f : word association scores table file" << endl;  
  cerr << "   -imss  f : match, stem, synonym table file" << endl;  
  cerr << "   -itga  f : tag association scores table file" << endl;  
  cerr << "   -isu  f : source unlinked model table file" << endl;  
  cerr << "   -itu  f : target unlinked model table file" << endl;  
  cerr << "   -isf  f : source fertility table file" << endl;  
  cerr << "   -itf  f : target fertility table file" << endl;  
  cerr << "   -iscf  f : source class fertility table file" << endl;  
  cerr << "   -itcf  f : target class fertility table file" << endl;  
  cerr << "   -lev     n : number of levels in multi-level search                [def 1: only words]" << endl;
  cerr << "   -sinputtype n: source input type: text (0) or confusion network (1) [default 0]" << endl<<endl;
  cerr << "   -tinputtype n: target input type: text (0) or confusion network (1) [default 0]" << endl<<endl;

  cerr << "   -n     n : number of possible association types for each word      [def 5]" << endl;
  cerr << "   -b[i]     n : hypothesis histogram pruning for decoding iteration i       [def 50]" << endl;
  cerr << "   -t     f : hypothesis threshold pruning                [def 0:NOT USED]" << endl;
  cerr << "   -nms     n : maximum number of substacks when using multiple stacks      [def 0: all]" << endl;
  cerr << "   -d     n : maximum iterations in decoding loop (trying to change all links from initial alignment)      [def 1]" << endl;
  cerr << "   -f     n : number of forward words to estimate future costs      [def 0: not used]" << endl;
  cerr << "   -hr     n : number of last decision identical for hypothesis recombination      [def 0: not used]" << endl;
  cerr << "   -m     n : maximum length for link combinations in the selection of links to be expanded (for exp=word)       [def 1]" << endl;
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
  cerr << "   -exp     link|word : search expanding all links from same word at same time ('word') or one link after the other ('link')  [def 'word']";   
  cerr << "   -dir     a|as|at|st|ts|stts|stIts|asIat[-X]: search direction: order in which links are added to hypothesis during search [def 'as']" << endl;
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
  cerr << "   -wwdaN    w : weight for the Nth word association model [def 0]" << endl;
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
  cerr << "   -wcl    w : weight for the accumulated crossing length [def 0:NOT USED]" << endl;
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
  cerr << "   -wstemb  w : weight for the stem bonus [def 0: NOT USED]" << endl;
  cerr << "   -wsynb  w : weight for the synonym bonus [def 0: NOT USED]" << endl;
  cerr << "   -cmatch  w : cost of a match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -cstem  w : cost of a stem match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -csyn  w :  cost of a synonym match (subsituted in wda1 model) [def -1: NOT USED]" << endl;
  cerr << "   -meshlc best|interp : each of the costs (association, rank, match, etc.) of a link between two meshes is calculated as: [def best]" << endl;
  cerr << "                  best: the best cost between the mesh words" << endl;
  cerr << "                  interp: the interpolation of the costs of the mesh words (weighted by the normalised probability of the words in the mesh)" << endl;

#ifdef _SERVER
  cerr << "   -port    i : port to place BIA server     [def 0:NOT server]" << endl;
#endif
  cerr << "   -outputtype     'word-links' (word-to-word links, ex 1-1), 'phrase-links' (phrase-to-phrase links, ex 1,2-1), or 'clusters' [def word-links]" << endl;
  cerr << "    (Cluster: minimal set of words such that all source (target) words are linked only to the target (source) words in the same set.)" << endl;
  cerr << "   -range     begin-end: limits range between sentence pair numbers 'begin' and 'end' [def '-' (not used)]" << endl;
  cerr << "   -verbose     l : verbosity level (0|1|2)                        [def 0]" << endl;
  cerr << "                  l=0: (no additional information)" << endl;
  cerr << "                  l>0: verbosity in outputfile.VERBOSE" << endl;
#ifdef _SERVER
  cerr << endl;
  cerr << "   [to kill a server, use:] bia-[version].client -die port" << endl;
#endif
  //cerr << endl << "   -h         : a detailed help" << endl;
  //cerr << "   -warranty         : warranty and conditions" << endl;
  if (err.size()>0) cerr << endl << "ERROR " << err << " while parsing BIA parameters !!" << endl;
  exit(0);
}

void param::reset () {
  //PORT=0;
  nNewStates=0;
  verbose=0;
  sInputType=0; tInputType=0;
  isfile.assign(maxLevels,"-");itfile.assign(maxLevels,"-");
  ofile="-";ostfile="-";otsfile="-";itgsfile="-";itgtfile="-";
  sumTab="-";tumTab="-";
  wordAsTab="-";mssTab="-"; tagAsTab="-";sfertTab="-";tfertTab="-";sClFertTab="-";tClFertTab="-";
  nLevels=1;
  outputType="word-links"; nbestFile="-"; nbestSize=1; range="-";direction="as";accu=false; init="empty";first="none";phas="possl";phLinkWeight="weighted"; oneToOne=false; backTrack=false; exp="word";multipleStacks="none";
  b.assign(10,50);
  t=0;n=5;d=1;f=0;hr=0;m=1;c=0;ct=0;nms=0;
  wlb.assign(maxLevels,0);wpp.assign(maxLevels,0);wcn.assign(maxLevels,0);wcl.assign(maxLevels,0);
  wup.assign(maxLevels,0); wus.assign(maxLevels,0); wut.assign(maxLevels,0); 
  whp.assign(maxLevels,0);whs.assign(maxLevels,0);wht.assign(maxLevels,0);wzp.assign(maxLevels,0);
  wwda1.assign(maxLevels,0); wwda2=0;wwda3=0;wwda4=0;wwda5=0;wwda6=0;
  wtga=0;wrk=0;wrks=0;wrkt=0;wrkb=0;wrkw=0;wum=0;wums=0;wumt=0;wf=0;wfs=0;wft=0;wchfilt=0;wchfilts=0;wchfiltt=0;
  w1to1=0; w1toN=0; wNto1=0; w1toNsum=0; wNtoM=0;
  wmatchb=0;wstemb=0;wsynb=0;cmatch=-1;cstem=-1;csyn=-1;meshLinkCost="best";
  nmodels.assign(maxLevels,0);
  models.assign(maxLevels,vector<string>());
  doba=0;dOnlyBestAsso=0;avgeOnlyBestAsso=0;
}

void param::read_params(int argc, char **argv){
  servermode=false;

  if (argc==1) error_params("");
  reset();

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
    if (wrd.compare("-itga")==0){
      inStr >> wrd;
      tagAsTab=wrd;
      continue;
    }
    if (wrd.compare("-imss")==0){
      inStr >> wrd;
      mssTab=wrd;
      continue;
    }
    if (wrd.compare("-isu")==0){
      inStr >> wrd;
      sumTab=wrd;
      continue;
    }
    if (wrd.compare("-itu")==0){
      inStr >> wrd;
      tumTab=wrd;
      continue;
    }
    if (wrd.compare("-isf")==0){
      inStr >> wrd;
      sfertTab=wrd;
      continue;
    }
    if (wrd.compare("-itf")==0){
      inStr >> wrd;
      tfertTab=wrd;
      continue;
    }
    if (wrd.compare("-iscf")==0){
      inStr >> wrd;
      sClFertTab=wrd;
      continue;
    }
    if (wrd.compare("-itcf")==0){
      inStr >> wrd;
      tClFertTab=wrd;
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

    if (wrd.substr(0,3)=="-it" ){
      int index;
      if (wrd.substr(3).compare("")==0) index=0;
      else index=atoi(wrd.substr(3).c_str());
      inStr >> wrd;
      itfile[index]=wrd;
      continue;
    }

    if (wrd.compare("-iwda")==0){
      inStr >> wrd;
      wordAsTab=wrd;
      continue;
    }

    if (wrd.compare("-lev")==0){
      inStr >> wrd;
      nLevels=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-sinputtype")==0){
      inStr >> wrd;
      sInputType=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-tinputtype")==0){
      inStr >> wrd;
      tInputType=atoi(wrd.data());
      continue;
    }

    if (wrd.substr(0,2)=="-b" && wrd!="-backtrack"){
      int index;
      if (wrd.substr(2).compare("")==0) index=0;
      else index=atoi(wrd.substr(2).c_str());
      inStr >> wrd;
      b[index]=atoi(wrd.data());
      continue;
    }

    if (wrd.compare("-t")==0){
      inStr >> wrd;
      t=atof(wrd.data());
      continue;
    }
    if (wrd.compare("-nms")==0){
      inStr >> wrd;
      nms=atoi(wrd.data());
      continue;
    }

    if (wrd.compare("-n")==0){
      inStr >> wrd;
      n=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-d")==0){
      inStr >> wrd;
      d=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-f")==0){
      inStr >> wrd;
      f=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-hr")==0){
      inStr >> wrd;
      hr=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-m")==0){
      inStr >> wrd;
      m=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-c")==0){
      inStr >> wrd;
      c=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-ct")==0){
      inStr >> wrd;
      ct=atof(wrd.data());
      continue;
    }
    if (wrd.compare("-dir")==0){
      inStr >> wrd;
      direction=wrd;
      continue;
    }
    if (wrd.compare("-accu")==0){
      accu=true;
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
      if ( wrd != "0"){
	oneToOne=true;
      }
      continue;
    }
    if (wrd.compare("-backtrack")==0){
      inStr >> wrd;
      if ( wrd != "0"){
	backTrack=true;
      }
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
    if (wrd.compare("-multiple-stacks")==0){
      inStr >> wrd;
      multipleStacks=wrd;
      continue;
    }
    if (wrd.compare("-doba")==0){
      inStr >> wrd;
      doba=atoi(wrd.data());
      continue;
    }
    if (wrd.compare("-aoba")==0){
      inStr >> wrd;
      avgeOnlyBestAsso=atoi(wrd.data());
      continue;
    }
    // word association scores
    if (wrd.substr(0,6)=="-wwda1"){
      int index;
      if (wrd.substr(6).compare("")==0) index=0;
      else index=atoi(wrd.substr(6).c_str());
      inStr >> wrd;
      wwda1[index]=atof(wrd.data());
      if (wwda1[index] != 0){
	++nmodels[index];
	models[index].push_back("wda1");
      }
      continue;
    }
    if (wrd.compare("-wwda2")==0){
      inStr >> wrd;
      wwda2=atof(wrd.data());
      if (wwda2 != 0){
	++nmodels[0];
	models[0].push_back("wda2");
      }
      continue;
    }
    if (wrd.compare("-wwda3")==0){
      inStr >> wrd;
      wwda3=atof(wrd.data());
      if (wwda3 != 0){
	++nmodels[0];
	models[0].push_back("wda3");
      }
      continue;
    }
    if (wrd.compare("-wwda4")==0){
      inStr >> wrd;
      wwda4=atof(wrd.data());
      if (wwda4 != 0){
	++nmodels[0];
	models[0].push_back("wda4");
      }
      continue;
    }
    if (wrd.compare("-wwda5")==0){
      inStr >> wrd;
      wwda5=atof(wrd.data());
      if (wwda5 != 0){
	++nmodels[0];
	models[0].push_back("wda5");
      }
      continue;
    }
    if (wrd.compare("-wwda6")==0){
      inStr >> wrd;
      wwda6=atof(wrd.data());
      if (wwda6 != 0){
	++nmodels[0];
	models[0].push_back("wda6");
      }
      continue;
    }
    // Part-of-Speech association score
    if (wrd.compare("-wtga")==0){
      inStr >> wrd;
      wtga=atof(wrd.data());
      if (wtga != 0){
	++nmodels[0];
	models[0].push_back("tga");
      }
      continue;
    }
    // association rank
    if (wrd.compare("-wrk")==0){
      inStr >> wrd;
      wrk=atof(wrd.data());
      if (wrk != 0){
	++nmodels[0];
	models[0].push_back("rk");
      }
      continue;
    }
    if (wrd.compare("-wrks")==0){
      inStr >> wrd;
      wrks=atof(wrd.data());
      if (wrks != 0){
	++nmodels[0];
	models[0].push_back("rks");
      }
      continue;
    }
    if (wrd.compare("-wrkt")==0){
      inStr >> wrd;
      wrkt=atof(wrd.data());
      if (wrkt != 0){
	++nmodels[0];
	models[0].push_back("rkt");
      }
      continue;
    }
    if (wrd.compare("-wrkb")==0){
      inStr >> wrd;
      wrkb=atof(wrd.data());
      if (wrkb != 0){
	++nmodels[0];
	models[0].push_back("rkb");
      }
      continue;
    }
    if (wrd.compare("-wrkw")==0){
      inStr >> wrd;
      wrkw=atof(wrd.data());
      if (wrkw != 0){
	++nmodels[0];
	models[0].push_back("rkw");
      }
      continue;
    }
    // match, stem and syn bonus
    if (wrd.compare("-wmatchb")==0){
      inStr >> wrd;
      wmatchb=atof(wrd.data());
      if (wmatchb != 0){
	++nmodels[0];
	models[0].push_back("matchb");
      }
      continue;
    }
    if (wrd.compare("-wstemb")==0){
      inStr >> wrd;
      wstemb=atof(wrd.data());
      if (wstemb != 0){
	++nmodels[0];
	models[0].push_back("stemb");
      }
      continue;
    }
    if (wrd.compare("-wsynb")==0){
      inStr >> wrd;
      wsynb=atof(wrd.data());
      if (wsynb != 0){
	++nmodels[0];
	models[0].push_back("synb");
      }
      continue;
    }
    // link bonus
    if (wrd.substr(0,4)=="-wlb"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wlb[index]=atof(wrd.data());
      if (wlb[index]!=0){
	++nmodels[index];
	models[index].push_back("lb");
      }
      continue;
    }
    // phrase-link penalty
    if (wrd.substr(0,4)=="-wpp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wpp[index]=atof(wrd.data());
      if (wpp[index]!=0){
	++nmodels[index];
	models[index].push_back("pp");
      }
      continue;
    }
    // unlinked word penalties
    if (wrd.substr(0,4)=="-wup"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wup[index]=atof(wrd.data());
      if (wup[index]!=0){
	++nmodels[index];
	models[index].push_back("up");
      }
      continue;
    }
    if (wrd.substr(0,4)=="-wus"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wus[index]=atof(wrd.data());
      if (wus[index]!=0){
	++nmodels[index];
	models[index].push_back("us");
      }
      continue;
    }
    if (wrd.substr(0,4)=="-wut"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wut[index]=atof(wrd.data());
      if (wut[index]!=0){
	++nmodels[index];
	models[index].push_back("ut");
      }
      continue;
    }

    //unlinked word model
    if (wrd.compare("-wum")==0){
      inStr >> wrd;
      wum=atof(wrd.data());
      if (wum != 0){
	++nmodels[0];
	models[0].push_back("um");
      }
      continue;
    }
    if (wrd.compare("-wums")==0){
      inStr >> wrd;
      wums=atof(wrd.data());
      if (wums != 0){
	++nmodels[0];
	models[0].push_back("ums");
      }
      continue;
    }
    if (wrd.compare("-wumt")==0){
      inStr >> wrd;
      wumt=atof(wrd.data());
      if (wumt != 0){
	++nmodels[0];
	models[0].push_back("umt");
      }
      continue;
    }
    // fertility costs
    if (wrd.compare("-wf")==0){
      inStr >> wrd;
      wf=atof(wrd.data());
      if (wf != 0){
	++nmodels[0];
	models[0].push_back("f");
      }
      continue;
    }
    if (wrd.compare("-wfs")==0){
      inStr >> wrd;
      wfs=atof(wrd.data());
      if (wfs != 0){
	++nmodels[0];
	models[0].push_back("fs");
      }
      continue;
    }
    if (wrd.compare("-wft")==0){
      inStr >> wrd;
      wft=atof(wrd.data());
      if (wft != 0){
	++nmodels[0];
	models[0].push_back("ft");
      }
      continue;
    }
    // crossing penalties
    if (wrd.substr(0,4)=="-wcn"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wcn[index]=atof(wrd.data());
      if (wcn[index]!=0){
	++nmodels[index];
	models[0].push_back("cn");
      }
      continue;
    }
    if (wrd.substr(0,4)=="-wcl"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wcl[index]=atof(wrd.data());
      if (wcl[index]!=0){
	++nmodels[index];
	models[0].push_back("cl");
      }
      continue;
    }
    // gap penalties
    if (wrd.substr(0,4)=="-whp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      whp[index]=atof(wrd.data());
      if (whp[index]!=0){
	++nmodels[index];
	models[index].push_back("hp");
      }
      continue;
    }
    if (wrd.substr(0,4)=="-whs"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      whs[index]=atof(wrd.data());
      if (whs[index]!=0){
	++nmodels[index];
	models[index].push_back("hs");
      }
      continue;
    }
    if (wrd.substr(0,4)=="-wht"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wht[index]=atof(wrd.data());
      if (wht[index]!=0){
	++nmodels[index];
	models[index].push_back("ht");
      }
      continue;
    }
    // zigzag penalty
    if (wrd.substr(0,4)=="-wzp"){
      int index;
      if (wrd.substr(4).compare("")==0) index=0;
      else index=atoi(wrd.substr(4).c_str());
      inStr >> wrd;
      wzp[index]=atof(wrd.data());
      if (wzp[index]!=0){
	++nmodels[index];
	models[index].push_back("zp");
      }
      continue;
    }

    // link type counts
    if (wrd.compare("-w1to1")==0){
      inStr >> wrd;
      w1to1=atof(wrd.data());
      if (w1to1 != 0){
	++nmodels[0];
	models[0].push_back("1to1");
      }
      continue;
    }
    if (wrd.compare("-w1toN")==0){
      inStr >> wrd;
      w1toN=atof(wrd.data());
      if (w1toN != 0){
	++nmodels[0];
	models[0].push_back("1toN");
      }
      continue;
    }
    if (wrd.compare("-wNto1")==0){
      inStr >> wrd;
      wNto1=atof(wrd.data());
      if (wNto1 != 0){
	++nmodels[0];
	models[0].push_back("Nto1");
      }
      continue;
    }
    if (wrd.compare("-w1toNsum")==0){
      inStr >> wrd;
      w1toNsum=atof(wrd.data());
      if (w1toNsum != 0){
	++nmodels[0];
	models[0].push_back("1toNsum");
      }
      continue;
    }
    if (wrd.compare("-wNtoM")==0){
      inStr >> wrd;
      wNtoM=atof(wrd.data());
      if (wNtoM != 0){
	++nmodels[0];
	models[0].push_back("NtoM");
      }
      continue;
    }

    // chunk filter
    if (wrd.compare("-wchfilt")==0){
      inStr >> wrd;
      wchfilt=atof(wrd.data());
      if (wchfilt != 0){
	++nmodels[0];
	models[0].push_back("chfilt");
      }
      continue;
    }
    if (wrd.compare("-wchfilts")==0){
      inStr >> wrd;
      wchfilts=atof(wrd.data());
      if (wchfilts != 0){
	++nmodels[0];
	models[0].push_back("chfilts");
      }
      continue;
    }
    if (wrd.compare("-wchfiltt")==0){
      inStr >> wrd;
      wchfiltt=atof(wrd.data());
      if (wchfiltt != 0){
	++nmodels[0];
	models[0].push_back("chfiltt");
      }
      continue;
    }
    if (wrd.compare("-cmatch")==0){
      inStr >> wrd;
      cmatch=atof(wrd.data());
      //      if (cmatch != 0){++nmodels[0];}
      continue;
    }
    if (wrd.compare("-cstem")==0){
      inStr >> wrd;
      cstem=atof(wrd.data());
      //      if (cstem != 0){++nmodels[0];}
      continue;
    }
    if (wrd.compare("-csyn")==0){
      inStr >> wrd;
      csyn=atof(wrd.data());
      //      if (csyn != 0){++nmodels[0];}
      continue;
    }
    if (wrd.compare("-o")==0){
      inStr >> wrd;
      ofile=wrd;
      continue;
    }
    if (wrd.compare("-meshlc")==0){
      inStr >> wrd;
      meshLinkCost=wrd;
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
    if (wrd.compare("-outputtype")==0){
      inStr >> wrd;
      outputType=wrd;
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
      verbose=atoi(wrd.data());
      if (verbose<0 || verbose >3) error_params("verbose out_of_bounds");
      continue;
    }

#ifdef _SERVER
    if (wrd.compare("-port")==0){
      inStr >> wrd;
      PORT=atoi(wrd.data());
      servermode=true;
      continue;
    }
#endif
    string err("param ");
    err.append(wrd).append (" UNKNOWN,");
    error_params(err);
  }
 
  //****************************************
  // checking parameters
  //****************************************
  vector<string> isfileTmp;
  for (vector<string>::iterator v=isfile.begin(); v!=isfile.end();++v){
    if ((*v).compare("-")!=0) isfileTmp.push_back(*v);
  }
  isfile=isfileTmp;
  vector<string> itfileTmp;
  for (vector<string>::iterator v=itfile.begin(); v!=itfile.end();++v){
    if ((*v).compare("-")!=0) itfileTmp.push_back(*v);
  }
  itfile=itfileTmp;
  //vector<string>::size_type nLevels=isfile.size();
  wwda1.erase(wwda1.begin()+nLevels,wwda1.end());
  wlb.erase(wlb.begin()+nLevels,wlb.end());
  wpp.erase(wpp.begin()+nLevels,wpp.end());
  wup.erase(wup.begin()+nLevels,wup.end());
  wus.erase(wus.begin()+nLevels,wus.end());
  wut.erase(wut.begin()+nLevels,wut.end());
  wcn.erase(wcn.begin()+nLevels,wcn.end());
  wcl.erase(wcl.begin()+nLevels,wcl.end());
  whp.erase(whp.begin()+nLevels,whp.end());
  whs.erase(whs.begin()+nLevels,whs.end());
  wht.erase(wht.begin()+nLevels,wht.end());
  wzp.erase(wzp.begin()+nLevels,wzp.end());

  if (wwda1[0]==0 && wwda2==0 && wwda3==0 && wwda4==0 && wwda5==0 && wwda6==0){
    string err("All word association weights are 0 ");
    error_params(err);
  }
  // make next test except if launching server (wordAsTab!="-")
  if (wup[0]==0 && wlb[0]==0 && wus[0]==0 && wut[0]==0 && wum==0 && wums==0 && wumt==0 && wf==0 && wfs==0 && wft==0 && (servermode==false || wordAsTab=="-")){
    string err("Link bonus, unlinked word penalty and fertility models can't all have 0 weight.");
    error_params(err);
  }
  for (int lev=1; lev<nLevels;++lev){
    if (wup[lev]==0 && wlb[lev]==0 && wus[lev]==0 && wut[lev]==0){
      string err("Level $lev: link bonus and unlinked word penalty models can't both have 0 weight.");
      error_params(err);
    }
  }

 if (isfile[0].compare("-")==0){//si servermode, l'input el passa el client!!
   if (!servermode){
     string err("is=-");
     error_params(err);
   }
  }

  if (itfile[0].compare("-")==0){//si servermode, l'input el passa el client!!
    if (!servermode){
      string err("it=-");
      error_params(err);
    }
  }

  if (wordAsTab.compare("-")==0){
    string err("iwda=-");
    error_params(err);
  }
  if ((wf>0 || wfs>0 || wft>0)&& sfertTab=="-" && sClFertTab=="-" && tfertTab=="-" && tClFertTab=="-"){
    string err("No fertility table specified on input");
    error_params(err);
  }
  if (wtga>0 && tagAsTab.compare("-")==0){
    string err("itga=-");
    error_params(err);
  }

  if (multipleStacks!="coverage" && multipleStacks!="source-coverage" && multipleStacks!="target-coverage"){nms=0;}
  /*  if (verbose>0 && ofile.compare("-")==0){
    string err("verbose>0 without output file");
    error_params(err);
    }*/
#ifdef _SERVER
  if (PORT<10000 || PORT>=50000){
    string err("port should be [10000,50000)");
    error_params(err);
  }
#endif
}

void param::print_server_params () const {
  if (servermode){cerr << ":                    SERVER mode                      :" << endl;}
  cerr << "TABLES:"<< endl;
  cerr << " word association scores table:\t" << wordAsTab << endl;
  cerr << " match-stem-syn association table:\t" << mssTab << endl;
  cerr << " tag association scores table:\t" << tagAsTab << endl;
  cerr << " source unlinked model table:\t" << sumTab << endl;
  cerr << " target unlinked model table:\t" << tumTab << endl;
  cerr << " source fertility table:\t" << sfertTab << endl;
  cerr << " target fertility table:\t" << tfertTab << endl;
  cerr << " source class fertility table:\t" << sClFertTab << endl;
  cerr << " target class fertility table:\t" << tClFertTab << endl;
}

void param::print_client_params () const {
  cerr << "INPUT files (inputtypes: src:" <<sInputType<<" trg:"<<tInputType<< " range:" << range << ") :" << endl;
  for (int cnt=0; cnt<isfile.size(); ++cnt){
    cerr << " src (level "<<cnt<<"):\t" << isfile[cnt] << endl;
  }
  for (int cnt=0; cnt<itfile.size(); ++cnt){
    cerr << " trg (level "<<cnt<<"):\t" << itfile[cnt] << endl;
  }
  cerr << " tag src:\t" << itgsfile << endl;
  cerr << " tag trg:\t" << itgtfile << endl;
  if (ofile.compare("-")==0) cerr << "OUTFILE      : STDOUT" << endl;
  else  cerr << "OUTFILE      : " << ofile << endl;
  if (ostfile.compare("-")!=0) cerr << "Src-Trg OUTFILE      : "<<ostfile << endl;
  if (otsfile.compare("-")!=0) cerr << "Trg-Src OUTFILE      : "<<otsfile << endl;
  cerr << endl;
  cerr << "MODEL weights: "<<endl;
  for (int cnt=0; cnt<isfile.size();++cnt){
    cerr <<"Level "<<cnt<<": -wwda1:"<<wwda1[cnt];
    if (cnt==0){
      cerr<<" -wwda2:"<<wwda2<<" -wwda3:"<<wwda3<<" -wwda4:"<<wwda4<<" -wwda5:"<<wwda5<<" -wwda6:"<<wwda6<<" -wtga:"<<wtga<<" -wrk:"<<wrk<<" -wrks:"<<wrks<<" -wrkt:"<<wrkt<<" -wrkb:"<<wrkb<<" -wrkw:"<<wrkw<<endl;
      cerr<<" -wum:"<<wum<<" -wums:"<<wums<<" -wumt"<<wumt<<" -wf:"<<wf<<" -wfs:"<<wfs<<" -wft:"<<wft<<" -wchfilts:"<<wchfilts<<" -wchfiltt:"<<wchfiltt<<" -wchfilt:"<<wchfilt<<endl;
      cerr<<" -wmatchb:"<<wmatchb<<" -wstemb:"<<wstemb<<" -wsynb:"<<wsynb<<" -cmatch:"<<cmatch<<" -cstem:"<<cstem<<" -csyn:"<<csyn<<" meshlc:"<<meshLinkCost<<endl;
    }
    cerr<<" -wlb:"<<wlb[cnt]<<" -wpp:"<<wpp[cnt]<<" -wup:"<<wup[cnt]<<" -wus:"<<wus[cnt]<<" -wut:"<<wut[cnt]<<endl;
    cerr<<" -wcn:"<<wcn[cnt]<<" -wcl:"<<wcl[cnt]<<" -whp:"<<whp[cnt]<<" -whs:"<<whs[cnt]<<" -wht:"<<wht[cnt]<<" -wzp:"<<wzp[cnt]<<endl;
    cerr<<" -w1to1:"<<w1to1<<" -w1toN:"<<w1toN<<" -wNto1:"<<wNto1<<" -w1toNsum:"<<w1toNsum<<" -wNtoM:"<<wNtoM<<endl;
  }
  //if (lt!=0) cerr << " lt=" << lt;
  cerr << endl;
  cerr << "LEVELS: nLevels=" << nLevels<<endl;
  cerr << "LINKS pruning: n=" << n<<endl;
  cerr << "SEARCH params: b="; for (int cnt=0;cnt<d;++cnt){cerr << b[cnt];if (cnt<d-1) cerr <<",";}
  cerr << " t=" << t<<" max number of substacks: nms="<<nms<<" max combination length: m="<<m<<" max combinations: c="<<c<<" combinations threshold pruning:"<<ct<<" doba:"<<doba<<" aoba:"<<avgeOnlyBestAsso<<endl;
  cerr << "max decoding iters:d="<<d<<" Initial alignment:"<<init<<" Direction="<<direction<<" first:"<<first<<" accu:"<<accu<<" expanding:"<<exp<<" multiple stacks:"<<multipleStacks<<endl;
  cerr << "forward cost estimation  (words ahead):"<<f<<" hypothesis recombination:hr="<<hr;
  cerr << endl;
  cerr << "SETTINGS     : verbose=" << verbose<<" output type:"<<outputType<<" n-best-list:"<<nbestFile<<" "<<nbestSize<<" phlink-count:"<<phLinkWeight<<" one-to-one:"<<oneToOne<<" backtrack:"<<backTrack<<endl;
  cerr << endl;
}
