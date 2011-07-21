#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include "defs.h"
#include "math.h"
#include "sentPair.h"
#include "confusionNet.h"

using namespace std;

#ifndef WLINK
#define WLINK

class wlink
{
  int _sbeg,_send,_tbeg,_tend;
 public:
  float trgRankForSrc,srcRankForTrg;
  float match,stem,syn;
  linkIndex id;
  vector<cost> costs;
  cost cst;
  cost pofsCst;
  wlink ( int sbeg, int send, int tbeg, int tend, linkIndex _id, const vector<cost> & costs, const param & pars, float tRkForS, float sRkForT, float _match, float _stem, float _syn);
  wlink ( int sbeg, int send, int tbeg, int tend, linkIndex _id);
  wlink ( int s, int t , linkIndex _id);
  wlink ( int s, int t);
  int sbeg() const; 
  int send() const; 
  int tbeg() const; 
  int tend() const; 
  void setPofsCost (cost _pofsCst);
  bool operator< (const wlink & y) const;
  bool operator> (const wlink & y) const;
  bool operator== (const wlink & y) const;
  string print () const;
  string printVoc (const sentPair & sp, int level=0) const;
  string printDebug (const sentPair & sp) const;
  string printExtended () const;
  string printTalp () const;
  string printTalpDebug () const;
  string printBlinker () const;
};
struct ltwlink
{
  bool operator()(const wlink l1, const wlink l2) const
  {
    return l1.cst<l2.cst || (l1.cst==l2.cst && l1.id<l2.id);
  }
};
struct ltpairst
{
  bool operator()(const pair<int,int> l1, const pair<int,int> l2) const
  {
    return l1.first<l2.first || (l1.first==l2.first && l1.second<l2.second);
  }
};
struct ltwlinkst
{
  bool operator()(const wlink l1, const wlink l2) const
  {
    return l1.sbeg()<l2.sbeg() || (l1.sbeg()==l2.sbeg() && l1.cst<l2.cst) || (l1.sbeg()==l2.sbeg() && l1.cst==l2.cst && l1.id<l2.id);
  }
};
struct ltwlinkts
{
  bool operator()(const wlink l1, const wlink l2) const
  {
    return l1.tbeg()<l2.tbeg() || (l1.tbeg()==l2.tbeg() && l1.cst<l2.cst) || (l1.tbeg()==l2.tbeg() && l1.cst==l2.cst && l1.id<l2.id);
  }
};
struct ltwlinkid
{
  bool operator()(const wlink l1, const wlink l2) const
  {
    return l1.id<l2.id;
  }
};
struct ltwlinkidts
{
  bool operator()(const wlink l1, const wlink l2) const
  {
    return l1.tbeg()<l2.tbeg() || (l1.tbeg()==l2.tbeg() && l1.sbeg()<l2.sbeg());
  }
};

#endif

#ifndef HASHLNKLEV
#define HASHLNKLEV

class hashflnklev : public unary_function<pair<wlink,int>, size_t>{
 public:
  size_t operator()(pair<wlink,int> const &p) const{
    int res=0;
    res += (size_t) p.second;
    res += (size_t) pow((double)p.first.sbeg(),(double) 2);
    res += (size_t) pow((double)p.first.tbeg(),(double) 3);
    //cout<<"res:"<<res%10000<<endl;
    return res%10000; 
  }
};

class hasheqlnklev{
 public:
  bool operator()(pair<wlink,int> const &p1, pair<wlink,int> const &p2) const{
    return p1.first.sbeg()==p2.first.sbeg() && p1.first.tbeg()==p2.first.tbeg() && p1.second==p2.second;
  }
};

#endif

#ifndef HASHINTINT
#define HASHINTINT

class hashfintint : public unary_function<pair<int,int>, size_t>{
 public:
  size_t operator()(pair<int,int> const &p) const{
    wordIndex res=0;
    res += (size_t) p.first;
    res += (size_t) pow((double)p.second,double(2));
    return res%200;
  }
};
                                                                                    
class hasheqintint{
 public:
  bool operator()(pair<int,int> const &p1, pair<int,int> const &p2) const{
    return p1==p2;
  }
};

#endif

#ifndef WLINKSEQUENCE
#define WLINKSEQUENCE

