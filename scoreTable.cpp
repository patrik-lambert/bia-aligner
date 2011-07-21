#include "scoreTable.h"

string wdAssoScore::print() const {
  ostringstream oss;
  for (vector<cost>::const_iterator it=vcosts.begin();it!=vcosts.end();++it){
    oss << *it << " ";
  }
  oss << "(" <<trgRankForSrc << " "<< srcRankForTrg<<";"<<match<<" "<<stem<<" "<<syn<<")";
  return oss.str();
}
void wdAssoScore::normalise( float Z ) {
  for (vector<cost>::iterator it=vcosts.begin();it!=vcosts.end();++it){
    *it /= Z;
  }
  trgRankForSrc /= Z;
  srcRankForTrg /= Z;
}

/* wdAssoQScoreTable class
********************/
void wdAssoQScoreTable::load (string fname, phraseDetection & sPhSearch, phraseDetection & tPhSearch, vocTable & svoc, vocTable & tvoc){
  ifstream ffile(fname.c_str());
  // open input and target (word forms) files:
  if (! ffile){
    cerr << "ERROR while opening file:" << fname << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  if (ffile.eof()){
    cerr << "file "<< fname << " is empty" << endl;
    exit(EXIT_FAILURE);
  }
  while (getline(ffile,line)){
    //line format: src_alUnit Delim trg_alUnit Delim score1 score2 ... scoreN
    istringstream iss(line);
    //cout<<line<<endl;
    alUnit srcv,trgv;
    string curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=svoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	srcv.push_back(curInd);
      }
    }
    if (srcv.size()>1){
      sPhSearch.loadPhrase(srcv);
    }
    //cout<<"|"<<srcv.print()<<"|";
    curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=tvoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	trgv.push_back(curInd);
      }
    }
    if (trgv.size()>1){
      tPhSearch.loadPhrase(trgv);
    }
    //cout<<"|"<<trgv.print()<<"|"<<endl;
    
    int count=0;
    vector<qscore> qscores;    
    float buf;
    while (iss >> buf){
      qscore val=quant.quantize(buf);
      qscores.push_back(val);
      ++count;
    }    
    jash_wdas[make_pair(srcv,trgv)]= qscores;
  }
}

