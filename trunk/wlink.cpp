#include "wlink.h"

//***
// WLINK CLASS
//***

// TODO: implement these 4 functions which give the position of begin-end of source and target phrases. 
// Problem: wlink.source (or target) is the position of the uniqAlUnit is sp.uniqAlUnitCollection. In order to access the uniqAlUnit, we need to access the sp object. So shouldn't contain a wlink the pointers to 2 uniqAlUnits + the costs of the link ?
//other solution: include redondantly in the wlink the begin, end, etc without access to the uniqAlUnit
int wlink::sbeg() const{
  return _sbeg;
}
int wlink::tbeg() const{
  return _tbeg;
}
int wlink::send() const{
  return _send;
}
int wlink::tend() const{
  return _tend;
}

string wlink::print () const{
  std::ostringstream oss("");
  oss<<id<<":("<<_sbeg;
  if (_send>_sbeg){oss<<","<<_send;}
  oss<<"-"<<_tbeg;
  if (_tend>_tbeg){oss<<","<<_tend;}
  oss<<"){"<<trgRankForSrc<<"-"<<srcRankForTrg<<"}:"<<cst<<"[";
  for (vector<cost>::const_iterator c=costs.begin(); c != costs.end();++c){oss<<*c<<",";}
  oss<<pofsCst<<"]";
  return oss.str();
}
string wlink::printVoc (const sentPair & sp, int level) const{
  std::ostringstream oss("");
  if (level==0){
    oss<<id<<":("<<_sbeg;
    if (_send>_sbeg){oss<<","<<_send;}
    oss<<"|";
    for (int posi=_sbeg;posi<=_send;++posi){
      iwordCnColumn col=sp.src().at(posi);
      if (posi>_sbeg){oss<<" ";}
      for (iwordCnColumn::const_iterator cit=col.begin(); cit!=col.end(); ++cit){
	if (cit!=col.begin()){oss<<"_";}
	oss<<cit->first.getStform(level);
      }
    }
    oss<<"-"<<_tbeg;
    if (_tend>_tbeg){oss<<","<<_tend;}
    oss<<"|";
    for (int posi=_tbeg;posi<=_tend;++posi){
      iwordCnColumn col=sp.trg().at(posi);
      if (posi>_tbeg){oss<<" ";}
      for (iwordCnColumn::const_iterator cit=col.begin(); cit!=col.end(); ++cit){
	if (cit!=col.begin()){oss<<"_";}
	oss<<cit->first.getStform(level);
      }
    }
    oss<<"){"<<trgRankForSrc<<"-"<<srcRankForTrg<<";"<<match<<"-"<<stem<<"-"<<syn<<"}:"<<cst<<"[";
  }else{
    oss<<id<<":("<<_sbeg<<"|"<<sp.srcat(sp.srcWdPosis(level,_sbeg).at(0)).getStform(level)<<","<<_tbeg<<"|"<<sp.trgat(sp.trgWdPosis(level,_tbeg).at(0)).getStform(level)<<"):"<<cst<<"[";
  }
  for (vector<cost>::const_iterator c=costs.begin(); c != costs.end();++c){oss<<*c<<",";}
  oss<<pofsCst<<"]";
  return oss.str();
}
string wlink::printDebug (const sentPair & sp) const{
  std::ostringstream oss("");
  oss<<id<<":("<<_sbeg<<"|"<<sp.srcat(_sbeg).print()<<","<<_tbeg<<"|"<<sp.trgat(_tbeg).print()<<"):"<<cst<<"[";
  for (vector<cost>::const_iterator c=costs.begin(); c != costs.end();++c){oss<<*c<<",";}
  oss<<pofsCst<<"]";
  return oss.str();
}
string wlink::printBlinker () const{
  std::ostringstream oss("");
  oss<<(_sbeg+1)<<" "<<(_tbeg+1)<<"\n";	
  return oss.str();
}
string wlink::printTalp () const{
  std::ostringstream oss("");
  oss<<(_sbeg+1)<<"-"<<(_tbeg+1);	
  return oss.str();
}
string wlink::printExtended () const{
  std::ostringstream oss("");
  int cnt=0;
  for (int sposi=_sbeg; sposi<=_send; ++sposi){
    for (int tposi=_tbeg; tposi<=_tend; ++tposi){
      if (cnt>0){oss<<" ";}
      oss<<sposi<<"-"<<tposi;
      ++cnt;
    }
  }
  return oss.str();
}

