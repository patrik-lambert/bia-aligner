#include <vector>
#include <set>
#include <list>
#include <time.h>
#include <sys/times.h>
#include <cstring>
#include "math.h"
#include "defs.h"
#include "wlink.h"
#include "params.h"
#include "sentPair.h"
#include "linkCluster.h"

#if __GNUC__>2
#include <ext/hash_set>
using __gnu_cxx::hash_set;
#else
#include <hash_set>
#endif

using namespace std;

//size of problem:
#define STMODUL 1000                                                                                

#ifndef STATE
#define STATE

class state{
 public:
  unsigned int id;
  vector<bool> stcov; // position ordered source-target pair coverage vector
  vector<pair<linkIndex,bool> > lnkids; // links with action: addlink (true) or remove (false)
  vector<int> scov; // src positions coverage vector 
  int nscov; // number of src positions covered
  int internalCrossNumber, internalCrossLength; // crossing number and length in internal phrase-based links
  int nInternalZigzags; // number of zigzags within phrase-based links
  vector<int> tcov; // trg positions coverage vector
  int ntcov; // number of trg positions covered
  list<vector<int> > hist; // history of decisions taken for hypothesis recombination
  int nhist; //number of elements in history list
  cost cst,transmissibleCst; // cst=transmissibleCst + crossing cost + gap penalty cost
  cost fcst; // "forward cost": cost of the best alignment considering future positions
  vector<cost> costs; //llista de costs segons model
  linkIndex first,last,nstcov; //first and last positions covered, number of positions covered
  state *father;
  int nPtr;
  vector<int> fcHist; //future cost calculated at each source (or target) position
  vector<int> lcov; //link coverage vector, used only in n-best lists
  state(){
    nPtr=0;
  };
  //state (unsigned int & last_state_id, state * _father, const wlink & lnk, bool addLink, const wlinkSequence & possLinks, param & pars, const sentPair & sp, ostringstream & oss, int level);
  state (unsigned int & last_state_id, state * _father, const vector<pair<wlink,bool> > & lnks, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, param & pars, const sentPair & sp, ostringstream & oss, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits, const futureCostEstimation & fcEstim, string direction="");
  state (const state & _state){
    id=_state.id;
    lcov=_state.lcov; 
    lnkids=_state.lnkids;
    stcov=_state.stcov;
    scov=_state.scov;
    nscov=_state.nscov;
    tcov=_state.tcov;
    ntcov=_state.ntcov;
    cst=_state.cst;
    transmissibleCst=_state.transmissibleCst;
    fcst=_state.fcst;
    costs=_state.costs;
    first=_state.first;
    last=_state.last;
    nstcov=_state.nstcov;
    father=_state.father;
    internalCrossNumber=_state.internalCrossNumber;
    internalCrossLength=_state.internalCrossLength;
    nInternalZigzags=_state.nInternalZigzags;
    hist=_state.hist;
    nhist=_state.nhist;
    nPtr=0;
    fcHist=_state.fcHist;
  };
  ~state(){};
   bool operator< (const state & y) const {
     //    return cst<y.cst;
     return fcst<y.fcst || (fcst==y.fcst && cst<y.cst) ||  (fcst==y.fcst && cst==y.cst && id<y.id);
   }
  bool operator> (const state & y) const {
    return cst>y.cst;
  }
  // we cannot say whether two states are equal because the only way to know this is comparing lcov vectors, which are not always defined.
  //  bool operator= (const state & y) const {
  //  }
  bool empty () const {
    return (nstcov==0);
  }
  void setEmptyAl (linkIndex nstpairs, linkIndex nPossLinks, const sentPair & sp, int level, const param & pars);
  string print (const wlinkSequence & posOrderedPossLinks, const param & pars) const;
  string printAlignment(const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars ) const;
  string printForNbest (const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars,int numSentPair ) const;
  void printLinkSequence(const wlinkSequence & posOrderPossLinks, wlinkSequence & linkseq) const;
  void calculateTrgChFiltLimits(vector<pair<int,int> > & chFiltLims, const sentPair & sp, const wlinkSequence & posiOrderPossLinks,int level); 
  void calculateSrcChFiltLimits(vector<pair<int,int> > & chFiltLims, const sentPair & sp, const wlinkSequence & posiOrderPossLinks,int level); 
  bool compatibleWithLink( const wlink & lnk, const wlinkCoverage & posiOrderLinkCoverage, const wordLinkSequence & srcLinks, const wordLinkSequence & trgLinks, const param & pars, set<linkIndex> & conflicts ) const;
  void retrieveLinkCoverage( vector<int> & vec ) const;
  void setlcov( wlinkSequence::size_type nposslinks ){
    lcov.assign(nposslinks,false);
    retrieveLinkCoverage(lcov);
  }
  void setlcov( const vector<int> & vec ){
    if (lcov.size()==0){
      lcov=vec;
    }
  }
};