class wlinkSequence: public vector<wlink>
{
 public:
  void put ( const wlink & lnk){
    push_back(lnk);
  }
  void insert ( const wlink & lnk){
    push_back(lnk);
  }
  string display (const sentPair & sp, int level=0, bool showvoc=false) const;
};

#endif

#ifndef WLINKSEQUENCEVEC
#define WLINKSEQUENCEVEC

class wlinkSequencePtrVec: public vector<wlinkSequence*>
{
 public:
  wlinkSequencePtrVec(int nlev){
    for (int i=0; i<nlev; ++i){
      wlinkSequence *seq = new wlinkSequence;
      this->push_back(seq);
    }
  }
  ~wlinkSequencePtrVec(){
    for (wlinkSequencePtrVec::iterator st=this->begin(); st != this->end(); ++st){
      delete *st;
    }
  }  
  string display (const sentPair & sp, bool showvoc=false) const{
    ostringstream oss("");
    int lev=0;
    for (wlinkSequencePtrVec::const_iterator v=this->begin(); v!=this->end();++v){
      oss<<"level "<<lev<<":\n"<<(*v)->display(sp,lev,showvoc)<<endl;
      ++lev;
    }
    return oss.str();
  }
};
class wlinkSequenceVec: public vector<wlinkSequence>
{
 public:
  wlinkSequenceVec(){}
  wlinkSequenceVec(int nlev){
    for (int i=0; i<nlev; ++i){
      wlinkSequence seq;
      this->push_back(seq);
    }
  }
  void insert(const wlinkSequence & lseq){
    push_back(lseq);
  }
  string printTalpDebug () const{
    ostringstream oss("");
    int lev=0;
    for (wlinkSequenceVec::const_iterator v=this->begin(); v!=this->end();++v){
      oss<<lev<<":";
      for (wlinkSequence::const_iterator seq=v->begin();seq!=v->end();++seq){
	oss<<seq->printTalpDebug()<<" ";
      }
      oss<<endl;
      ++lev;
    }
    return oss.str();
  }
};
#endif

#ifndef WEIGHTEDLINKSEQUENCE
#define WEIGHTEDLINKSEQUENCE

class weightedLinkSequence: public wlinkSequence
{
  bool set;
 public:
  cost cst;
  weightedLinkSequence(){
    cst=100;
    set=false;
  }
  weightedLinkSequence( const wlink & lnk ){
    push_back(lnk);
    cst=lnk.cst;
  }
  void put ( const wlink & lnk){
    push_back(lnk);
    cst=( cst*(this->size()-1) + lnk.cst )/this->size();
  }
  bool isSet() const {return set;}
  void setCst(cost _cst){
    cst=_cst;
    set=true;
  }
  string display (const sentPair & sp, int level=0, bool showvoc=false) const;
};

/*
struct ltweightedlinkseq
{
  bool operator()(const weightedLinkSequence l1, const weightedLinkSequence l2) const
  {
    if (l1.cst<l2.cst || (l1.cst==l2.cst && l1.size() < l2.size())){
      return true;
    }else if (l1.cst==l2.cst && l1.size() == l2.size()){
      for (weightedLinkSequence::size_type n=0;n<l1.size();++n){
	return  (l1[n].id<l2[n].id);
      }
    }else{
      return false;
    }
  }
  };*/

struct ltweightedlinkseq
{
  bool operator()(const weightedLinkSequence l1, const weightedLinkSequence l2) const
  {
    if (l1.cst<l2.cst || (l1.cst==l2.cst && l1.size() < l2.size())){
      return true;
    }else if (l1.cst==l2.cst && l1.size() == l2.size()){
      for (weightedLinkSequence::size_type n=0;n<l1.size();++n){
	return  (abs(l1[n].sbeg()-l1[n].tbeg())<abs(l2[n].sbeg()-l2[n].tbeg()) || abs(l1[n].sbeg()-l1[n].tbeg())==abs(l2[n].sbeg()-l2[n].tbeg()) && l1[n].id<l2[n].id);
      }
    }else{
      return false;
    }
  }
 };