string wlink::printTalpDebug () const{
  std::ostringstream oss("");
  oss<<_sbeg;
  if (_send>_sbeg){oss<<","<<_send;}
  oss<<"-"<<_tbeg;	
  if (_tend>_tbeg){oss<<","<<_tend;}
  return oss.str();
}

/*bool wlink::operator< (const wlink & y) const {
	return _sbeg<y._sbeg || (!(y._sbeg < _sbeg) && _tbeg< y._tbeg);
}
bool wlink::operator== (const wlink & y) const {
	return _sbeg==y._sbeg && _tbeg == y._tbeg;
}
bool wlink::operator> (const wlink & y) const {
	return _sbeg>y._sbeg || (!(y._sbeg > _sbeg) && _tbeg> y._tbeg);
}
*/
bool wlink::operator< (const wlink & y) const {
  return cst<y.cst;
}
bool wlink::operator== (const wlink & y) const {
  return cst==y.cst;
}
bool wlink::operator> (const wlink & y) const {
  return cst>y.cst;
}

wlink::wlink( int sbeg, int send, int tbeg, int tend, linkIndex _id, const vector<cost> & _costs, const param & pars, float tRkForS, float sRkForT, float _match, float _stem, float _syn ) : _sbeg(sbeg), _send(send), _tbeg(tbeg), _tend(tend), id(_id), costs(_costs),trgRankForSrc(tRkForS),srcRankForTrg(sRkForT), match(_match), stem(_stem), syn(_syn) {
  pofsCst=0;
  cst=0;
  //  for (vector<cost>::const_iterator v=costs.begin(); v != costs.end(); ++v){
  // cst+=*v;
  // }
  if (pars.wwda1[0]!=0){
    cst+=pars.wwda1[0] * costs[0];
  }
  if (pars.wwda2!=0){
    cst+=pars.wwda2 * costs[1];
  }
  if (pars.wwda3!=0){
    cst+=pars.wwda3 * costs[2];
  }
  if (pars.wwda4!=0){
    cst+=pars.wwda4 * costs[3];
  }
  if (pars.wwda5!=0){
    cst+=pars.wwda5 * costs[4];
  }
  if (pars.wwda6!=0){
    cst+=pars.wwda6 * costs[5];
  }
  if (pars.wrk!=0){
    cst+=pars.wrk * (trgRankForSrc+srcRankForTrg);
  }
  if (pars.wrks!=0){
    cst+=pars.wrks * trgRankForSrc;
  }
  if (pars.wrkt!=0){
    cst+=pars.wrkt * srcRankForTrg;
  }
  if (pars.wrkb!=0){
    if (trgRankForSrc > srcRankForTrg){
      cst+=pars.wrkb * srcRankForTrg;
    }else{
      cst+=pars.wrkb * trgRankForSrc;
    }
  }
  if (pars.wrkw!=0){
    if (trgRankForSrc > srcRankForTrg){
      cst+=pars.wrkw * trgRankForSrc;
    }else{
      cst+=pars.wrkw * srcRankForTrg;
    }
  }
  if (pars.wmatchb!=0){
    cst-=pars.wmatchb * match;
  }
  if (pars.wstemb!=0){
    cst-=pars.wstemb * stem;
  }
  if (pars.wsynb!=0){
    cst-=pars.wsynb * syn;
  }
}
wlink::wlink( int s, int t, linkIndex _id ) : _sbeg(s), _tbeg(t), id(_id) {
  pofsCst=0;
  cst=0;
}
wlink::wlink( int sbeg, int send, int tbeg, int tend, linkIndex _id ) : _sbeg(sbeg), _send(send), _tbeg(tbeg), _tend(tend), id(_id) {
  pofsCst=0;
  cst=0;
}
wlink::wlink( int s, int t) : _sbeg(s), _tbeg(t) {
  pofsCst=0;
  cst=0;
  id=-1;
}

void wlink::setPofsCost (cost _pofsCst){
  pofsCst=_pofsCst;
}