struct ltstate
{
  bool operator()(const state* s1, const state* s2) const
  {
    return s1->fcst < s2->fcst || (s1->fcst == s2->fcst && s1->cst < s2->cst ) || (s1->fcst == s2->fcst && s1->cst == s2->cst && s1->id < s2->id);
  }
};

struct ltpairuintpaircostuint
{
  bool operator()(const pair<unsigned int, pair<cost,unsigned int> > & p1, const pair<unsigned int, pair<cost,unsigned int> > & p2) const
  {
    return ( (p1.second.first < p2.second.first) || (p1.second.first == p2.second.first && p1.second.second < p2.second.second) );
  }
};

#endif

#ifndef SYMMETRISATION
#define SYMMETRISATION
struct symmetrisation {
  public:
  state* getRefinedIntersection ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits);
  state* getIntersection ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits);
  state* getUnion ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits);
  state* getRefinedUnion ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits);
  //  state* grow ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, bool diagonal=false,bool final=false,bool bothuncovered=false);
  string grow ( state *s1, state *s2, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp, bool diagonal=false,bool final=false,bool bothuncovered=false, int level=0);
};
#endif

#ifndef HASHINTLINKSEQ
#define HASHINTLINKSEQ
                                                                                    
class hashfintlinkseq{
 public:
  size_t operator()(pair<int,wlinkSequence> const &p) const{
    int res=0;
    int i=0;
    for (wlinkSequence::const_iterator it=p.second.begin();it!=p.second.end();++it){
      res += (size_t) pow((double)it->id , double((i%6)+1));
      ++i;
    }
    res += (size_t) pow((double)p.first , double((i%6)+1));
    return res%STMODUL;
  }
};

class hasheqintlinkseq{
 public:
  bool operator()(pair<int,wlinkSequence> const &p1, pair<int,wlinkSequence> const &p2) const{
    return p1.first==p2.first && p1.second==p2.second;
  }
};

#endif 

#ifndef HASHLISTINTS
#define HASHLISTINTS
                                                                                    
class hashflistints{
 public:
  size_t operator()(list<int> const &v) const{
    int res=0;
    int i=0;
    for (list<int>::const_iterator it=v.begin();it!=v.end();++it){
      if (*it>=0){res += (size_t) pow((double)*it , double((i%6)+1));}
      ++i;
    }
    return res%STMODUL;
  }
};

class hasheqlistints{
 public:
  bool operator()(list<int> const &l1, list<int> const &l2) const{
    return l1==l2;
  }
};

#endif 

#ifndef HASHLISTVECINTS
#define HASHLISTVECINTS
                                                                                    
class hashflistvecints{
 public:
  size_t operator()(list<vector<int> > const &lv) const{
    int res=0;
    int i=0;
    for (list<vector<int> >::const_iterator lit=lv.begin();lit!=lv.end();++lit){
      for (vector<int>::const_iterator it=lit->begin();it!=lit->end();++it){
	if (*it>=0){res += (size_t) pow((double)*it , double((i%6)+1));}
	++i;
      }
    }
    return res%STMODUL;
  }
};

class hasheqlistvecints{
 public:
  bool operator()(list<vector<int> > const &l1, list<vector<int> > const &l2) const{
    return l1==l2;
  }
};

#endif 

#ifndef HASHLISTSTRING
#define HASHLISTSTRING

class hashfliststring : public unary_function<list<string>, size_t>{
 public:
  size_t operator()(list<string> const &ls) const{
    unsigned int res=0;
    ostringstream oss("");
    for (list<string>::const_iterator it=ls.begin();it!=ls.end();++it){
      if (it!=ls.begin()){oss<<" ";}
      oss<<*it;
    }
    string s=oss.str();
    for (int i=0;i<s.size();i++){
      res += (size_t) (unsigned int) pow((double)s.at(i), double((i%6)+1));
    }
    return res%STMODUL;
  }
};

class hasheqliststring{
 public:
  bool operator()(list<string> const &ls1, list<string> const &ls2) const{
    return ls1==ls2;
  }
};

#endif


#ifndef STATESTACK
#define STATESTACK