/*
struct ltweightedlinkseq
{
  bool operator()(const weightedLinkSequence l1, const weightedLinkSequence l2) const
  {
    if (l1.cst<l2.cst){
      return true;
    }else if (l1.cst==l2.cst){
      for (weightedLinkSequence::size_type n=0;n<l1.size();++n){
	return  (abs(l1[n]._sbeg-l1[n].tbeg())<abs(l2[n]._sbeg-l2[n].tbeg()) || abs(l1[n]._sbeg-l1[n].tbeg())==abs(l2[n]._sbeg-l2[n].tbeg()) && l1[n].id<l2[n].id);
      }
    }else{
      return false;
    }
  }
};
  */

#endif

#ifndef WLINKCOMBINATIONS
#define WLINKCOMBINATIONS
template <class T>
void combinations_recursive(
			    const wlinkSequence &elems, //vector of elements to be combined
			    T & combs, //output for combinations
			    string direc, // are we parsing srcLinks or trgLinks?
			    unsigned int req_len, // number of elements in combinations
			    vector<unsigned int> &pos, //vector of size req_len that will hold the positions or indexes of the chosen elements
			    unsigned int depth, //how many elements we have chosen so far, that is, the recursion depth
			    unsigned int margin, //first position to start looking for a new element (the left "margin")
			    const vector<bool> &compulsoryPos=vector<bool>(), //vector with true for positions of elements that are IMPLICIT in all combinations (they must actually be present but don't appear in the 'combs' output container
			    int nCompulsory=0
			    )
{
  //if (req_len==3) cout<<"combinations of length "<<req_len<<": so far we have chosen "<<depth<<" elements\n";
  // Have we selected the requested number of elements?
  if (depth >= req_len) {
    weightedLinkSequence weilnkseq;
    int holeSum=1;
    int last;
    if (direc=="st") last=elems[pos[0]].tbeg();
    else last=elems[pos[0]].sbeg();
    int nCompulsoryChosen=0;
    for (unsigned int ii = 0; ii < pos.size(); ++ii){
      unsigned int current;
      if (direc=="st") current=elems[pos[ii]].tbeg();
      else current=elems[pos[ii]].sbeg();
      holeSum+=current-last-1;
      //if (req_len==3) cout<<"pos:"<<pos[ii]<<" current:"<<current<<" last:"<<last<<" holesum:"<<holeSum<<endl<<" ";
      if (holeSum>1) return;
      if (nCompulsory>0 && compulsoryPos[pos[ii]]) ++nCompulsoryChosen;
      else weilnkseq.put(elems[pos[ii]]);
      last=current;
      //if (req_len==3) cout<<elems[pos[ii]].printTalpDebug()<<" ";
    }
    if (nCompulsoryChosen==nCompulsory && weilnkseq.size()>0){
      combs.insert(weilnkseq);
    }
    //if (req_len==3) cout << endl;
    return;
  }
  // Are there enough remaining elements to be selected?
  // This test isn't required for the function to be
  // correct, but it saves futile calls.
  if ((elems.size() - margin) < (req_len - depth))
    return;

  // Try to select new elements to the right of the last
  // selected one.
  for (unsigned int ii = margin; ii < elems.size(); ++ii) {
    pos[depth] = ii;
    //if (req_len==3) cout<<"call for combinations_recursive(elems,combs,"<<req_len<<",pos,"<<(depth+1)<<","<<(ii+1)<<")\n";
    combinations_recursive(elems,combs, direc, req_len, pos, depth + 1, ii + 1,compulsoryPos,nCompulsory);
  }
  return;
}

/*struct wlinkCombinations {
  public:
  void combinations_recursive(const wlinkSequence & elems, set<weightedLinkSequence,ltweightedlinkseq> & combSet, string direc, unsigned int req_len, vector<unsigned int> &pos, unsigned int depth, unsigned int margin );
  };
*/
#endif

#ifndef SORTEDLINKSEQUENCE
#define SORTEDLINKSEQUENCE
class sortedLinkSequence: public pair<int, set <wlink,ltwlink> >
{
 public:
  string display () const;
};

#endif

#ifndef WORDLINKSEQUENCE
#define WORDLINKSEQUENCE

class wordLinkSequence: public vector<sortedLinkSequence>
{
 public:
  wordLinkSequence(){}
  wordLinkSequence(size_type N){init(N);}
  void init(size_type N){
    for (size_type i=0; i<N;++i){
      sortedLinkSequence myset;
      this->push_back(myset);
    }
  }
  string display (const sentPair & sp, int level=0, bool showvoc=false) const;
};