/*
void wlinkCombinations::combinations_recursive(
					   const wlinkSequence &elems, //vector of elements to be combined
					   set<weightedLinkSequence,ltweightedlinkseq> & combSet, //output for combinations
					   string direc, // are we parsing srcLinks or trgLinks?
					   unsigned int req_len, // number of elements in combinations
					   vector<unsigned int> &pos, //vector of size req_len that will hold the positions or indexes of the chosen elements
					   unsigned int depth, //how many elements we have chosen so far, that is, the recursion depth
					   unsigned int margin //first position to start looking for a new element (the left "margin")
					   )
{
  //cout<<"combinations of length "<<req_len<<": so far we have chosen "<<depth<<" elements\n";
  // Have we selected the requested number of elements?
  if (depth >= req_len) {
    weightedLinkSequence weilnkseq;
    int holeSum=1;
    int last;
    if (direc=="st") last=elems[pos[0]]._tbeg;
    else last=elems[pos[0]]._sbeg;
    for (unsigned int ii = 0; ii < pos.size(); ++ii){
      int current;
      if (direc=="st") current=elems[pos[ii]]._tbeg;
      else current=elems[pos[ii]]._sbeg;
      holeSum+=current-last-1;
      if (holeSum>1) return;
      weilnkseq.put(elems[pos[ii]]);
      last=current;
      //cout << elems[pos[ii]];
    }
    combSet.insert(weilnkseq);
    //cout << endl;
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
    //cout<<"call for combinations_recursive(elems,combSet,"<<req_len<<",pos,"<<(depth+1)<<","<<(ii+1)<<")\n";
    combinations_recursive(elems,combSet, direc, req_len, pos, depth + 1, ii + 1);
  }
  return;
}
*/
string wlinkSequence::display (const sentPair & sp, int level,bool showvoc) const {
  ostringstream oss("");
  for (wlinkSequence::const_iterator l=this->begin(); l != this->end(); ++l){
    if (showvoc) oss<<"\t"<<l->printVoc(sp,level)<<"\n";	
    else oss<<l->print()<<" ";	
  }
  oss<<endl;
  return oss.str();
}
string weightedLinkSequence::display (const sentPair & sp, int level, bool showvoc) const {
  ostringstream oss("");
  oss<<"cost:"<<cst<<"- ";
  for (wlinkSequence::const_iterator l=this->begin(); l != this->end(); ++l){
    if (showvoc) oss<<l->printVoc(sp,level)<<" ";	
    else oss<<l->print()<<" ";	
  }
  return oss.str();
}
string sortedLinkSequence::display() const {
  ostringstream oss("");
  for (set<wlink,ltwlink>::const_iterator l=second.begin(); l!=second.end();++l){
    oss<<l->print()<<" ";
  }
  oss<<endl;
  return oss.str();
}
string wordLinkSequence::display (const sentPair & sp, int level, bool showvoc) const{
  ostringstream oss("");
  int cnt=0;
  for (wordLinkSequence::const_iterator s=this->begin(); s!= this->end();++s){
    oss<<cnt<<" ("<<s->second.size()<<" iter:"<<s->first<<"):\n";
    for (set<wlink>::const_iterator l=s->second.begin(); l!=s->second.end();++l){
      if (showvoc) oss<<"\t"<<l->printVoc(sp,level)<<"\n";
      else oss<<l->print()<<" ";
    }
    oss<<endl;
    ++cnt;
  }
  return oss.str();
}
string wordLinkSequenceVec::display (const sentPair & sp, bool showvoc) const{
  ostringstream oss("");
  int level=0;
  for (wordLinkSequenceVec::const_iterator v=this->begin(); v!=this->end();++v){
    oss<<"level "<<level<<":\n";
    oss<<v->display(sp,level,showvoc);
    ++level;
  }
  oss<<endl;
  return oss.str();
}

