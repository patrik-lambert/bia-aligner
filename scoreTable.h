#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include "defs.h"
#include "params.h"
#include "math.h"
#include "sentPair.h"
#include "alUnit.h"
#include "phrase.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

#ifndef QUANTIZATION
#define QUANTIZATION

class quantization {
 public:
  qscore max;
  cost fmax;
  cost nDiv;
  cost minuslog10;
  // public:
  quantization () {
    max=MAX_QSCORE;
    fmax=max*1.0;
    nDiv=max+1.0;
    minuslog10=-1.0*log(10.0);
  }
  //  qscore quantize(float low, float high,float score) const {
  qscore quantize(cost cst) const {
    // qscore= p(x)*max; cost=-log[p(x)]/log10;=> qscore=max*exp[-log10*cost]
    cost fq=fmax*exp(minuslog10*cst);
    cost fl=floor(fq);
    cost rem=fq-fl;
    qscore q=int(fl);
    if (rem>0.5){++q;}
    if (q>max){q=max;}
    return q;
  }
  cost dequantize(qscore score) const{
    return -log10((score+0.5)/nDiv);
  }
};
#endif 


#ifndef QSCORETABLE
#define QSCORETABLE

// QUANTIZED TABLE OF ASSOCIATION SCORES BETWEEN TOKENS (WORDS, TAGS, ETC.)
//***************************************************************
class wdAssoQScoreTable {
  hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit> jash_wdas;
  quantization quant;
 public:
  wdAssoQScoreTable (string fname, phraseDetection & sPhSearch, phraseDetection & tPhSearch, vocTable & svoc, vocTable & tvoc){load(fname,sPhSearch,tPhSearch,svoc,tvoc);};
  wdAssoQScoreTable (string fname, vocTable & svoc, vocTable & tvoc){load(fname,svoc,tvoc);};
  wdAssoQScoreTable (){};
  void load (string fname, phraseDetection & sPhSearch, phraseDetection & tPhSearch, vocTable & svoc, vocTable & tvoc);
  void load (string fname, vocTable & svoc, vocTable & tvoc);
  string print (const vocTable & svoc, const vocTable & tvoc) const;
  pair<bool,qscore> score(const alUnit & src, const alUnit & trg, int num) const {
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      return make_pair(true,(cit->second)[num]);
    }else{
      return make_pair(false,0);
    }
  }
  pair<bool,cost> cst(wordIndex srcwd, wordIndex trgwd, int num) const {
    alUnit src(srcwd);
    alUnit trg(trgwd);
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      return make_pair(true,quant.dequantize((cit->second)[num]));
    }else{
      return make_pair(false,0);
    }
  }
  int size(){return jash_wdas.size();}
  bool scoreVec(wordIndex srcwd, wordIndex trgwd, vector<qscore> & vec) const ;
  bool scoreVec(const alUnit & src, const alUnit & trg, vector<qscore> & vec) const;
  bool cstVec(wordIndex srcwd, wordIndex trgwd, vector<cost> & vec) const;
  bool cstVec(const alUnit & src, const alUnit & trg, vector<cost> & vec) const;
  cost combCst(wordIndex src, wordIndex trg, const param & pars, cost worstCost);
  cost combCst(alUnit src, alUnit trg, const param & pars, cost worstCost);
  vector<cost>::size_type nCosts() const {return jash_wdas.begin()->second.size();};
  void addEntry(alUnit & srcv, alUnit & trgv, vector<cost> & vec);
};
#endif

//**************************************************
#ifndef WDASSOSCORE
#define WDASSOSCORE