void wdAssoQScoreTable::load (string fname, vocTable & svoc, vocTable & tvoc){
  //  quantization q;
  ifstream ffile(fname.c_str());
  // open input and target (word forms) files:
  if (! ffile){
    cerr << "ERROR while opening file:" << fname << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  //cout << "low:"<<low[0]<<"high:"<<high[0]<<endl;
  if (ffile.eof()){
    cerr << "file "<< fname << " is empty" << endl;
    exit(EXIT_FAILURE);
  }
  while (getline(ffile,line)){
    //line format: src_alUnit Delim trg_alUnit Delim score1 score2 ... scoreN
    istringstream iss(line);
    alUnit srcv,trgv;
    string curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=svoc.insert(curtok);
     if (curtok != alUnitConst::unitDelim){
	srcv.push_back(curInd);
      }
    }
    curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=tvoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	trgv.push_back(curInd);
      }
    }

    int count=0;
    vector<qscore> qscores;    
    float buf;
    while (iss >> buf){
      qscore val=quant.quantize(buf);
      qscores.push_back(val);
      ++count;
    }
    jash_wdas.insert(make_pair( make_pair(srcv,trgv) , qscores ));
    //    jash_wdas[make_pair(srcv,trgv)]= qscores;
  }
}
string wdAssoQScoreTable::print(const vocTable & svoc, const vocTable & tvoc) const {
  ostringstream oss;
  for (hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator hit=jash_wdas.begin();hit!=jash_wdas.end();++hit){
    oss<<hit->first.first.print(svoc)<<"-"<<hit->first.second.print(tvoc);
    oss<<":costs:";
    for (vector<qscore>::const_iterator vit=hit->second.begin();vit!=hit->second.end();++vit){oss<<*vit<<" ";}
    oss<<endl;
  }
  return oss.str();
}

  bool wdAssoQScoreTable::scoreVec(wordIndex srcwd, wordIndex trgwd, vector<qscore> & vec) const {
    alUnit src(srcwd);
    alUnit trg(trgwd);
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      vec=cit->second;
      return true;
    }else{
      return false;
    }
  }
  bool wdAssoQScoreTable::scoreVec(const alUnit & src, const alUnit & trg, vector<qscore> & vec) const {
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      vec=cit->second;
      return true;
    }else{
      return false;
    }
  }
  bool wdAssoQScoreTable::cstVec(wordIndex srcwd, wordIndex trgwd, vector<cost> & vec) const {
    alUnit src(srcwd);
    alUnit trg(trgwd);
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      for (vector<qscore>::const_iterator qv=(cit->second).begin();qv!=(cit->second).end();++qv){
	cost cs=quant.dequantize(*qv);
	vec.push_back(cs);
      }
      return true;
    }else{
      return false;
    }
  }
  bool wdAssoQScoreTable::cstVec(const alUnit & src, const alUnit & trg, vector<cost> & vec) const {
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::const_iterator cit=jash_wdas.find(make_pair(src,trg));
    if ( cit != jash_wdas.end() ) {
      for (vector<qscore>::const_iterator qv=(cit->second).begin();qv!=(cit->second).end();++qv){
	cost cs=quant.dequantize(*qv);
	vec.push_back(cs);
      }
      return true;
    }else{
      return false;
    }
  }
  cost wdAssoQScoreTable::combCst(wordIndex src, wordIndex trg, const param & pars, cost worstCost){
    vector<cost> curCosts;
    if (cstVec(src,trg,curCosts)){
      cost resCst=0;
      if (pars.wwda1[0]!=0){
	resCst+=pars.wwda1[0]*curCosts[0];
      }
      if (pars.wwda2!=0){
	resCst+=pars.wwda2*curCosts[1];
      }
      if (pars.wwda3!=0){
	resCst+=pars.wwda3*curCosts[2];
      }
      if (pars.wwda4!=0){
	resCst+=pars.wwda4*curCosts[3];
      }
      if (pars.wwda5!=0){
	resCst+=pars.wwda5*curCosts[4];
      }
      if (pars.wwda6!=0){
	resCst+=pars.wwda6*curCosts[5];
      }
      return resCst;
    }else{
      return worstCost;
    }
  }
  cost wdAssoQScoreTable::combCst(alUnit src, alUnit trg, const param & pars, cost worstCost){
    vector<cost> curCosts;
    if (cstVec(src,trg,curCosts)){
      cost resCst=0;
      if (pars.wwda1[0]!=0){
	resCst+=pars.wwda1[0]*curCosts[0];
      }
      if (pars.wwda2!=0){
	resCst+=pars.wwda2*curCosts[1];
      }
      if (pars.wwda3!=0){
	resCst+=pars.wwda3*curCosts[2];
      }
      if (pars.wwda4!=0){
	resCst+=pars.wwda4*curCosts[3];
      }
      if (pars.wwda5!=0){
	resCst+=pars.wwda5*curCosts[4];
      }
      if (pars.wwda6!=0){
	resCst+=pars.wwda6*curCosts[5];
      }
      return resCst;
    }else{
      return worstCost;
    }
  }

  void wdAssoQScoreTable::addEntry(alUnit & srcv, alUnit & trgv, vector<cost> & vec){ 
    vector<qscore> qscores;
    for (vector<cost>::const_iterator cstit=vec.begin();cstit!=vec.end();++cstit){
      qscore val=quant.quantize(*cstit);
      qscores.push_back(val);
    }
    hash_map<pair<alUnit,alUnit>, vector<qscore>, hashfpairalunit, hasheqpairalunit>::iterator it=jash_wdas.find(make_pair(srcv,trgv));
    if (it!=jash_wdas.end()){
      it->second=qscores;
    }else{
      jash_wdas.insert(make_pair( make_pair(srcv,trgv),qscores) );
    }
  }