string linksToExpand::buildList(string search, string direction, const wlinkSequence & possLinks,const wordLinkSequence & srcLinks,const wordLinkSequence & trgLinks,const param & pars){
  ostringstream oss("");
  if (search=="word"){
    //wlinkCombinations combs;
    if (direction=="st"){
      for (int n=0; n<srcLinks.size();++n){
	set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
	wlinkSequence srclnks;
	for (set<wlink>::const_iterator lit=srcLinks[n].second.begin();lit!=srcLinks[n].second.end();++lit){
	  srclnks.push_back(possLinks[lit->id]);
	}
	stable_sort(srclnks.begin(),srclnks.end(),ltwlinkid());
	unsigned int nlinks=1;
	while (nlinks<=srclnks.size() && nlinks<=pars.m){
	  vector<unsigned int> pos(nlinks);
	  //combs.combinations_recursive(srclnks,*wlseqSet, direction, nlinks,pos,0,0);
	  // TODO: check that combined links are compatible (e.g. 1-2 cannot be combined with 0,1-2)
	  combinations_recursive<set<weightedLinkSequence,ltweightedlinkseq> >(srclnks,*wlseqSet, direction, nlinks,pos,0,0);
	  ++nlinks;
	}
	if (!wlseqSet->empty()){
	  // insert pair of (iteration at which it will be expanded, set)
	  this->push_back(make_pair(srcLinks[n].first,wlseqSet));
	}
      }
    }else if (direction=="ts"){
      for (int n=0; n<trgLinks.size();++n){
	set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
	wlinkSequence trglnks;
	for (set<wlink>::const_iterator lit=trgLinks[n].second.begin();lit!=trgLinks[n].second.end();++lit){
	  trglnks.push_back(possLinks[lit->id]);
	}
	stable_sort(trglnks.begin(),trglnks.end(),ltwlinkid());
	unsigned int nlinks=1;
	while (nlinks<=trglnks.size() && nlinks<=pars.m){
	  vector<unsigned int> pos(nlinks);
	  //combs.combinations_recursive(trglnks,*wlseqSet, direction, nlinks,pos,0,0);
	  // TODO: check that combined links are compatible (e.g. 1-2 cannot be combined with 0,1-2)
	  combinations_recursive<set<weightedLinkSequence,ltweightedlinkseq> >(trglnks,*wlseqSet, direction, nlinks,pos,0,0);
	  ++nlinks;
	}
	if (!wlseqSet->empty()){
	  // insert pair of (iteration at which it will be expanded, set)
	  this->push_back(make_pair(trgLinks[n].first,wlseqSet));
	}
      }
    }else if (direction=="at"){ //direction=a
      //collect best link of each word and sort them
      wlinkSequence tmpSeq;
      for (vector<sortedLinkSequence>::const_iterator it=trgLinks.begin();it!=trgLinks.end();++it){
	if (!it->second.empty()) tmpSeq.put(*(it->second.begin()));
      }
      stable_sort(tmpSeq.begin(),tmpSeq.end());
      
      //build stacks in this sorting order
      for (wlinkSequence::const_iterator sit=tmpSeq.begin();sit!=tmpSeq.end();++sit){
	set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
	wlinkSequence trglnks;
	for (set<wlink>::const_iterator lit=trgLinks[sit->tend()].second.begin();lit!=trgLinks[sit->tend()].second.end();++lit){
	  trglnks.push_back(possLinks[lit->id]);
	}
	stable_sort(trglnks.begin(),trglnks.end(),ltwlinkid());
	unsigned int nlinks=1;
	while (nlinks<=trglnks.size() && nlinks<=pars.m){
	  vector<unsigned int> pos(nlinks);
	  //combs.combinations_recursive(trglnks,*wlseqSet, "ts", nlinks,pos,0,0);
	  // TODO: check that combined links are compatible (e.g. 1-2 cannot be combined with 0,1-2)
	  combinations_recursive<set<weightedLinkSequence,ltweightedlinkseq> >(trglnks,*wlseqSet, "ts", nlinks,pos,0,0);
	  ++nlinks;
	}
	if (!wlseqSet->empty()){
	  // insert pair of (iteration at which it will be expanded, set)
	  this->push_back(make_pair(trgLinks[sit->tend()].first,wlseqSet));
	}
      }
    }else{ //direction="a" or "as"
      //collect best link of each word and sort them
      wlinkSequence tmpSeq;
      for (vector<sortedLinkSequence>::const_iterator it=srcLinks.begin();it!=srcLinks.end();++it){
	if (!it->second.empty()) tmpSeq.put(*(it->second.begin()));
      }
      stable_sort(tmpSeq.begin(),tmpSeq.end());
      
      //build stacks in this sorting order
      for (wlinkSequence::const_iterator sit=tmpSeq.begin();sit!=tmpSeq.end();++sit){
	//cerr<<"[buildlist] "<<sit->printTalpDebug()<<endl;
	set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
	wlinkSequence srclnks;
	for (set<wlink>::const_iterator lit=srcLinks[sit->send()].second.begin();lit!=srcLinks[sit->send()].second.end();++lit){
	  srclnks.push_back(possLinks[lit->id]);
	}
	stable_sort(srclnks.begin(),srclnks.end(),ltwlinkid());
	unsigned int nlinks=1;
	while (nlinks<=srclnks.size() && nlinks<=pars.m){
	  vector<unsigned int> pos(nlinks);
	  //combs.combinations_recursive(srclnks,*wlseqSet, "st", nlinks,pos,0,0);
	  // TODO: check that combined links are compatible (e.g. 1-2 cannot be combined with 0,1-2)
	  combinations_recursive<set<weightedLinkSequence,ltweightedlinkseq> >(srclnks,*wlseqSet, "st", nlinks,pos,0,0);
	  ++nlinks;
	}
	if (!wlseqSet->empty()){
	  // insert pair of (iteration at which it will be expanded, set)
	  this->push_back(make_pair(srcLinks[sit->send()].first,wlseqSet));
	}
      }
    } // if direction==a or as
  }else{ //if (search!="word")
    wlinkSequence possLnks=possLinks;
    if (direction=="st"){
      stable_sort(possLnks.begin(), possLnks.end(),ltwlinkst());
    }else if (direction=="ts"){
      stable_sort(possLnks.begin(), possLnks.end(),ltwlinkts());
    }else{
      stable_sort(possLnks.begin(), possLnks.end());
    }
    wlinkSequence::const_iterator possl=possLnks.begin();
    while ( possl != possLnks.end() ){
      set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
      weightedLinkSequence weilnkseq(*possl);
      wlseqSet->insert(weilnkseq);
      // insert pair of (iteration at which it will be expanded, set)
      this->push_back(make_pair(srcLinks[possl->send()].first,wlseqSet));
      ++possl;
    }
  }
  return oss.str();
}