struct wdAssoScore {
  vector<cost> vcosts;
  bool match,stem,syn;
  float trgRankForSrc; // not int because it can be the linear combination of various integers*prob in confusion network
  float srcRankForTrg;
  wdAssoScore(){
    trgRankForSrc=0;
    srcRankForTrg=0;
    vcosts.clear();
    match=false;
    stem=false;
    syn=false;
};
wdAssoScore(const vector<cost> & _vcosts, bool _match=false, bool _stem=false, bool _syn=false, int trgRk=-1, int srcRk=-1):match(_match), stem(_stem), syn(_syn), trgRankForSrc(trgRk), srcRankForTrg(srcRk){
    vcosts=_vcosts;
  }
  string print() const;
  void normalise( float Z);
};

struct ltcst
{
  float w1, w2, w3, w4, w5, w6;
ltcst( float _w1, float _w2, float _w3, float _w4, float _w5, float _w6 ) : w1(_w1), w2(_w2), w3(_w3), w4(_w4), w5(_w5), w6(_w6) {}
  bool operator()(const pair<alUnit,wdAssoScore > & p1, const pair<alUnit,wdAssoScore > & p2) const
  {
    cost cst1,cst2;
    cst1=0;
    cst2=0;
    if (w1!=0){
      cst1+=w1 * p1.second.vcosts[0];
      cst2+=w1 * p2.second.vcosts[0];
    }
    if (w2!=0){
      cst1+=w2 * p1.second.vcosts[1];
      cst2+=w2 * p2.second.vcosts[1];
    }
    if (w3!=0){
      cst1+=w3 * p1.second.vcosts[2];
      cst2+=w3 * p2.second.vcosts[2];
    }
    if (w4!=0){
      cst1+=w4 * p1.second.vcosts[3];
      cst2+=w4 * p2.second.vcosts[3];
    }
    if (w5!=0){
      cst1+=w5 * p1.second.vcosts[4];
      cst2+=w5 * p2.second.vcosts[4];
    }
    if (w6!=0){
      cst1+=w6 * p1.second.vcosts[5];
      cst2+=w6 * p2.second.vcosts[5];
    }

    if (cst1 < cst2 || (cst1 == cst2 && p1.first.size() < p2.first.size())){
      return true;
    }else if (cst1 == cst2 && p1.first.size() == p2.first.size()){
      for (alUnit::size_type n=0;n<p1.first.size();++n){
	if (p1.first.at(n) < p2.first.at(n)){
	  return true;
	}else if (p1.first.at(n) > p2.first.at(n) || n==p1.first.size()-1){
	  return false;
	}
      }
    }else{ 
      return false;
    }
  }
};

typedef set< pair<alUnit,wdAssoScore >, ltcst > wd2cst;

#endif

#ifndef MATCHSTEMSYNTABLE
#define MATCHSTEMSYNTABLE
struct matchStemSyn {
  bool match,stem,syn;
  matchStemSyn(bool _match, bool _stem, bool _syn): match(_match), stem(_stem), syn(_syn) {};
  matchStemSyn() {};
};

class matchStemSynTable { 
  hash_map<pair<alUnit,alUnit>,matchStemSyn,hashfpairalunit, hasheqpairalunit> jash_wdas;
 public:
  matchStemSynTable (){};
  void load(string fname, vocTable & svoc, vocTable & tvoc);