class wordLinkSequenceVec: public vector<wordLinkSequence>
{
 public:
  wordLinkSequenceVec(int nlev){
    for (int i=0; i<nlev; ++i){
      wordLinkSequence seq;
      this->push_back(seq);
    }
  }
  void initLevel(size_type lev, size_type N){
    for (size_type i=0; i<N;++i){
      //pair<int, set<wlink,ltwlink> > myset;
      sortedLinkSequence myset;
      (*this)[lev].push_back(myset);
    }
  }
  string display (const sentPair & sp, bool showvoc) const;  
};

#endif

#ifndef PHRASELINKTABLE
#define PHRASELINKTABLE
class phraseLinkTable {
 public:
  hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev> jash_phLink;
  ~phraseLinkTable(){
    for (hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator st=jash_phLink.begin(); st != jash_phLink.end(); ++st){
      delete st->second;
    }
  }  
  bool put (const wlink & phlnk,int lev, const wlink & lnk);
  string printChilds (const wlink & lnk, int level) const ;
  string display(const sentPair & sp) const;
};
#endif

#ifndef LINKSTOEXPAND
#define LINKSTOEXPAND
class linksToExpand: public vector< pair<int,set<weightedLinkSequence,ltweightedlinkseq>* > >
{
 public:
  ~linksToExpand(){
    for (linksToExpand::iterator st=this->begin(); st != this->end(); ++st){
      delete (*st).second;
    }
  }
  linksToExpand(){}
  linksToExpand(int n){
    for (int i=0; i<n;++i){
      set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
      this->push_back(make_pair(0,wlseqSet));
    }
  }
  string buildList(string search, string direction, const wlinkSequence & posiOrderPossLinks,const wordLinkSequence & srcLinks,const wordLinkSequence & trgLinks,const param & pars);
  string display (const sentPair & sp, int level=0, bool showvoc=false) const;
  unsigned int maxLinkSeqSize () const;
};
#endif

#ifndef FUTURECOSTESTIMATION
#define FUTURECOSTESTIMATION
class futureCostEstimation
{
  // srcposi-> (first,last) positions of best target link sequence for this src position
  typedef hash_map<int,pair<int,int> > bestLinkPosiMap;
  bestLinkPosiMap bestLinkPosi; 
  int _nPosis;
  bool _parsingSource;
  bool status;
 public:
  futureCostEstimation(){
    _nPosis=0;
    status=false;
  };
  string loadBestLinks(const wlinkSequence & posiOrderPossLinks, const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, param & pars, const sentPair & sp, string _direction, int level);
  string printBestLinks() const;
  void enable() {status=true;}
  void disable() {status=false;}
  bool enabled() const {return status;}
  int calculateDistortion(const wlink & lnk, const vector<int> & scov, const vector<int> & tcov, vector<int> & fcHist) const;
  bool parsingSource() const {return _parsingSource;}
};
#endif


#ifndef WLINKCOVERAGE
#define WLINKCOVERAGE

class wlinkCoverage
{
  set<pair<int,int>,ltpairst> posiOrderSet;
  hash_map<pair<int,int>, linkIndex, hashfintint, hasheqintint> coverageToIndexMap;
  vector<pair<int,int> > posiOrderVector;
  hash_map<pair<int,int>,vector<linkIndex>,hashfintint,hasheqintint> pbLinksForSrcTrgPair; // IDs of phrase-based links containing a (src,trg) pair
 public:
  wlinkCoverage(){};
  void addLink(int src, int trg){
    pair<int,int> p(src,trg);
    posiOrderSet.insert(p);
  }
  void buildIndex();
  vector<pair<int,int> >::size_type size() const {return posiOrderVector.size();}
  bool getIDifExists(int s, int t, linkIndex & id) const;
  linkIndex getID(int s, int t) const;
  int src(linkIndex id) const {return posiOrderVector[id].first;}
  int trg(linkIndex id) const {return posiOrderVector[id].second;}
  void addAssociatedPbLinks(const wlinkSequence & possLinks);
  string printAssociatedPbLinks() const;
  bool getAssociatedPbLinks(int s, int t, vector<linkIndex> & vec) const;
};

#endif