/* wdAssoScoreTable class
********************/
void wdAssoScoreTable::loadByPruningForSentPair (const wdAssoQScoreTable & motherWdas, const matchStemSynTable & mssTab, const sentPair & sp, const param & pars){

  ltcst cmpFunc(pars.wwda1[0],pars.wwda2,pars.wwda3,pars.wwda4,pars.wwda5,pars.wwda6);
  hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit > cost_st;
  hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit > cost_ts;
  //loading src and trg hashes
  vector<alUnit> sAlUnits, tAlUnits;
  sp.getAllSrcAlUnits(sAlUnits);
  sp.getAllTrgAlUnits(tAlUnits);
  bool useMssInfo = (pars.mssTab != "-");
  int ncosts=motherWdas.nCosts();
  for (vector<alUnit>::const_iterator sit=sAlUnits.begin(); sit != sAlUnits.end(); ++sit){
    for (vector<alUnit>::const_iterator tit=tAlUnits.begin();tit != tAlUnits.end(); ++tit){
      wdAssoScore assoScore;
      bool existsAssoScore=false;
      // STATISTICAL DICTIONARY ENTRIES
      if (motherWdas.cstVec(*sit,*tit,assoScore.vcosts)){
	existsAssoScore=true;
	// complete info with match, stem, syn if exists for this entry
	if (useMssInfo && sit->size()==1 && tit->size()==1 && mssTab.getMatchStemSyn(*sit,*tit,assoScore.match,assoScore.stem,assoScore.syn)){
	  for (vector<cost>::iterator vc=assoScore.vcosts.begin();vc!=assoScore.vcosts.end();++vc){
	    if ( assoScore.match ){*vc=pars.cmatch;}
	    else if ( assoScore.stem ){*vc=pars.cstem;}
	    else if ( assoScore.syn ){*vc=pars.csyn;}
	  }
	}
      }else{
	// MATCH, STEM, SYN ENTRIES WHICH ARE NOT IN THE STATISTICAL DICTIONARY
	if (useMssInfo && sit->size()==1 && tit->size()==1 && mssTab.getMatchStemSyn(*sit,*tit,assoScore.match,assoScore.stem,assoScore.syn)){
	  existsAssoScore=true;
	  if ( assoScore.match ){assoScore.vcosts.assign(ncosts,pars.cmatch);}
	  else if ( assoScore.stem ){assoScore.vcosts.assign(ncosts,pars.cstem);}
	  else if ( assoScore.syn ){assoScore.vcosts.assign(ncosts,pars.csyn);}
	  //cout<<"ncosts:"<<ncosts<<" vcosts:"<<sit->print()<<"-"<<tit->print()<<assoScore.vcosts[0]<<endl;
	}
      }

      if (existsAssoScore){
	//cout<<"("<<sit->print()<<","<<tit->print()<<"):"<<assoScore.print()<<endl;
	// load in cost_st
	hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::iterator srcit=cost_st.find(*sit);
	if (srcit==cost_st.end()){
	  //define a set with our instantiated compare function:
	  wd2cst *setPt= new wd2cst(cmpFunc);
	  setPt->insert(make_pair(*tit,assoScore));
	  cost_st.insert(make_pair(*sit,setPt));
	}else{
	  srcit->second->insert(make_pair(*tit,assoScore));
	}
	// load in cost_ts
	hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::iterator trgit=cost_ts.find(*tit);
	if (trgit==cost_ts.end()){
	  wd2cst *setPt= new wd2cst(cmpFunc);
	  setPt->insert(make_pair(*sit,assoScore));
	  cost_ts.insert(make_pair(*tit,setPt));
	}else{
	  trgit->second->insert(make_pair(*sit,assoScore));
	}
      }
    } //for tit
  } //for sit
  //put ranking in hash (srcIndex,trgIndex)->(trgRankForSrc,srcRankForTarget)
  hash_map<pair<alUnit,alUnit>, pair<int,int>, hashfpairalunit, hasheqpairalunit> jash_ranking; 
  for (hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::const_iterator cit=cost_st.begin();cit!=cost_st.end();++cit){
    int cnt=0;
    for (wd2cst::const_iterator wit=cit->second->begin();wit!=cit->second->end();++wit){
      jash_ranking.insert(make_pair(make_pair(cit->first,wit->first),make_pair(cnt,-1)));
      ++cnt;
    }
  }
  for (hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::const_iterator cit=cost_ts.begin();cit!=cost_ts.end();++cit){
    int cnt=0;
    for (wd2cst::const_iterator wit=cit->second->begin();wit!=cit->second->end();++wit){
      hash_map<pair<alUnit,alUnit>, pair<int,int>, hashfpairalunit, hasheqpairalunit>::iterator rit=jash_ranking.find(make_pair(wit->first,cit->first)); 
      rit->second.second=cnt;
      ++cnt;
    }
  }

  //add nbest trg for each src
  for (hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::const_iterator cit=cost_st.begin();cit!=cost_st.end();++cit){
    int cnt=0;
    wd2cst::iterator wit=cit->second->begin();
    while (wit!=cit->second->end() && cnt<pars.n){
      hash_map<pair<alUnit,alUnit>, pair<int,int>, hashfpairalunit, hasheqpairalunit>::const_iterator rit=jash_ranking.find(make_pair(cit->first,wit->first)); 
      jash_wdas.insert(make_pair(make_pair(cit->first,wit->first),wdAssoScore(wit->second.vcosts, wit->second.match, wit->second.stem, wit->second.syn, rit->second.first, rit->second.second)));
      ++cnt;
      ++wit;
    }
    delete cit->second;
  }
  //add nbest src for each trg (if not already in hash)
  for (hash_map<alUnit, wd2cst*, hashfalunit, hasheqalunit >::const_iterator cit=cost_ts.begin();cit!=cost_ts.end();++cit){
    int cnt=0;
    wd2cst::iterator wit=cit->second->begin();
    while (wit!=cit->second->end() && cnt<pars.n){
      hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::iterator np=jash_wdas.find(make_pair(wit->first,cit->first));
      if (np == jash_wdas.end()){
	hash_map<pair<alUnit,alUnit>, pair<int,int>, hashfpairalunit, hasheqpairalunit>::const_iterator rit=jash_ranking.find(make_pair(wit->first,cit->first)); 
	jash_wdas.insert(make_pair(make_pair(wit->first,cit->first),wdAssoScore(wit->second.vcosts, wit->second.match, wit->second.stem, wit->second.syn, rit->second.first, rit->second.second)));
      }
      ++cnt;
      ++wit;
    }
    delete cit->second;
  }
}