  bool getMatchStemSyn(const alUnit & src, const alUnit & trg, bool & match, bool & stem, bool & syn) const {
    hash_map<pair<alUnit,alUnit>,matchStemSyn,hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      match=cit->second.match;
      stem=cit->second.stem;
      syn=cit->second.syn;
      //cout<<"[getMatchStemSyn] match:"<<match<<" stem:"<<stem<<" syn:"<<syn<<endl;
      return true;
    }else{
      return false;
    }
  }
  string print(const vocTable & svoc, const vocTable & tvoc) const {
    ostringstream oss("");
    for (hash_map<pair<alUnit,alUnit>, matchStemSyn, hashfpairalunit, hasheqpairalunit>::const_iterator it=jash_wdas.begin();it!=jash_wdas.end();++it){
      oss<<it->first.first.print(svoc)<<" ||| "<<it->first.second.print(tvoc)<<" ||| "<<it->second.match<<" "<<it->second.stem<<" "<<it->second.syn<<endl;
    }
    return oss.str();
  }
  string printVoc(const vocTable & svoc, const vocTable & tvoc) const {
    ostringstream oss("");
    for (hash_map<pair<alUnit,alUnit>, matchStemSyn, hashfpairalunit, hasheqpairalunit>::const_iterator it=jash_wdas.begin();it!=jash_wdas.end();++it){
      // source word
      for (vector<formType>::const_iterator unit=it->first.first.begin();unit!=it->first.first.end();++unit){
	if (unit != it->first.first.begin()){oss<<" ";}
	oss<<svoc.form(*unit);
      }
      oss<<" ||| ";
      // target word
      for (vector<formType>::const_iterator unit=it->first.second.begin();unit!=it->first.second.end();++unit){
	if (unit != it->first.second.begin()){oss<<" ";}
	oss<<tvoc.form(*unit);
      }
      oss<<" ||| "<<it->second.match<<" "<<it->second.stem<<" "<<it->second.syn<<endl;
    }
    return oss.str();
  }
};
#endif

#ifndef SCORETABLE
#define SCORETABLE

// TABLE OF ASSOCIATION SCORES (COSTS) BETWEEN TOKENS (WORDS, TAGS, ETC.)
//***************************************************************
class wdAssoScoreTable {
  hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit> jash_wdas;

 public:
  wdAssoScoreTable (const wdAssoQScoreTable & motherTable, const matchStemSynTable & mssTab, const sentPair & sp, const param & pars){loadByPruningForSentPair(motherTable,mssTab,sp,pars);};
  wdAssoScoreTable (){};
  void loadByPruningForSentPair (const wdAssoQScoreTable & motherWdas, const matchStemSynTable & mssTab, const sentPair & sp, const param & pars);
  pair<bool,cost> cst(const alUnit & src, const alUnit & trg, int num) const {
    hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      return make_pair(true,(cit->second.vcosts)[num]);
    }else{
      return make_pair(false,0);
    }
  }
  bool cstVec(wordIndex srcwd, wordIndex trgwd, vector<cost> & vec) {
    alUnit src(srcwd);
    alUnit trg(trgwd);
    hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      for (vector<cost>::const_iterator qv=cit->second.vcosts.begin();qv!=cit->second.vcosts.end();++qv){
	vec.push_back(*qv);
      }
      return true;
    }else{
      return false;
    }
  }
  bool cstVec(const alUnit & src, const alUnit & trg, vector<cost> & vec) {
    hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      for (vector<cost>::const_iterator qv=cit->second.vcosts.begin();qv!=cit->second.vcosts.end();++qv){
	vec.push_back(*qv);
      }
      return true;
    }else{
      return false;
    }
  }
  bool assoScore(wordIndex srcwd, wordIndex trgwd, wdAssoScore & ascore) {
    alUnit src(srcwd);
    alUnit trg(trgwd);
    hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      ascore=cit->second;
      //ascore.trgRankForSrc=cit->second.trg
      //for (vector<cost>::const_iterator qv=cit->second.vcosts.begin();qv!=cit->second.vcosts.end();++qv){
      //vec.push_back(*qv);
      //}
      return true;
    }else{
      return false;
    }
  }
  bool assoScore(const alUnit & src, const alUnit & trg, wdAssoScore & ascore) {
    hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      ascore=cit->second;
      //ascore.trgRankForSrc=cit->second.trg
      //for (vector<cost>::const_iterator qv=cit->second.vcosts.begin();qv!=cit->second.vcosts.end();++qv){
      //vec.push_back(*qv);
      //}
      return true;
    }else{
      return false;
    }
  }
  int size() const {return jash_wdas.size();}
  string display(const vocTable & svoc, const vocTable & tvoc) const;
};

#endif