string linksToExpand::display (const sentPair & sp, int level, bool showvoc) const {
  ostringstream oss("");
  int cnt=0;
  for (vector<pair<int,set<weightedLinkSequence,ltweightedlinkseq>* > >::const_iterator v=this->begin();v!=this->end();++v){
    oss<<cnt<<" (iter "<<v->first<<"):\n";
    for (set<weightedLinkSequence,ltweightedlinkseq>::const_iterator s=v->second->begin();s!=v->second->end();++s){
      oss<<"\t"<<s->display(sp,level,showvoc)<<"\n";
    }
    ++cnt;
  }
  return oss.str();
}

unsigned int linksToExpand::maxLinkSeqSize () const {
  unsigned int max=0;
  for (vector<pair<int,set<weightedLinkSequence,ltweightedlinkseq>* > >::const_iterator v=this->begin(); v!=this->end();++v){
    unsigned int cnt=0;
    for (set<weightedLinkSequence,ltweightedlinkseq>::const_iterator s=v->second->begin();s!=v->second->end();++s){++cnt;}
    if (cnt>max){max=cnt;}
  }
  return max;
}

bool phraseLinkTable::put (const wlink & phlnk,int lev, const wlink & lnk){
  hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator exlit=jash_phLink.find(make_pair(phlnk,lev));
  if (exlit == jash_phLink.end()){
    weightedLinkSequence* lnkseq = new weightedLinkSequence; 
    lnkseq->put(lnk);
    (*lnkseq)[0].id=0;
    jash_phLink.insert(make_pair(make_pair(phlnk,lev),lnkseq));
    return true;
  }else{
    exlit->second->put(lnk);
    (*exlit->second)[exlit->second->size()-1].id=exlit->second->size()-1;
    return false;
  }
}
string phraseLinkTable::printChilds (const wlink & phlnk, int lev) const {
  ostringstream oss("");
  //  if (lev==0){oss<<phlnk.printTalp();}
  if (lev==0){oss<<phlnk.printExtended();}
  else{
    hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::const_iterator exlit=jash_phLink.find(make_pair(phlnk,lev));
    if (exlit != jash_phLink.end()){
      for (wlinkSequence::const_iterator l=exlit->second->begin(); l!=exlit->second->end();++l){
	if (l!=exlit->second->begin()){oss<<" ";}
	oss<<printChilds(*l,lev-1);
      }
    }
  }
  return oss.str();
}
string phraseLinkTable::display(const sentPair & sp) const {
  ostringstream oss("");
  for (hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::const_iterator st=jash_phLink.begin(); st != jash_phLink.end(); ++st){
    oss<<st->first.first.printTalpDebug()<<" "<<st->first.second<<":"<<st->second->display(sp, st->first.second)<<endl;
  }
  return oss.str();

}