class stateStack: public set< state*,ltstate > {
 public:
  ~stateStack(){
    for (stateStack::iterator st=this->begin(); st != this->end(); ++st){
      //cout<<"deleting:"<<*st<<" nPtr:"<<(*st)->nPtr;
      --(*st)->nPtr;
      //cout<<" nPtr(2):"<<(*st)->nPtr<<endl;
      if ((*st)->nPtr==0){
	//cout<<*st<<" deleted\n";
	delete *st;
      }
    }
  }
  string nbest ( int numbest, const wlinkCoverage & posiOrderLinkCoverage, const phraseLinkTable & existingLinks, int level) const;
  string onebest ( const wlinkSequence & posslnks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars ) const;
  string nbest (const wlinkSequence & posslnks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars, int numbest, int numSentPair ) const;
  string print (const wlinkSequence & posOrderPosslnks, const param & pars ) const;
  void cleverInsert(state *s){    
    this->insert(s);
    //cout<<"inserting (BICHINHO):"<<s<<" nPtr:"<<s->nPtr;
    ++s->nPtr;
    //cout<<" nPtr(2):"<<s->nPtr<<endl;
  }
  void cleverInsert(stateStack::iterator pos, state *s){    
    this->insert(pos, s);
    ++s->nPtr;
  }
  void del (){
    for (stateStack::iterator st=this->begin(); st != this->end(); ++st){
      --(*st)->nPtr;
      if ((*st)->nPtr==0){
	delete *st;
      }
    }
  }
};  
struct ltstatestack
{
  bool operator()(const stateStack & s1, const stateStack & s2) const
  //bool operator()(const stateStack & s1, const stateStack & s2)
  {
    //if (s2.size()==0){return true;}
    //else if (s1.size()==0){return false;}
    //else{
    //    return ( **(s1.begin()) < **(s2.begin()) );
    if ((**(s1.begin())).id == (**(s2.begin())).id){return false;}
    else{
      return ( ((*s1.begin())->fcst < (*s2.begin())->fcst) || ((*s1.begin())->fcst == (*s2.begin())->fcst && (*s1.begin())->cst < (*s2.begin())->cst ) || ((*s1.begin())->fcst == (*s2.begin())->fcst && (*s1.begin())->cst == (*s2.begin())->cst && (*s1.begin())->id < (*s2.begin())->id));
    }
	     //    return ( (**(s1.begin())).fcst < (**(s2.begin())).fcst );
  }
};
#endif

#ifndef STATELIST
#define STATELIST
class stateList: public list< state* > {
 public:
  ~stateList(){
    for (stateList::iterator st=this->begin(); st != this->end(); ++st){
      delete *st;
    }
  }
  string print ( const wlinkSequence & posOrderPosslnks, const param & pars) const;
};  
#endif

#ifndef HASHVECLINKINDEX
#define HASHVECLINKINDEX
                                                                                    
class hashfveclinkindex{
 public:
  size_t operator()(const vector<linkIndex> & v) const{
    unsigned int res=0;
    for (vector<linkIndex>::const_iterator it=v.begin();it!=v.end();++it){
      res += (size_t) pow((double)2 , double((*it%6)+1) );
    }
    return res%STMODUL;
  }
};

class hasheqveclinkindex{
 public:
  bool operator()(const vector<linkIndex> & v1, const vector<linkIndex> & v2) const{
    return v1==v2;
  }
};

#endif 
#ifndef HASHVECINT
#define HASHVECINT
                                                                                    
class hashfvecint{
 public:
  size_t operator()(const vector<int> & v) const{
    unsigned int res=0;
    for (vector<int>::size_type i=0; i<v.size();++i){
      if (v[i]>0) res += (size_t) pow((double)2 , double((i%6)+1) );
    }
    return res%STMODUL;
  }
};

class hasheqvecint{
 public:
  bool operator()(const vector<int> & v1, const vector<int> & v2) const{
    return v1==v2;
  }
};

#endif 


#ifndef NBESTLIST
#define NBESTLIST

class nbestList {
  hash_map<vector<int>,state*,hashfvecint,hasheqvecint> nbjash;
 public:
  stateStack nbstack;
  void cleverInsert(state *st, linkIndex nposlinks);
  void cleverInsert(const nbestList & nb, linkIndex nposlinks);
  void clear(linkIndex nposlink);
  bool empty() const {return nbstack.empty();}
  string nbest (const wlinkSequence & posslnks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars, int numbest, int numSentPair ) const{
    return nbstack.nbest(posslnks,srcLinksVec,trgLinksVec,direction,pars,numbest,numSentPair);
  }
  string onebest (const wlinkSequence & posslnks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars ) const{
    return nbstack.onebest(posslnks,srcLinksVec,trgLinksVec,direction,pars);
  }
  // not used any more
  string nbest ( int numbest, const wlinkCoverage & posiOrderLinkCoverage, const phraseLinkTable & existingLinks, int level) const{
    return nbstack.nbest(numbest,posiOrderLinkCoverage,existingLinks,level);
  }
  string print(const wlinkSequence & posslnks, const param & pars){
    //cout<<"nblist-display:"<<*nbstack.begin()<<endl;
    return nbstack.print(posslnks,pars);
  }
};  
#endif