string wdAssoScoreTable::display(const vocTable & svoc, const vocTable & tvoc) const {
  ostringstream oss;
  for (hash_map<pair<alUnit,alUnit>, wdAssoScore, hashfpairalunit, hasheqpairalunit>::const_iterator hit=jash_wdas.begin();hit!=jash_wdas.end();++hit){
    oss<<hit->first.first.print(svoc)<<"-"<<hit->first.second.print(tvoc);
    oss<<":costs:";
    for (vector<cost>::const_iterator vit=hit->second.vcosts.begin();vit!=hit->second.vcosts.end();++vit){oss<<*vit<<" ";}
    oss<<"trkfors:"<<hit->second.trgRankForSrc<<" srkfort:"<<hit->second.srcRankForTrg<<endl;
  }
  return oss.str();
}

void matchStemSynTable::load(string fname, vocTable & svoc, vocTable & tvoc){
  ifstream ffile(fname.c_str());
  // open input and target (word forms) files:
  if (! ffile){
    cerr << "ERROR while opening file:" << fname << endl;
    exit(EXIT_FAILURE);
  }
  string line;
  //cout << "low:"<<low[0]<<"high:"<<high[0]<<endl;
  if (ffile.eof()){
    cerr << "file "<< fname << " is empty" << endl;
    exit(EXIT_FAILURE);
  }
  while (getline(ffile,line)){
    //line format: src_alUnit Delim trg_alUnit Delim score1 score2 ... scoreN
    istringstream iss(line);
    alUnit srcv,trgv;
    string curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=svoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	srcv.push_back(curInd);
      }
    }
    curtok="";
    while (curtok != alUnitConst::unitDelim){
      iss >> curtok;
      wordIndex curInd=tvoc.insert(curtok);
      if (curtok != alUnitConst::unitDelim){
	trgv.push_back(curInd);
      }
    }

    hash_map<pair<alUnit,alUnit>, matchStemSyn, hashfpairalunit, hasheqpairalunit>::iterator it=jash_wdas.find(make_pair(srcv,trgv));
    if (it==jash_wdas.end()){
      matchStemSyn mss;    
      iss >> mss.match >> mss.stem >> mss.syn;
      jash_wdas.insert(make_pair( make_pair(srcv,trgv) , mss ));
    }
  }
}