string futureCostEstimation::loadBestLinks(const wlinkSequence & posiOrderPossLinks, const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, param & pars, const sentPair & sp, string _direction, int level){
  // to do: for level>0
  ostringstream oss("");
  wordLinkSequenceVec fc_srcLinksVec=srcLinksVec;
  wordLinkSequenceVec fc_trgLinksVec=trgLinksVec;
  string direction,search;
  search="word";
  if (_direction=="as" || _direction=="st"){
    _parsingSource=true;
    float srcAve=posiOrderPossLinks.size()*1.0/fc_srcLinksVec[level].size();
    for (int n=0; n<fc_srcLinksVec[level].size();++n){
      if (fc_srcLinksVec[level][n].second.size()>srcAve){fc_srcLinksVec[level][n].first=1;}
      else{fc_srcLinksVec[level][n].first=0;}
    }
    direction="st";
  }else{
    _parsingSource=false;
    float trgAve=posiOrderPossLinks.size()*1.0/fc_trgLinksVec[level].size();
    for (sentence::size_type n=0; n<fc_trgLinksVec[level].size();++n){
      if (fc_trgLinksVec[level][sp.trgat(n).getPos(level)].second.size()>trgAve){fc_trgLinksVec[level][sp.trgat(n).getPos(level)].first=1;}
      else{fc_trgLinksVec[level][sp.trgat(n).getPos(level)].first=0;}
    }
    direction="ts";
  }
  linksToExpand fc_toExpandList;
  fc_toExpandList.buildList(search,direction, posiOrderPossLinks, fc_srcLinksVec[level],fc_trgLinksVec[level],pars);
  if (pars.verbose>1){
    oss<<"fc_to-expand list (level "<<level<<"):\n"<<fc_toExpandList.display(sp,level,1);
  }
  int count=0;
  for (linksToExpand::const_iterator lit=fc_toExpandList.begin();lit!=fc_toExpandList.end();++lit){
    if (lit->first==0){
      weightedLinkSequence bestSeq=*(lit->second->begin());
      if (direction=="st"){
	bestLinkPosi[count]=make_pair(bestSeq.at(0).tbeg(),bestSeq.at(bestSeq.size()-1).tbeg());
      }else{
	bestLinkPosi[count]=make_pair(bestSeq.at(0).sbeg(),bestSeq.at(bestSeq.size()-1).sbeg());
      }

    }else{
      bestLinkPosi[count]=make_pair(-1,-1);
    }
    ++count;
  }
  _nPosis=count;
  status=true;
  return oss.str();
}

int futureCostEstimation::calculateDistortion(const wlink & lnk, const vector<int> & scov, const vector<int> & tcov, vector<int> & fcHist) const {
  int prevdistortion=0;
  int nextdistortion=0;
  if (_parsingSource){
    //get previous best link position
    if (lnk.sbeg()>0){
      int curPosi=lnk.sbeg()-1;
      //cout<<"lnk:"<<lnk.sbeg()<<","<<lnk.tbeg()<<"\n\t(prev) curPosi:"<<curPosi;
      bestLinkPosiMap::const_iterator hit=bestLinkPosi.find(curPosi);
      while ( curPosi>=0 && (hit==bestLinkPosi.end() || hit->second.first==-1)){
 	--curPosi;
 	hit=bestLinkPosi.find(curPosi);
      }
      if (curPosi>=0 && !scov[curPosi]){
	//cout<<" (after while):"<<curPosi<<" best link:"<<hit->first<<"->"<<hit->second.first<<"-"<<hit->second.second;
 	if (lnk.tbeg() < hit->second.first ){
 	  prevdistortion = hit->second.first-lnk.tbeg();
	  fcHist[curPosi]+=prevdistortion;
 	}
      }
      //cout<<" prev distortion:"<<prevdistortion<<endl;

      //get next best link position
      curPosi=lnk.sbeg()+1;
      //cout<<"\t(next) curPosi:"<<curPosi;
      hit=bestLinkPosi.find(curPosi);
      while (curPosi<_nPosis && (hit==bestLinkPosi.end() || hit->second.first==-1)){
  	++curPosi;
	hit=bestLinkPosi.find(curPosi);
      }
      if (curPosi<_nPosis && !scov[curPosi]){
	//cout<<" (after while):"<<curPosi<<" best link:"<<hit->first<<"->"<<hit->second.first<<"-"<<hit->second.second;
 	if (lnk.tbeg()>hit->second.second){
 	  nextdistortion=lnk.tbeg()-hit->second.second;
 	  fcHist[curPosi]+=nextdistortion;
	}
      }
      //cout<<" prev+next distortion:"<<nextdistortion<<endl;
    }
  }else{
    //get previous best link position
    if (lnk.tbeg()>0){
      int curPosi=lnk.tbeg()-1;
      //cout<<"lnk:"<<lnk.sbeg()<<","<<lnk.tbeg()<<"\n\t(prev) curPosi:"<<curPosi;
      bestLinkPosiMap::const_iterator hit=bestLinkPosi.find(curPosi);
      while ( curPosi>=0 && (hit==bestLinkPosi.end() || hit->second.first==-1)){
 	--curPosi;
 	hit=bestLinkPosi.find(curPosi);
      }
      if (curPosi>=0 && !tcov[curPosi]){
	//cout<<" (after while):"<<curPosi<<" best link:"<<hit->first<<"->"<<hit->second.first<<"-"<<hit->second.second;
 	if (lnk.sbeg() < hit->second.first ){
 	  prevdistortion = hit->second.first-lnk.sbeg();
	  fcHist[curPosi]+=prevdistortion;
 	}
      }
      //cout<<" prev distortion:"<<prevdistortion<<endl;

      //get next best link position
      curPosi=lnk.tbeg()+1;
      //cout<<"\t(next) curPosi:"<<curPosi;
      hit=bestLinkPosi.find(curPosi);
      while (curPosi<_nPosis && (hit==bestLinkPosi.end() || hit->second.first==-1)){
  	++curPosi;
	hit=bestLinkPosi.find(curPosi);
      }
      if (curPosi<_nPosis && !tcov[curPosi]){
	//cout<<" (after while):"<<curPosi<<" best link:"<<hit->first<<"->"<<hit->second.first<<"-"<<hit->second.second;
 	if (lnk.sbeg()>hit->second.second){
 	  nextdistortion=lnk.sbeg()-hit->second.second;
 	  fcHist[curPosi]+=nextdistortion;
	}
      }
      //cout<<" prev+next distortion:"<<nextdistortion<<endl;
    }
  }
  return (prevdistortion+nextdistortion);
}