#ifndef HASHPAIRVECINTASBOOL
#define HASHPAIRVECINTASBOOL
// take pair<vector<int>,vector<int> > as input, but two vectors are equal if for the same indices i, v[i]>0 and v[i]=0, as if it were bools
class hashfpairvecintasbool : public unary_function<pair<vector<int>,vector<int> >, size_t>{
public:
  size_t operator()(pair<vector<int>,vector<int> > const &p) const{
    unsigned int res=0;
    int i=0;
    for (vector<int>::const_iterator it=p.first.begin(); it!=p.first.end();++it){
      if (*it>0) res += (size_t) pow((double)2 , double((i%6)+1) );
      ++i;
    }
    for (vector<int>::const_iterator it=p.second.begin(); it!=p.second.end();++it){
      if (*it>0) res += (size_t) pow((double)2 , double((i%6)+1) );
      ++i;
    }
    return res%STMODUL;
  }
};

class hasheqpairvecintasbool{
public:
  bool operator()(pair<vector<int>,vector<int> >const &p1, pair<vector<int>,vector<int> > const &p2) const{
    if ( (p1.first.size()!= p2.first.size()) || (p1.second.size() != p2.second.size()) ){
      return false;
    }else{
      // vectors of pairs 1 and 2 have same size
      // if some (boolean ie v[i]=0 or >0) entry is different between pair 1 and 2, return false
      for (vector<int>::size_type i=0; i<p1.first.size();++i){
	if ( (p1.first.at(i)>0 && p2.first.at(i)==0) || (p1.first.at(i)==0 && p2.first.at(i)>0) ){return false;}
      }
      for (vector<int>::size_type i=0; i<p1.second.size();++i){
	if ( (p1.second.at(i)>0 && p2.second.at(i)==0) || (p1.second.at(i)==0 && p2.second.at(i)>0) ){return false;}
      }
    }
    return true;
  }
};
#endif

#ifndef HASHVECINTASBOOL
#define HASHVECINTASBOOL
// take vector<int> as input, but two vectors are equal if for the same indices i, v[i]>0 and v[i]=0, as if it were bools
class hashfvecintasbool : public unary_function<vector<int>, size_t>{
public:
  size_t operator()(const vector<int> & cov) const{
    unsigned int res=0;
    int i=0;
    for (vector<int>::const_iterator it=cov.begin(); it!=cov.end();++it){
      if (*it>0) res += (size_t) pow((double)2 , double((i%6)+1) );
      ++i;
    }
    return res%STMODUL;
  }
};

class hasheqvecintasbool{
public:
  bool operator()(const vector<int> & cov1, const vector<int> & cov2) const{
    if ( cov1.size()!= cov2.size() ){
      return false;
    }else{
      // vectors of pairs 1 and 2 have same size
      // if some (boolean ie v[i]=0 or >0) entry is different between pair 1 and 2, return false
      for (vector<int>::size_type i=0; i<cov1.size();++i){
	if ( (cov1.at(i)>0 && cov2.at(i)==0) || (cov1.at(i)==0 && cov2.at(i)>0) ){return false;}
      }
      for (vector<int>::size_type i=0; i<cov1.size();++i){
	if ( (cov1.at(i)>0 && cov2.at(i)==0) || (cov1.at(i)==0 && cov2.at(i)>0) ){return false;}
      }
    }
    return true;
  }
};
#endif

#ifndef STATECOVERAGE
#define STATECOVERAGE

class stateCoverage {
  hash_map<pair<vector<int>,vector<int> >, unsigned long long int, hashfpairvecintasbool,hasheqpairvecintasbool> wordCoverageMap;
  vector<pair<vector<int>,vector<int> > > wordCoverageVoc;
  hash_map<vector<int>, unsigned long long int, hashfvecintasbool,hasheqvecintasbool> monolWordCoverageMap;
  vector<vector<int> > monolWordCoverageVoc;
 public:
  stateCoverage(){};
  unsigned long long int getWordCoverageID(const vector<int> & scov, const vector<int> & tcov);
  string printWordCoverage(unsigned long long int id) const;
  unsigned long long int getMonolWordCoverageID(const vector<int> & cov);
  string printMonolWordCoverage(unsigned long long int id) const;
  void clear(){
    wordCoverageMap.clear();
    wordCoverageVoc.clear();
    monolWordCoverageMap.clear();
    monolWordCoverageVoc.clear();
  };
  unsigned int size() const {
    return wordCoverageVoc.size();
  }
  unsigned int monolSize() const {
    return monolWordCoverageVoc.size();
  }
};  
#endif

#ifndef MULTIPLESTACKS
#define MULTIPLESTACKS
class multipleStacks{
 public:
  unsigned long long int subStackIndex(const param & pars, bool sourceWordParse, state* _state, stateCoverage & stateCov) const;
};
#endif