string futureCostEstimation::printBestLinks() const {
  ostringstream oss("");
  for (bestLinkPosiMap::const_iterator hit=bestLinkPosi.begin();hit!=bestLinkPosi.end();++hit){
    oss<<hit->first<<"-> ("<<hit->second.first<<","<<hit->second.second<<")\n";
  }
  return oss.str();
}

#include "defs.h"

template <> void cnColumn<wlink>::merge(wlink & outlink, string mode) const {
  if (mode == "best"){
    cnColumn<wlink>::const_iterator bestit=column.begin();
    for (cnColumn<wlink>::const_iterator cit=column.begin(); cit!=column.end();++cit){
      if ( cit!=bestit && (cit->first.cst<bestit->first.cst || (cit->first.cst==bestit->first.cst && cit->first.id<bestit->first.id))){bestit=cit;}
    }
    outlink.costs=bestit->first.costs;
    outlink.trgRankForSrc=bestit->first.trgRankForSrc;
    outlink.srcRankForTrg=bestit->first.srcRankForTrg;
    outlink.match=bestit->first.match;
    outlink.stem=bestit->first.stem;
    outlink.syn=bestit->first.syn;
    outlink.pofsCst=bestit->first.pofsCst;
    outlink.cst=bestit->first.cst;
  }else{
    // interpolation
    for (cnColumn<wlink>::const_iterator cit=column.begin(); cit!=column.end();++cit){
      if (outlink.costs.size()==0){
	for (vector<cost>::const_iterator scorit=cit->first.costs.begin(); scorit!=cit->first.costs.end();++scorit){
	  outlink.costs.push_back(*scorit * cit->second);
	}
	outlink.trgRankForSrc = cit->first.trgRankForSrc * cit->second;
	outlink.srcRankForTrg = cit->first.srcRankForTrg * cit->second;
	outlink.match = cit->first.match * cit->second;
	outlink.stem = cit->first.stem * cit->second;
	outlink.syn = cit->first.syn * cit->second;
	outlink.pofsCst = cit->first.pofsCst * cit->second;
	outlink.cst = cit->first.cst * cit->second;
      }else{
	vector<cost>::iterator it=outlink.costs.begin();
	for (vector<cost>::const_iterator scorit=cit->first.costs.begin(); scorit!=cit->first.costs.end();++scorit){
	  *it += *scorit * cit->second;
	  ++it;
	}
	outlink.trgRankForSrc += cit->first.trgRankForSrc * cit->second;
	outlink.srcRankForTrg += cit->first.srcRankForTrg * cit->second;
	outlink.match += cit->first.match * cit->second;
	outlink.stem += cit->first.stem * cit->second;
	outlink.syn += cit->first.syn * cit->second;
	outlink.pofsCst += cit->first.pofsCst * cit->second;
	outlink.cst += cit->first.cst * cit->second;
      }
    }
  } // for cnColumn...
}

void wlinkCoverage::buildIndex(){
  // parse set to have (src,trg) pairs in order, attribute an ID and load hash_map and vector
  linkIndex pairID=0;
  for (set<pair<int,int>,ltpairst>::const_iterator pit=posiOrderSet.begin();pit!=posiOrderSet.end();++pit){
    hash_map<pair<int,int>, linkIndex, hashfintint, hasheqintint>::iterator hit=coverageToIndexMap.find(*pit);
    if (hit==coverageToIndexMap.end()){
      coverageToIndexMap.insert(make_pair(*pit,pairID));
    }else{
      cerr<<"ERROR when loading wlinkCoverage hash_map\n";
      exit(0);
    }
    posiOrderVector.push_back(*pit);
    ++pairID;
  }
  //  cout<<"POSIORDERVECTOR:\n";
  //for (vector<pair<int,int> >::const_iterator it=posiOrderVector.begin();it!=posiOrderVector.end();++it){
  //  cout<<it->first<<","<<it->second<<endl;
  //}
  //cout<<"coverageToIndexMap:\n";
  //for (hash_map<pair<int,int>, linkIndex, hashfintint, hasheqintint>::const_iterator it=coverageToIndexMap.begin();it!=coverageToIndexMap.end();++it){
  //  cout<<it->first.first<<","<<it->first.second<<"=>"<<it->second<<endl;
  //}
}

linkIndex wlinkCoverage::getID(int s, int t) const {
  hash_map<pair<int,int>, linkIndex, hashfintint, hasheqintint>::const_iterator hit=coverageToIndexMap.find(make_pair(s,t));
  if (hit==coverageToIndexMap.end()){
    cerr<<"ERROR ("<<s<<","<<t<<") pair not found in linkCoverage hash\n";exit(0);
  }else{
    return hit->second;
  }
}
bool wlinkCoverage::getIDifExists(int s, int t, linkIndex & id) const {
  hash_map<pair<int,int>, linkIndex, hashfintint, hasheqintint>::const_iterator hit=coverageToIndexMap.find(make_pair(s,t));
  if (hit==coverageToIndexMap.end()){
    return false;
  }else{
    id=hit->second;
    return true;
  }
}

void wlinkCoverage::addAssociatedPbLinks(const wlinkSequence & possLinks){
  for (wlinkSequence::const_iterator lit=possLinks.begin();lit!=possLinks.end();++lit){
    for (int s=lit->sbeg();s<=lit->send();++s){
      for (int t=lit->tbeg();t<=lit->tend();++t){
	hash_map<pair<int,int>,vector<linkIndex>,hashfintint,hasheqintint>::iterator hit=pbLinksForSrcTrgPair.find(make_pair(s,t));
	if (hit==pbLinksForSrcTrgPair.end()){
	  pbLinksForSrcTrgPair.insert(make_pair(make_pair(s,t),vector<linkIndex>(1,lit->id)));
	}else{
	  hit->second.push_back(lit->id);
	}
      }
    }
  }
}
string wlinkCoverage::printAssociatedPbLinks() const {
  ostringstream oss("");
  for (hash_map<pair<int,int>,vector<linkIndex>,hashfintint,hasheqintint>::const_iterator hit=pbLinksForSrcTrgPair.begin();hit!=pbLinksForSrcTrgPair.end();++hit){
    oss<<"("<<hit->first.first<<","<<hit->first.second<<") ->";
    for (vector<linkIndex>::const_iterator it=hit->second.begin();it!=hit->second.end();++it){oss<<" "<<*it;}
    oss<<endl;
  }
  return oss.str();
}

bool wlinkCoverage::getAssociatedPbLinks(int s, int t, vector<linkIndex> & vec) const {
  hash_map<pair<int,int>,vector<linkIndex>,hashfintint,hasheqintint>::const_iterator hit=pbLinksForSrcTrgPair.find(make_pair(s,t));
  if (hit==pbLinksForSrcTrgPair.end()){
    return false;
  }else{
    vec=hit->second;
    return true;
  }
}
