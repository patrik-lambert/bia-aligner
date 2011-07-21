#include "state.h"

state::state (unsigned int & last_state_id, state * _father, const vector<pair<wlink,bool> > & lnks, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, param & pars, const sentPair & sp, ostringstream & oss, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits, const futureCostEstimation & fcEstim, string direction){
  struct tms *time_buffer = new tms;
  float clocks_per_sec = sysconf (_SC_CLK_TCK);
  pars.nNewStates+=1.0;
  ++last_state_id;
  nhist=0;
  nPtr=0;
  id=last_state_id; 
  father=_father;
  //  (void) times (time_buffer);
  //  double tt1 = time_buffer->tms_utime / clocks_per_sec;
  //  stcov.assign(posiOrderLinkCoverage.size(),false);
  //  (void) times (time_buffer);
  //  double tt2 = time_buffer->tms_utime / clocks_per_sec;
  //  pars.timeControl[5]+=tt2-tt1;  

  (void) times (time_buffer);
  double t1 = time_buffer->tms_utime / clocks_per_sec;

  stcov=father->stcov;
  (void) times (time_buffer);
  double t2 = time_buffer->tms_utime / clocks_per_sec;
  pars.timeControl[0]+=t2-t1;
  (void) times (time_buffer);
  t1 = time_buffer->tms_utime / clocks_per_sec;
  nstcov=father->nstcov;
  if (pars.backTrack){lcov=father->lcov;}
  for (vector<pair<wlink,bool> >::const_iterator vpit=lnks.begin();vpit!=lnks.end();++vpit){
    lnkids.push_back(make_pair(vpit->first.id,vpit->second));
    if (pars.backTrack){
      // update link coverage vector
      if (vpit->second){lcov.at(vpit->first.id)=true;}
      else {lcov.at(vpit->first.id)=false;}
    }
  }
  int newLinkedSrc=0;
  int newLinkedTrg=0;
  scov=father->scov;
  nscov=father->nscov;
  internalCrossNumber=father->internalCrossNumber; // compensation for internal crossings in phrase-based links involving more than one src and trg words
  internalCrossLength=father->internalCrossLength;
  nInternalZigzags=father->nInternalZigzags;
  tcov=father->tcov;
  ntcov=father->ntcov;
  (void) times (time_buffer);
  t2 = time_buffer->tms_utime / clocks_per_sec;
  pars.timeControl[1]+=t2-t1;

  cost sfertCost=0;
  cost tfertCost=0;
  cost sumCost=0;
  cost tumCost=0;
  set<int> visitedSposi,visitedTposi;
  int formerFirst=father->first;
  int formerLast=father->last;
  int formerNstcov=father->nstcov;
  bool updateFirst=false;
  bool updateLast=false;
  vector<int> histKey; // state history vector for hypothesis recombination
  for (vector<pair<wlink,bool> >::const_iterator lit=lnks.begin();lit!=lnks.end();++lit){
    bool addLink=lit->second;

    // update weighted nstcov
    int nsrc=lit->first.send()-lit->first.sbeg()+1;
    int ntrg=lit->first.tend()-lit->first.tbeg()+1;
    if (nsrc>1 && ntrg>1){
      if (addLink) nInternalZigzags+=nsrc*ntrg;
      else nInternalZigzags-=nsrc*ntrg;
    }
    // update coverage vectors
    for (int sposi=lit->first.sbeg();sposi<=lit->first.send();++sposi){
      for (int tposi=lit->first.tbeg();tposi<=lit->first.tend();++tposi){
	visitedSposi.insert(sposi);
	visitedTposi.insert(tposi);
	linkIndex pairId=posiOrderLinkCoverage.getID(sposi,tposi);
	if (addLink){
	  // update link coverage vector
	  stcov[pairId]=true;
	  ++nstcov;
	  // update word coverage vectors
	  ++scov[sposi];
	  ++tcov[tposi];
	}else{
	  // update link coverage vector
	  stcov[pairId]=false;
	  --nstcov;
	  // update word coverage vectors
	  --scov[sposi];
	  --tcov[tposi];
	}
      }
    }
    // update internalCrossNumber, internalCrossLength
    if (lit->first.send()>lit->first.sbeg() && lit->first.tend()>lit->first.tbeg()){
      int deltacn=lit->first.send()-lit->first.sbeg();
      int deltacl=deltacn*(lit->first.tend()-lit->first.tbeg());
      if (addLink){
	internalCrossNumber+=deltacn;
	internalCrossLength+=deltacl;
      }else{
	internalCrossNumber-=deltacn;
	internalCrossLength-=deltacl;
      }
      //    cout<<"[internal] "<<lit->first.print()<<"delta cn:"<<deltacn<<" internal cn:"<<internalCrossNumber<<" delta cl:"<<deltacl<<" internal cl:"<<internalCrossLength<<endl;
    }

    // update (first,last) info
    linkIndex firstCovLinkId=posiOrderLinkCoverage.getID(lit->first.sbeg(),lit->first.tbeg());
    linkIndex lastCovLinkId=posiOrderLinkCoverage.getID(lit->first.send(),lit->first.tend());
    //cout<<"firstCovLinkId:"<<firstCovLinkId<<" lastCovLinkId:"<<lastCovLinkId<<endl;
    //cout<<"[before] former first:"<<formerFirst<<" former last:"<<formerLast<<" first:"<<first<<" last:"<<last<<endl;
    if (addLink && formerNstcov==0){
      first=firstCovLinkId;
      last=lastCovLinkId;
    }else if (!addLink && formerNstcov==1){
      first=-1;
      last=-1;
    }else {
      if (addLink){
	// first
	if (firstCovLinkId < formerFirst){
	  first=firstCovLinkId;
	}else{
	  first=formerFirst;
	}
	// last
	if (lastCovLinkId > formerLast){
	  last=lastCovLinkId;
	}else{
	  last=formerLast;
	}
      }else{
	// first
	if (firstCovLinkId == formerFirst){ // we remove the first link; we know the new first link is greater, but we need to search for its exact position
	  first=firstCovLinkId;
	  updateFirst=true; 
	}else{
	  first=formerFirst;
	}
	// last
	if (lastCovLinkId == formerLast){ // we remove the last link; we know the new last link is smaller, but we need to search for its exact position
	  last=lastCovLinkId;
	  updateLast=true; 
	}else{
	  last=formerLast;
	}
      }
    }
    //cout<<"[after] former first:"<<formerFirst<<" former last:"<<formerLast<<" first:"<<first<<" last:"<<last<<endl;
    formerNstcov=nstcov;
    formerFirst=first;
    formerLast=last;
    //cout<<"[after 2] former first:"<<formerFirst<<" former last:"<<formerLast<<" first:"<<first<<" last:"<<last<<endl;

    // update history for hypothesis recombination
    if (pars.hr>0){
      if (pars.exp=="word" && (direction=="as" || direction =="st")){histKey.push_back(lit->first.tbeg());}
      else if (pars.exp=="word" && (direction=="at" || direction =="ts")){histKey.push_back(lit->first.sbeg());}
      else {histKey.push_back(lit->first.id);}
    }
  } // for (vector<pair<wlink,bool> >::const_iterator lit=lnks.begin();lit!=lnks.end();++lit){

  // update history from father
  if (pars.hr>0){
    list<vector<int> >::reverse_iterator h=father->hist.rbegin();
    int nh=0;
    while (h != father->hist.rend() && nh< pars.hr-1){
      hist.push_front(*h);++nhist;
      ++nh;
      ++h;
    }
    hist.push_back(histKey);
    ++nhist;
  }

  (void) times (time_buffer);
  t1 = time_buffer->tms_utime / clocks_per_sec;
  for (set<int>::const_iterator posit=visitedSposi.begin();posit!=visitedSposi.end();++posit){
    if (scov[*posit]>0 && father->scov[*posit]==0){
      ++newLinkedSrc;
      ++nscov;
      if (pars.wum!=0 || pars.wums!=0) sumCost+=sp.src().at(*posit).getNullCst();
    }else if (scov[*posit]==0 && father->scov[*posit]>0	 ){
      --newLinkedSrc;
      --nscov;
      if (pars.wum!=0 || pars.wums!=0) sumCost-=sp.src().at(*posit).getNullCst();
    }
    if (pars.wf>0 || pars.wfs>0) sfertCost+=sp.src().at(*posit).getFertCst(scov[*posit])-sp.src().at(*posit).getFertCst(father->scov[*posit]);
  }
  for (set<int>::const_iterator posit=visitedTposi.begin();posit!=visitedTposi.end();++posit){
    if (tcov[*posit]>0 && father->tcov[*posit]==0){
      ++newLinkedTrg;
      ++ntcov;
      if (pars.wum!=0 || pars.wumt!=0) tumCost+=sp.trg().at(*posit).getNullCst();
    }else if (tcov[*posit]==0 && father->tcov[*posit]>0	 ){
      --newLinkedTrg;
      --ntcov;
      if (pars.wum!=0 || pars.wumt!=0) tumCost-=sp.trg().at(*posit).getNullCst();
    }
    if (pars.wf>0 || pars.wft>0) tfertCost+=sp.trg().at(*posit).getFertCst(tcov[*posit])-sp.trg().at(*posit).getFertCst(father->tcov[*posit]);
  }
  
  int ncross=0;
  int lcross=0;
  int nSrcGaps=0;
  int nTrgGaps=0;
  int nzigzags=0;
  int n1to1=0;
  int n1toN=0;
  int nNto1=0;
  int nNtoM=0;
  //int nSameSrc=0; // number of links with same source as lnk
  vector<int> lastSrc(tcov.size(),-1); // Last source position seen for this target position

  (void) times (time_buffer);
  t2 = time_buffer->tms_utime / clocks_per_sec;
  pars.timeControl[2]+=t2-t1;
  // UPDATE COVERAGE AND OTHER COUNTS
  int curTrg=-1;
  int curSrc=-1;
  int firstFound=-1;
  int lastFound=-1;
  (void) times (time_buffer);
  t1 = time_buffer->tms_utime / clocks_per_sec;
  //  for (linkIndex v=first;v<=last;++v){
  for (linkIndex v=0;v<stcov.size();++v){
    if (stcov[v]){
      if (updateFirst && firstFound<0){firstFound=v;}
      if (updateLast){lastFound=v;}
      int sposi=posiOrderLinkCoverage.src(v);
      int tposi=posiOrderLinkCoverage.trg(v);
      //cerr<<"("<<sposi<<","<<tposi<<")"<<endl;
      // count crossings
      if (tposi-curTrg<0){
	++ncross;
	lcross+=-tposi+curTrg;
	//cerr<<"\tcrossing\n";
      }
      // gap calculations
      if (lastSrc[tposi]>-1 && sposi-lastSrc[tposi]>1){
	nSrcGaps+=sposi-lastSrc[tposi]-1;
      }
      lastSrc[tposi]=sposi;
      if (sposi>curSrc){
	curSrc=sposi;
      }else{
	if (tposi-curTrg>1){
	  nTrgGaps+=tposi-curTrg-1;
	}
      }
      curTrg=tposi;
      // zigzag calculations (links between positions which both have at least another link)
      if (scov.at(sposi)>1 && tcov.at(tposi)>1){++nzigzags;}

      // link type counts
      if (scov.at(sposi)==1){
	if (tcov.at(tposi)==1){++n1to1;}
	else if (tcov.at(tposi)>1){++nNto1;}
      }else if (scov.at(sposi)>1){
	if (tcov.at(tposi)==1){++n1toN;}
	else if (tcov.at(tposi)>1){++nNtoM;}
      }
    }
  } //for (linkIndex v=first;v<=last;++v){
  // For many-to-many links, we count only nsrc*ntrg*linkWeight zigzags instead of nsrc*ntrg
  nzigzags-=nInternalZigzags;
 
  (void) times (time_buffer);
  t2 = time_buffer->tms_utime / clocks_per_sec;
  pars.timeControl[3]+=t2-t1;
  
  (void) times (time_buffer);
  t1 = time_buffer->tms_utime / clocks_per_sec;
  if (updateFirst){first=firstFound;}
  if (updateLast){last=lastFound;}

  if (pars.verbose>2){
    oss<<"nc:"<<ncross<<" lc:"<<lcross<<"\n";
    oss<<"src gaps:"<<nSrcGaps<<"\n";
    oss<<"trg gaps:"<<nTrgGaps<<"\n";
  }

  // Chunk Filter
  int nChFiltSrc=0;
  int nChFiltTrg=0;
  // WARNING: NOT COMPATIBLE WITH LEVEL>0 SO FAR
  /*
  if ( (pars.wchfilt>0 ||pars.wchfilts>0) && !srcChFiltLimits.empty()){
    int curCh=sp.srcat(lnk.sbeg()).getPos(1);
    if (lnk.tbeg()<srcChFiltLimits[curCh].first || lnk.tbeg()>srcChFiltLimits[curCh].second){
      if (addLink) ++nChFiltSrc;
      else --nChFiltSrc;
      //oss<<"chfiltsrc:"<<lnk.tbeg()<<" outside ["<<srcChFiltLimits[curCh].first<<","<<srcChFiltLimits[curCh].second<<"]\n";
    }
  }
  if ( (pars.wchfilt>0 ||pars.wchfiltt>0) && !trgChFiltLimits.empty()){
    int curCh=sp.trgat(lnk.tbeg()).getPos(1);
    if (lnk.sbeg()<trgChFiltLimits[curCh].first || lnk.sbeg()>trgChFiltLimits[curCh].second){
      if (addLink) ++nChFiltTrg;
      else --nChFiltTrg;
      //oss<<"chfilttrg:"<<lnk.sbeg()<<" outside ["<<trgChFiltLimits[curCh].first<<","<<trgChFiltLimits[curCh].second<<"]\n";
    }
  }
  */
  
// COST CALCULATIONS
  //********************
  int lastModel=0;
  cost currentCost;
  transmissibleCst=father->transmissibleCst;
  cost wwda1Cst=0;  cost wwda2Cst=0;  cost wwda3Cst=0;	cost wwda4Cst=0;  cost wwda5Cst=0;  cost wwda6Cst=0;  cost wtgaCst=0;
  cost wrkCst=0;  cost wrksCst=0;  cost wrktCst=0;  cost wrkbCst=0;  cost wrkwCst=0;
  cost wmatchbCst=0;  cost wstembCst=0;  cost wsynbCst=0;
  cost wlbCst=0;  cost wppCst=0;  cost wupCst=0;  cost wusCst=0;  cost wutCst=0; cost wumCst=0; cost wumsCst=0; cost wumtCst=0;
  cost wfCst=0; cost wfsCst=0; cost wftCst=0;

  for (vector<pair<wlink,bool> >::const_iterator lit=lnks.begin();lit!=lnks.end();++lit){
    bool addLink=lit->second;
    
    // word association scores
    if (pars.wwda1[level]!=0){
      if (addLink) currentCost=pars.wwda1[level] * lit->first.costs[0];
      else currentCost=-pars.wwda1[level] * lit->first.costs[0];
      wwda1Cst+=currentCost;
    }
    if (pars.wwda2!=0 && level==0){
      if (addLink) currentCost=pars.wwda2*lit->first.costs[1];
      else currentCost=-pars.wwda2*lit->first.costs[1];
      wwda2Cst+=currentCost;
    }
    if (pars.wwda3!=0 && level==0){
      if (addLink) currentCost=pars.wwda3*lit->first.costs[2];
      else currentCost=-pars.wwda3*lit->first.costs[2];
      wwda3Cst+=currentCost;
    }
    if (pars.wwda4!=0 && level==0){
      if (addLink) currentCost=pars.wwda4*lit->first.costs[3];
      else currentCost=-pars.wwda4*lit->first.costs[3];
      wwda4Cst+=currentCost;
    }
    if (pars.wwda5!=0 && level==0){
      if (addLink) currentCost=pars.wwda5*lit->first.costs[3];
      else currentCost=-pars.wwda5*lit->first.costs[3];
      wwda5Cst+=currentCost;
    }
    if (pars.wwda6!=0 && level==0){
      if (addLink) currentCost=pars.wwda6*lit->first.costs[3];
      else currentCost=-pars.wwda6*lit->first.costs[3];
      wwda6Cst+=currentCost;
    }
    // Part-of-Speech association score
    if (pars.wtga!=0 && level==0){
      if (addLink) currentCost=pars.wtga*lit->first.pofsCst;
      else currentCost=-pars.wtga*lit->first.pofsCst;
      wtgaCst+=currentCost;
    }
    // association rank
    if (pars.wrk!=0 && level==0){
      if (addLink) currentCost=pars.wrk*(lit->first.trgRankForSrc+lit->first.srcRankForTrg);
      else currentCost=-pars.wrk*(lit->first.trgRankForSrc+lit->first.srcRankForTrg);
      wrkCst+=currentCost;
    }
    if (pars.wrks!=0 && level==0){
      if (addLink) currentCost=pars.wrks*lit->first.trgRankForSrc;
      else currentCost=-pars.wrks*lit->first.trgRankForSrc;
      wrksCst+=currentCost;
    }
    if (pars.wrkt!=0 && level==0){
      if (addLink) currentCost=pars.wrkt*lit->first.srcRankForTrg;
      else currentCost=-pars.wrkt*lit->first.srcRankForTrg;
      wrktCst+=currentCost;
    }
    float bestRank,worstRank;
    if (lit->first.trgRankForSrc>lit->first.srcRankForTrg){
      bestRank=lit->first.srcRankForTrg;
      worstRank=lit->first.trgRankForSrc;
    }else{
      bestRank=lit->first.trgRankForSrc;
      worstRank=lit->first.srcRankForTrg;
    }
    if (pars.wrkb!=0 && level==0){
      if (addLink) currentCost=pars.wrkb*bestRank;
      else currentCost=-pars.wrkb*bestRank;
      wrkbCst+=currentCost;
    }
    if (pars.wrkw!=0 && level==0){
      if (addLink) currentCost=pars.wrkw*worstRank;
      else currentCost=-pars.wrkw*worstRank;
      wrkwCst+=currentCost;
    }

    // match, stem and syn bonus
    if (pars.wmatchb!=0 && level==0){
      if (addLink) currentCost=-pars.wmatchb*lit->first.match;
      else currentCost=pars.wmatchb*lit->first.match;
      wmatchbCst+=currentCost;
    }
    if (pars.wstemb!=0 && level==0){
      if (addLink) currentCost=-pars.wstemb*lit->first.stem;
      else currentCost=pars.wstemb*lit->first.stem;
      wstembCst+=currentCost;
    }
    if (pars.wsynb!=0 && level==0){
      if (addLink) currentCost=-pars.wsynb*lit->first.syn;
      else currentCost=pars.wsynb*lit->first.syn;
      wsynbCst+=currentCost;
    }
    // link bonus
    int nsrc=lit->first.send()-lit->first.sbeg()+1;
    int ntrg=lit->first.tend()-lit->first.tbeg()+1;
    float linkWeight;
    if (pars.phLinkWeight=="one"){
      linkWeight=1.0/(nsrc*ntrg);
    }else if (pars.phLinkWeight=="weighted"){
      linkWeight=0.5*(1.0/nsrc+1.0/ntrg);
    }else{
      linkWeight=1.0;
    }
    if (pars.wlb[level]!=0){
      if (addLink) currentCost=-1.0*nsrc*ntrg*linkWeight*pars.wlb[level];
      else currentCost=nsrc*ntrg*linkWeight*pars.wlb[level];
      //	  if (addLink) currentCost=-1.0*nsrc*ntrg*pars.wlb[level];
      //	  else currentCost=nsrc*ntrg*pars.wlb[level];
      wlbCst+=currentCost;
    }

    // phrase-link penalty
    if (pars.wpp[level]!=0){
      if (addLink) currentCost=(nsrc*ntrg-1)*pars.wpp[level];
      else currentCost=-1.0*(nsrc*ntrg-1)*pars.wpp[level];
      wppCst+=currentCost;
    }


  } //for (vector<pair<wlink,bool> >::const_iterator lit=lnks.begin();lit!=lnks.end();++lit){

  // unlinked word penalities/model and fertility Delta costs are updated for all links, so we keep this code outside the lnks loop
  // unlinked word penalties 
  if (pars.wup[level]!=0){
    currentCost=-pars.wup[level]*(newLinkedSrc+newLinkedTrg);
    wupCst+=currentCost;
  }
  if (pars.wus[level]!=0){
    currentCost=-pars.wus[level]*newLinkedSrc;
    wusCst+=currentCost;
  }
  if (pars.wut[level]!=0){
    currentCost=-pars.wut[level]*newLinkedTrg;
    wutCst+=currentCost;
  }

  //unlinked word model
  if (pars.wum!=0 && level==0){
    currentCost=-pars.wum*(sumCost+tumCost);
    wumCst+=currentCost;
  }
  if (pars.wums!=0 && level==0){
    currentCost=-pars.wums*sumCost;
    wumsCst+=currentCost;
  }
  if (pars.wumt!=0 && level==0){
    currentCost=-pars.wumt*tumCost;
    wumtCst+=currentCost;
  }

  // fertility costs
  if (pars.wf!=0 && level==0){
    currentCost=pars.wf*(sfertCost+tfertCost);
    wfCst+=currentCost;
  }
  if (pars.wfs!=0 && level==0){
    currentCost=pars.wfs*sfertCost;
    wfsCst+=currentCost;
  }
  if (pars.wft!=0 && level==0){
    currentCost=pars.wft*tfertCost;
    wftCst+=currentCost;
  }

  // word association scores
  if (pars.wwda1[level]!=0){
    transmissibleCst+=wwda1Cst;
    costs.push_back(father->costs[lastModel++]+wwda1Cst);
  }
  if (pars.wwda2!=0 && level==0){
    transmissibleCst+=wwda2Cst;
    costs.push_back(father->costs[lastModel++]+wwda2Cst);
  }
  if (pars.wwda3!=0 && level==0){
    transmissibleCst+=wwda3Cst;
    costs.push_back(father->costs[lastModel++]+wwda3Cst);
  }
  if (pars.wwda4!=0 && level==0){
    transmissibleCst+=wwda4Cst;
    costs.push_back(father->costs[lastModel++]+wwda4Cst);
  }
  if (pars.wwda5!=0 && level==0){
    transmissibleCst+=wwda5Cst;
    costs.push_back(father->costs[lastModel++]+wwda5Cst);
  }
  if (pars.wwda6!=0 && level==0){
    transmissibleCst+=wwda6Cst;
    costs.push_back(father->costs[lastModel++]+wwda6Cst);
  }
  // Part-of-Speech association score
  if (pars.wtga!=0 && level==0){
    transmissibleCst+=wtgaCst;
    costs.push_back(father->costs[lastModel++]+wtgaCst);
  }

  // association rank
  if (pars.wrk!=0 && level==0){
    transmissibleCst+=wrkCst;
    costs.push_back(father->costs[lastModel++]+wrkCst);
  }
  if (pars.wrks!=0 && level==0){
    transmissibleCst+=wrksCst;
    costs.push_back(father->costs[lastModel++]+wrksCst);
  }
  if (pars.wrkt!=0 && level==0){
    transmissibleCst+=wrktCst;
    costs.push_back(father->costs[lastModel++]+wrktCst);
  }
  if (pars.wrkb!=0 && level==0){
    transmissibleCst+=wrkbCst;
    costs.push_back(father->costs[lastModel++]+wrkbCst);
  }
  if (pars.wrkw!=0 && level==0){
    transmissibleCst+=wrkwCst;
    costs.push_back(father->costs[lastModel++]+wrkwCst);
  }

  // match, stem and syn bonus
  if (pars.wmatchb!=0 && level==0){
    transmissibleCst+=wmatchbCst;
    costs.push_back(father->costs[lastModel++]+wmatchbCst);
  }
  if (pars.wstemb!=0 && level==0){
    transmissibleCst+=wstembCst;
    costs.push_back(father->costs[lastModel++]+wstembCst);
  }
  if (pars.wsynb!=0 && level==0){
    transmissibleCst+=wsynbCst;
    costs.push_back(father->costs[lastModel++]+wsynbCst);
  }
  // link bonus
  if (pars.wlb[level]!=0){
    transmissibleCst+=wlbCst;
    costs.push_back(father->costs[lastModel++]+wlbCst);
  }
  // phrase-link penalty
  if (pars.wpp[level]!=0){
    transmissibleCst+=wppCst;
    costs.push_back(father->costs[lastModel++]+wppCst);
  }

  // unlinked word penalties
  if (pars.wup[level]!=0){
    transmissibleCst+=wupCst;
    costs.push_back(father->costs[lastModel++]+wupCst);
  }
  if (pars.wus[level]!=0){
    transmissibleCst+=wusCst;
    costs.push_back(father->costs[lastModel++]+wusCst);
  }
  if (pars.wut[level]!=0){
    transmissibleCst+=wutCst;
    costs.push_back(father->costs[lastModel++]+wutCst);
  }

  //unlinked word model
  if (pars.wum!=0 && level==0){
    transmissibleCst+=wumCst;
    costs.push_back(father->costs[lastModel++]+wumCst);
  }
  if (pars.wums!=0 && level==0){
    transmissibleCst+=wumsCst;
    costs.push_back(father->costs[lastModel++]+wumsCst);
  }
  if (pars.wumt!=0 && level==0){
    transmissibleCst+=wumtCst;
    costs.push_back(father->costs[lastModel++]+wumtCst);
  }

  // fertility costs
  if (pars.wf!=0 && level==0){
    transmissibleCst+=wfCst;
    costs.push_back(father->costs[lastModel++]+wfCst);
  }
  if (pars.wfs!=0 && level==0){
    transmissibleCst+=wfsCst;
    costs.push_back(father->costs[lastModel++]+wfsCst);
  }
  if (pars.wft!=0 && level==0){
    transmissibleCst+=wftCst;
    costs.push_back(father->costs[lastModel++]+wftCst);
  }

  cst=transmissibleCst;

  // crossing penalties
  //cout<<"id:"<<id<<" ncross:"<<ncross<<" intcn:"<<internalCrossNumber<<endl;
  if (pars.wcn[level]!=0){
    currentCost=pars.wcn[level]*(ncross-internalCrossNumber);
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.wcl[level]!=0){
    currentCost=pars.wcl[level]*(lcross-internalCrossLength);
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  
  // gap penalties
  if (pars.whp[level]!=0){
    currentCost=pars.whp[level]*(nSrcGaps+nTrgGaps);
    if (pars.verbose>2){oss<<"previous cst:"<<cst<< " current cst:"<<currentCost<<endl;}
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.whs[level]!=0){
    currentCost=pars.whs[level]*nSrcGaps;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.wht[level]!=0){
    currentCost=pars.wht[level]*nTrgGaps;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }

  // zigzag penalty
  if (pars.wzp[level]!=0){
    currentCost=pars.wzp[level]*nzigzags;
    if (pars.verbose>2){oss<<"previous cst:"<<cst<< " current cst:"<<currentCost<<endl;}
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }

  // link type counts
  if (pars.w1to1!=0 && level==0){
    currentCost=-pars.w1to1*n1to1;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.w1toN!=0 && level==0){
    currentCost=-pars.w1toN*n1toN;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.wNto1!=0 && level==0){
    currentCost=-pars.wNto1*nNto1;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.w1toNsum!=0 && level==0){
    currentCost=-pars.w1toNsum*(n1toN+nNto1);
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }
  if (pars.wNtoM!=0 && level==0){
    currentCost=-pars.wNtoM*nNtoM;
    cst+=currentCost;
    ++lastModel;
    costs.push_back(currentCost);
  }

  // chunk filter
  if (pars.wchfilt!=0){
    currentCost=pars.wchfilt*(nChFiltSrc+nChFiltTrg);
    cst+=currentCost;
    costs.push_back(father->costs[lastModel++]+currentCost);
  }
  if (pars.wchfilts!=0){
    currentCost=pars.wchfilts*nChFiltSrc;
    cst+=currentCost;
    costs.push_back(father->costs[lastModel++]+currentCost);
  }
  if (pars.wchfiltt!=0){
    currentCost=pars.wchfiltt*nChFiltTrg;
    cst+=currentCost;
    costs.push_back(father->costs[lastModel++]+currentCost);
  }

  // future cost
  if (fcEstim.enabled()){
    if (father->fcHist.size()==0){
      if (fcEstim.parsingSource()){fcHist.assign(scov.size(), 0);}
      else {fcHist.assign(tcov.size(), 0);}
    }else{
      fcHist=father->fcHist;
    }
    // now that we just covered the position, true distortion will be calculated for this position and we remove estimated future costs from that position:
    if (fcEstim.parsingSource()){cst-=fcHist[lnks[0].first.sbeg()];}
    else {cst-=fcHist[lnks[0].first.tbeg()]*1.0;}
    float curFutureCost=0;
    curFutureCost+=fcEstim.calculateDistortion(lnks[0].first,scov,tcov,fcHist)*1.0;
    fcst=curFutureCost+cst;
  }else{
    fcst=cst;
  }
  (void) times (time_buffer);
  t2 = time_buffer->tms_utime / clocks_per_sec;
  delete time_buffer;
  pars.timeControl[4]+=t2-t1;
}

void state::setEmptyAl (linkIndex nstpairs, linkIndex nPossLinks, const sentPair & sp, int level, const param & pars){
  id=0;
  //  lnkids.assign(1,make_pair(-1,false));
  int nsrc=sp.numSrcToks(level);
  int ntrg=sp.numTrgToks(level);
  if (pars.backTrack){lcov.assign(nPossLinks,false);}
  stcov.assign(nstpairs,false);
  scov.assign(nsrc,0);
  nscov=0;
  internalCrossNumber=0;
  internalCrossLength=0;
  nInternalZigzags=0;
  tcov.assign(ntrg,0);
  ntcov=0;
  first=-1;
  last=first;
  nstcov=0;
  father=NULL;
  cst=0;
  transmissibleCst=0;
  nhist=0;
  if (pars.wwda1[level]!=0) {costs.push_back(0);}
  if (pars.wwda2!=0 && level==0){costs.push_back(0);}
  if (pars.wwda3!=0 && level==0){costs.push_back(0);}
  if (pars.wwda4!=0 && level==0){costs.push_back(0);}
  if (pars.wwda5!=0 && level==0){costs.push_back(0);}
  if (pars.wwda6!=0 && level==0){costs.push_back(0);}
  if (pars.wtga!=0 && level==0){costs.push_back(0);}
  if (pars.wrk!=0 && level==0){costs.push_back(0);}
  if (pars.wrks!=0 && level==0){costs.push_back(0);}
  if (pars.wrkt!=0 && level==0){costs.push_back(0);}
  if (pars.wrkb!=0 && level==0){costs.push_back(0);}
  if (pars.wrkw!=0 && level==0){costs.push_back(0);}
  if (pars.wmatchb!=0 && level==0){costs.push_back(0);}
  if (pars.wstemb!=0 && level==0){costs.push_back(0);}
  if (pars.wsynb!=0 && level==0){costs.push_back(0);}
  if (pars.wlb[level]!=0){costs.push_back(0);}
  if (pars.wpp[level]!=0){costs.push_back(0);}
  if (pars.wup[level]!=0){
    cost currentCost=pars.wup[level]*(nsrc+ntrg);
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wus[level]!=0){
    cost currentCost=pars.wus[level]*nsrc;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wut[level]!=0){
    cost currentCost=pars.wut[level]*ntrg;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  cost sfertCost=0;
  cost sumCost=0;
  if ((pars.wums!=0 || pars.wum!=0 || pars.wfs!=0 || pars.wf!=0) && level==0){
    for (confusionNet<iword>::const_iterator s=sp.src().begin();s!=sp.src().end();++s){
      if (pars.wfs!=0 || pars.wf!=0) sfertCost+=s->getFertCst(0);
      if (pars.wums!=0 || pars.wum!=0) sumCost+=s->getNullCst();
    }
  }
  cost tfertCost=0;
  cost tumCost=0;
  if ((pars.wumt!=0 || pars.wum!=0 || pars.wft!=0 || pars.wf!=0) && level==0){
    for (confusionNet<iword>::const_iterator s=sp.trg().begin();s!=sp.trg().end();++s){
      if (pars.wft!=0 || pars.wf!=0) tfertCost+=s->getFertCst(0);
      if (pars.wumt!=0 || pars.wum!=0) tumCost+=s->getNullCst();
    }
  }
  if (pars.wum!=0 && level==0){
    cost currentCost=pars.wum*(sumCost+tumCost);
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wums!=0 && level==0){
    cost currentCost=pars.wums*sumCost;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wumt!=0 && level==0){
    cost currentCost=pars.wumt*tumCost;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wf!=0 && level==0){
    cost currentCost=pars.wf*(sfertCost+tfertCost);
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wfs!=0 && level==0){
    cost currentCost=pars.wfs*sfertCost;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wft!=0 && level==0){
    cost currentCost=pars.wft*tfertCost;
    cst+=currentCost;
    costs.push_back(currentCost);
  }
  if (pars.wcn[level]!=0){costs.push_back(0);}
  if (pars.wcl[level]!=0){costs.push_back(0);}
  if (pars.whp[level]!=0){costs.push_back(0);}
  if (pars.whs[level]!=0){costs.push_back(0);}
  if (pars.wht[level]!=0){costs.push_back(0);}
  if (pars.wzp[level]!=0){costs.push_back(0);}
  if (pars.w1to1!=0 && level==0){costs.push_back(0);}
  if (pars.w1toN!=0 && level==0){costs.push_back(0);}
  if (pars.wNto1!=0 && level==0){costs.push_back(0);}
  if (pars.w1toNsum!=0 && level==0){costs.push_back(0);}
  if (pars.wNtoM!=0 && level==0){costs.push_back(0);}
  fcst=cst;
  transmissibleCst=cst;
}
string state::print(const wlinkSequence & possLinks, const param & pars) const {
  ostringstream oss("");
  oss<<"ID:"<<id<<" ";//<<this;
  oss<<" (father:";//<<father<<" ";
  if (father!=NULL){
    oss<<father->id;
  }
  oss<<")";
  //print links in state:
  oss<<"\tCOV:";
  int cntlcov=0;
  if (lcov.size()>0){
    for (linkIndex v=0;v<lcov.size();++v){  
      if (lcov[v]>0){
	oss<<possLinks[v].printTalpDebug()<<" ";
	++cntlcov;
      }
    }
  }else{
    vector<int> vec(possLinks.size(),0);
    retrieveLinkCoverage(vec);
    for (linkIndex v=0;v<vec.size();++v){  
      if (vec[v]){
	oss<<possLinks[v].printTalpDebug()<<" ";
      }
    }
  }
  oss<<"(lcov "<<cntlcov<<"/"<<lcov.size()<<") nstcov:"<<nstcov<<" (1st,last): ("<<first<<","<<last<<")";
  //oss<<" hist ("<<nhist<<"):";
  oss<<" hist:";
  for (list<vector<int> >::const_iterator h=hist.begin();h!=hist.end();++h){
    if (h!=hist.begin()) oss<<",";
    for (vector<int>::const_iterator v=h->begin();v!=h->end();++v){
      if (v!=h->begin()){oss<<" ";}
      oss<<*v;
    }
  }
  oss<<" SRC:";
  for (vector<int>::const_iterator v=scov.begin();v!=scov.end();++v){oss<<*v;}
  oss<<" TRG:";
  for (vector<int>::const_iterator v=tcov.begin();v!=tcov.end();++v){oss<<*v;}
  // cost 
  if (costs.size()>0){
    oss<<"\tcosts:f"<<fcst<<" "<<cst<<" (";
    //oss<<"\tcosts:f"<<fcst<<" "<<cst<<" "<<transmissibleCst<<" (";
    int ncst=0;
    for (vector<cost>::const_iterator c=costs.begin();c!=costs.end();++c){
      oss<<" "<<pars.models.at(0).at(ncst)<<":"<<*c;
      ++ncst;
    }
    oss<<" )";
  }
  return oss.str();
}

string state::printAlignment (const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars ) const {
  ostringstream oss("");
  bool firstLink=true;
  linkClusterDiv clusts;
  for (int lid=0;lid<lcov.size();++lid){
    if (lcov[lid]){
      if (pars.outputType == "clusters"){
	//insterting word-to-word links in vector of clusters
	for (int sposi=possLinks[lid].sbeg();sposi<=possLinks[lid].send();++sposi){
	  for (int tposi=possLinks[lid].tbeg();tposi<=possLinks[lid].tend();++tposi){
	    clusts.addLink(sposi,tposi);
	  }
	}	  
      }else if (pars.outputType == "phrase-links"){
	if (firstLink){firstLink=false;}
	else{oss<<" ";}
	oss<<possLinks[lid].printTalpDebug();
      }else{ //pars.outputType == "word-links"
	if (direction=="at" || direction=="ts"){
	  // each target word can be linked to at most 1 source word
	  //oss<<":"<<possLinks[lid].printTalpDebug()<<":";
	  for (int tposi=possLinks[lid].tbeg();tposi<=possLinks[lid].tend();++tposi){
	    if (possLinks[lid].sbeg()==possLinks[lid].send()){
	      if (firstLink){firstLink=false;}
	      else{oss<<" ";}
	      oss<<possLinks[lid].sbeg()<<"-"<<tposi;
	    }else{
	      //look for the best linked source
	      int bestposi=-1;
	      for (set<wlink>::const_iterator l=trgLinksVec[0][tposi].second.begin(); l!=trgLinksVec[0][tposi].second.end();++l){
		if (l->sbeg()==l->send() && l->sbeg()>=possLinks[lid].sbeg() && l->sbeg()<=possLinks[lid].send()){
		  bestposi=l->sbeg();
		  //oss<<"\n"<<l->print()<<endl;
		  break;
		}
	      }
	      if (bestposi>=0){
		if (firstLink){firstLink=false;}
		else{oss<<" ";}
		oss<<bestposi<<"-"<<tposi;
	      }
	    }
	  }	    
	}else if (direction=="as" || direction=="st"){
	  // each source word can be linked to at most 1 target word
	  for (int sposi=possLinks[lid].sbeg();sposi<=possLinks[lid].send();++sposi){
	    if (possLinks[lid].tbeg()==possLinks[lid].tend()){
	      if (firstLink){firstLink=false;}
	      else{oss<<" ";}
	      oss<<sposi<<"-"<<possLinks[lid].tbeg();
	    }else{
	      //look for the best linked target
	      int bestposi=-1;
	      for (set<wlink>::const_iterator l=srcLinksVec[0][sposi].second.begin(); l!=srcLinksVec[0][sposi].second.end();++l){
		if (l->tbeg()==l->tend() && l->tbeg()>=possLinks[lid].tbeg() && l->tbeg()<=possLinks[lid].tend()){
		  bestposi=l->tbeg();
		  //oss<<"\n"<<l->print()<<endl;
		  break;
		}
	      }
	      if (bestposi>=0){
		if (firstLink){firstLink=false;}
		else{oss<<" ";}
		oss<<sposi<<"-"<<bestposi;
	      }
	    }
	  }  
	}else{
	  // we can have many-to-many links
	  if (firstLink){firstLink=false;}
	  else{oss<<" ";}
	  oss<<possLinks[lid].printExtended();
	} // if (direction==
      } // if (pars.outputType ==
    } //if (lcov[lid]){
  } //for
  if (pars.outputType == "clusters"){
    clusts.sortAndSplitIntoContiguous();
    oss<<clusts.print();
  }
  return oss.str();
}
string state::printForNbest (const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars,int numSentPair ) const {
  ostringstream oss("");
  oss<<numSentPair<<" ||| ";
  oss<<printAlignment (possLinks, srcLinksVec, trgLinksVec, direction, pars );
  oss<<" |||";
  int ncst=0;
  for (vector<cost>::const_iterator c=costs.begin();c!=costs.end();++c){
    oss<<" "<<pars.models.at(0).at(ncst)<<": "<<*c;
    ++ncst;
  }
  oss<<" ||| ";
  oss<<cst;
  return oss.str();
}

void state::printLinkSequence(const wlinkSequence & posOrderPossLinks, wlinkSequence & linkseq) const {
  for (linkIndex v=0;v<stcov.size();++v){  
    if (stcov[v]){
      linkseq.push_back(posOrderPossLinks[v]);
    }
  }
}

void state::calculateTrgChFiltLimits(vector<pair<int,int> > & trgChFiltLims, const sentPair & sp,const wlinkSequence & possLinks, int level){
  // initialize limits
  trgChFiltLims.assign(sp.numTrgToks(level+1),make_pair(-1,sp.numSrcToks(level)));
  int prevSrc=-1;
  int prevChk=-1;
  for (linkIndex v=0;v<stcov.size();++v){  
    if (stcov[v]){
      int curChk=sp.trgat(possLinks[v].tbeg()).getPos(level+1);
      //cout <<possLinks[v].printTalpDebug()<<" chk trg:"<<curChk<<endl;
      if (curChk != prevChk && prevChk != -1){
	if (trgChFiltLims[prevChk].second < possLinks[v].sbeg() || trgChFiltLims[prevChk].second == sp.numSrcToks(level)){
	  trgChFiltLims[prevChk].second=possLinks[v].sbeg();
	}
	if (trgChFiltLims[curChk].second > prevSrc){
	  trgChFiltLims[curChk].first=prevSrc;
	}else{
	  trgChFiltLims[curChk].second=sp.numSrcToks(level);
	}
      }
      prevChk=curChk;
      prevSrc=possLinks[v].sbeg();
    }
  }
}
void state::calculateSrcChFiltLimits(vector<pair<int,int> > & srcChFiltLims, const sentPair & sp,const wlinkSequence & possLinks, int level){
  // initialize limits
  srcChFiltLims.assign(sp.numSrcToks(level+1),make_pair(-1,sp.numTrgToks(level)));
  int prevTrg=-1;
  int prevChk=-1;
  // getting links to reorder them
  wlinkSequence tsOrderLinks;
  for (linkIndex v=0;v<stcov.size();++v){  
    if (stcov[v]){
      tsOrderLinks.push_back(possLinks[v]);
    }
  }
  stable_sort(tsOrderLinks.begin(),tsOrderLinks.end(),ltwlinkidts());

  for (linkIndex v=0;v<tsOrderLinks.size();++v){
    int curChk=sp.srcat(tsOrderLinks[v].sbeg()).getPos(level+1);
    //cout <<tsOrderLinks[v].printTalpDebug()<<" chk src:"<<curChk<<endl;
    if (curChk != prevChk && prevChk != -1){
      if (srcChFiltLims[prevChk].second < tsOrderLinks[v].tbeg() || srcChFiltLims[prevChk].second == sp.numTrgToks(level)){
	srcChFiltLims[prevChk].second=tsOrderLinks[v].tbeg();
      }
      if (srcChFiltLims[curChk].second > prevTrg){
	srcChFiltLims[curChk].first=prevTrg;
      }else{
	srcChFiltLims[curChk].second=sp.numTrgToks(level);
      }
    }
    prevChk=curChk;
    prevTrg=tsOrderLinks[v].tbeg();
  }
}

bool state::compatibleWithLink( const wlink & lnk, const wlinkCoverage & posiOrderLinkCoverage, const wordLinkSequence & srcLinks, const wordLinkSequence & trgLinks, const param & pars, set<linkIndex> & conflicts ) const {
  bool compatible=true;
  if (pars.oneToOne){
    for (int s=lnk.sbeg(); s<=lnk.send(); ++s){
      if (scov.at(s)){
	if (pars.backTrack){
	  for (set<wlink,ltwlink>::const_iterator it=srcLinks.at(s).second.begin();it!=srcLinks.at(s).second.end();++it){
	    if (lcov.at(it->id)){
	      compatible=false;
	      conflicts.insert(it->id);
	    }
	  }
	}else{
	  return false;
	}
      }
    }
    for (int t=lnk.tbeg(); t<=lnk.tend(); ++t){
      if (tcov.at(t)){
	if (pars.backTrack){
	  for (set<wlink,ltwlink>::const_iterator it=trgLinks.at(t).second.begin();it!=trgLinks.at(t).second.end();++it){
	    if (lcov.at(it->id)){
	      compatible=false;
	      conflicts.insert(it->id);
	    }
	  }
	}else{
	  return false;
	}
      }
    }
  }else{
    for (int s=lnk.sbeg(); s<=lnk.send(); ++s){
      if (! scov.at(s)) continue;
      for (int t=lnk.tbeg(); t<=lnk.tend(); ++t){
	if (! tcov.at(t)) continue;
	linkIndex pairID=posiOrderLinkCoverage.getID(s,t);
	if (stcov.at(pairID)){
	  if (pars.backTrack){
	    vector<linkIndex> vec;
	    if (posiOrderLinkCoverage.getAssociatedPbLinks(s, t, vec)){
	      for (vector<linkIndex>::const_iterator it=vec.begin();it!=vec.end();++it){
		if (lcov.at(*it)){
		  compatible=false;
		  conflicts.insert(*it);
		}
	      }
	    }
	  }else{
	    return false;
	  }
	}
      }
    }
  }
  return compatible;
}

void state::retrieveLinkCoverage( vector<int> & vec ) const{
  if (vec.size()==0){cerr << "ERROR in retrieveLinkCoverage: vec has size zero. Its size should be the number of possible links\n"; exit(0);}
  if (father != NULL && father->lcov.size()>0){
    for (int i=0; i< father->lcov.size();++i){
      if (father->lcov[i]){++vec[i];}
    }
  }
  if (lnkids.size()>0){
    for (vector<pair<linkIndex,bool> >::const_iterator it=lnkids.begin();it!=lnkids.end();++it){
      if (it->second){ //addlink
	++vec[it->first];
      }else{
	--vec[it->first];
      }
    }
    if (father != NULL && father->lcov.size()==0){
      father->retrieveLinkCoverage(vec);
    }
  }
}

  /*
void state::retrieveLinksInSrcInterval( vector<int> & vec ) const{
  if (vec.size()==0){cerr << "ERROR in retrieveLinkCoverage: vec has size zero\n"; exit(0);}
  if (father != NULL && father->lcov.size()>0){
    for (int i=0; i< father->lcov.size();++i){
      if (father->lcov[i]){++vec[i];}
    }
  }
  if (lnkid != -1){
    if (lnkAction == 1){ //addlink
      ++vec[lnkid];
    }else{
      --vec[lnkid];
    }
    if (father != NULL && father->lcov.size()==0){
      father->retrieveLinkCoverage(vec);
    }
  }
}
  */

state* symmetrisation::getIntersection ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits){
  futureCostEstimation fcNull;
  ostringstream oss;
  state *initAl=new state;
  initAl->setEmptyAl( posiOrderLinkCoverage.size(),possLinks.size(),sp,level,pars );
  vector<int> interlcov(possLinks.size(),false);
  vector<pair<wlink,bool> > lnkvec;
  
  for (int n=0; n<s1->lcov.size();++n){
    if (s1->lcov.at(n)>0 && s2->lcov.at(n)>0){
      lnkvec.push_back(make_pair(possLinks.at(n),true));
      interlcov.at(n)=true;
    }
  }
  
  unsigned int stateId=0;
  state* interState = new state(stateId,initAl,lnkvec, possLinks,posiOrderLinkCoverage, pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
  delete initAl;
  if (!pars.backTrack) interState->setlcov(interlcov);
  interState->id=0;
  return interState;
}

// take the intersection plus the links whose position have not been covered yet
state* symmetrisation::getRefinedIntersection ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits){
  futureCostEstimation fcNull;
  ostringstream oss;
  state *initAl=new state;
  initAl->setEmptyAl( posiOrderLinkCoverage.size(),possLinks.size(),sp,level,pars );
  vector<int> interlcov, interscov, intertcov;
  int nsrc=sp.numSrcToks(level);
  int ntrg=sp.numTrgToks(level);
  interscov.assign(nsrc,0);
  intertcov.assign(ntrg,0);
  interlcov.assign(possLinks.size(),false);
  state * currState;
  sortedLinkSequence notInIntersection; // we store in a set the links which are not in intersection
  for (int n=0; n<s1->lcov.size();++n){
    if (s1->lcov.at(n)>0 && s2->lcov.at(n)>0){
      // mark position covered in this intersection
      for (int sposi=possLinks.at(n).sbeg();sposi<=possLinks.at(n).send();++sposi){++interscov.at(sposi);}
      for (int tposi=possLinks.at(n).tbeg();tposi<=possLinks.at(n).tend();++tposi){++intertcov.at(tposi);}  
      unsigned int stateId=0;
      vector<pair<wlink,bool> > lnkvec(1,make_pair(possLinks.at(n),true));
      state* currState = new state(stateId,initAl,lnkvec, possLinks,posiOrderLinkCoverage, pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
      //cerr << "link "<<possLinks[n].printTalpDebug()<<"intersection:"<<currState->print(possLinks)<<endl;
      delete initAl;
      initAl=currState;
      interlcov.at(n)=true;
      //cerr<<"link "<<n<<"in intersection\n";
    }else if (s1->lcov.at(n)>0 || s2->lcov.at(n)>0){
      notInIntersection.second.insert(possLinks.at(n));
    }
  }
  // for each link outside intersection, we look if its positions have been covered
  // if we include it in refined intersection, we update the coverage vectors
  for (set<wlink>::const_iterator lit=notInIntersection.second.begin();lit!=notInIntersection.second.end();++lit){
    bool scovered=false;
    for (int sposi=lit->sbeg();sposi<=lit->send();++sposi){
      if (interscov.at(sposi)>0){scovered=true;}
    }
    bool tcovered=false;
    for (int tposi=lit->tbeg();tposi<=lit->tend();++tposi){
      if (intertcov.at(tposi)>0){tcovered=true;}
    }  
    if (! (scovered || tcovered)){
      // insert link in intersection and update coverage vectors
      unsigned int stateId=0;
      vector<pair<wlink,bool> > lnkvec(1,make_pair(*lit,true));
      state* currState = new state(stateId,initAl,lnkvec, possLinks,posiOrderLinkCoverage, pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
      delete initAl;
      initAl=currState;
      interlcov.at(lit->id)=true;
      for (int sposi=lit->sbeg();sposi<=lit->send();++sposi){++interscov.at(sposi);}
      for (int tposi=lit->tbeg();tposi<=lit->tend();++tposi){++intertcov.at(tposi);}  
    }
  }
  if (!pars.backTrack) initAl->setlcov(interlcov);
  initAl->id=0;
  return initAl;
}
state* symmetrisation::getUnion ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits){
  futureCostEstimation fcNull;
  ostringstream oss;
  state *initAl=new state;
  initAl->setEmptyAl( posiOrderLinkCoverage.size(),possLinks.size(),sp,level,pars);
  vector<int> unionlcov;
  unionlcov.assign(possLinks.size(),false);
  state * currState;
  for (int n=0; n<s1->lcov.size();++n){
    if (s1->lcov[n] || s2->lcov[n]){
      unsigned int stateId=0;
      vector<pair<wlink,bool> > lnkvec(1,make_pair(possLinks[n],true));
      state* currState = new state(stateId,initAl, lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
      delete initAl;
      initAl=currState;
      unionlcov[n]=true;
    }
  }
  initAl->setlcov(unionlcov);
  initAl->id=0;
  return initAl;
}

state* symmetrisation::getRefinedUnion ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, vector<pair<int,int> > & srcChFiltLimits, vector<pair<int,int> > & trgChFiltLimits){
  ostringstream oss;
  futureCostEstimation fcNull;

  // CONSTRUCT INTERSECTION STATE AND WLINKSEQUENCE VECTORS
  wlinkSequenceVec srcLinks(sp.numSrcToks(level)),trgLinks(sp.numTrgToks(level));
  vector<vector<bool> > srcLinksBool(sp.numSrcToks(level)),trgLinksBool(sp.numTrgToks(level));
  wlinkSequenceVec srcInterLinks(sp.numSrcToks(level)),trgInterLinks(sp.numTrgToks(level));
  wlinkSequenceVec srcUnionNoInterLinks(sp.numSrcToks(level)),trgUnionNoInterLinks(sp.numTrgToks(level));
  state *inter=new state;
  inter->setEmptyAl( possLinks.size(),possLinks.size(),sp,level,pars);
  for (int n=0; n<s1->stcov.size();++n){
    if (s1->stcov[n] && s2->stcov[n]){
      unsigned int stateId=0;
      vector<pair<wlink,bool> > lnkvec(1,make_pair(possLinks[n],true));
      state* currState = new state(stateId,inter, lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
      delete inter;
      inter=currState;

      srcInterLinks[possLinks[n].sbeg()].push_back(possLinks[n]);
      trgInterLinks[possLinks[n].tbeg()].push_back(possLinks[n]);
      srcLinks[possLinks[n].sbeg()].push_back(possLinks[n]);
      srcLinksBool[possLinks[n].sbeg()].push_back(true);
      trgLinks[possLinks[n].tbeg()].push_back(possLinks[n]);
      trgLinksBool[possLinks[n].tbeg()].push_back(true);
    }else if (s1->stcov[n] || s2->stcov[n]){
      srcLinks[possLinks[n].sbeg()].push_back(possLinks[n]);
      srcLinksBool[possLinks[n].sbeg()].push_back(false);
      trgLinks[possLinks[n].tbeg()].push_back(possLinks[n]);
      trgLinksBool[possLinks[n].tbeg()].push_back(false);
      srcUnionNoInterLinks[possLinks[n].sbeg()].push_back(possLinks[n]);
      trgUnionNoInterLinks[possLinks[n].tbeg()].push_back(possLinks[n]);
    }
  }
  
  stateList stateGarbage;
  // BUILD SRC POSSIBLE COMBINATIONS FOR EACH WORD AND CHOSE THE BEST ONE
  state* srefined=new state(*inter);
  for (int n=0; n<srcLinks.size();++n){
    //cout<<"word pos "<<n<<":\n";
    wlinkSequenceVec wlseqVec;
    unsigned int nlinks=1;
    while (nlinks<=srcLinks[n].size() && nlinks<=pars.m){
      vector<unsigned int> pos(nlinks);
      combinations_recursive<wlinkSequenceVec>(srcLinks[n],wlseqVec, "st", nlinks,pos,0,0,srcLinksBool[n],srcInterLinks[n].size());
      ++nlinks;
    }      
    //cout<<"srcLinks:"<<srcLinks[n].display(sp);
    //cout<<"interLinks:"<<srcInterLinks[n].display(sp);
    //cout<<"combs:\n"<<wlseqVec.printTalpDebug();

    // ADD LINKS FROM UNION\INTERSECTION 
    if (wlseqVec.size()==1){
      for (wlinkSequence::const_iterator seqit=wlseqVec.begin()->begin(); seqit!=wlseqVec.begin()->end();++seqit){
	unsigned int stateId=0;
	vector<pair<wlink,bool> > lnkvec(1,make_pair(*seqit,true));
 	state* currState = new state(stateId,srefined,lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
	stateGarbage.push_back(srefined);
	srefined=currState;
      }
    }else if (wlseqVec.size()>1){
      //put each combination in a stack to see which is best (starting from intersection)
      stateStack ststack;
      for (wlinkSequenceVec::const_iterator vit=wlseqVec.begin();vit!=wlseqVec.end();++vit){
	state* father=srefined;
	for (wlinkSequence::const_iterator seqit=vit->begin(); seqit!=vit->end();++seqit){
	  unsigned int stateId=0;
	  vector<pair<wlink,bool> > lnkvec(1,make_pair(*seqit,true));
	  state* currState = new state(stateId,father, lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
	  father=currState;
	  if (seqit<vit->end()-1){
	    stateGarbage.push_back(father);
	  }
	}
	ststack.cleverInsert(father);
	//cout<<"STACK :\n"<<ststack .print(possLinks)<<endl;
      }
      ++(*ststack.begin())->nPtr;
      stateGarbage.push_back(srefined);
      srefined=*(ststack.begin());
    }
  }
  //cout<<"s1:\n"<<s1->print(possLinks)<<endl;  
  //cout<<"s2:\n"<<s2->print(possLinks)<<endl;  
  //cout<<"srefined:\n"<<srefined->print(possLinks)<<endl;  

  // BUILD TRG POSSIBLE COMBINATIONS FOR EACH WORD AND CHOSE THE BEST ONE
  state* trefined=new state(*inter);
  for (int n=0; n<trgLinks.size();++n){
    //cout<<"word pos "<<n<<":\n";
    wlinkSequenceVec wlseqVec;
    unsigned int nlinks=1;
    while (nlinks<=trgLinks[n].size() && nlinks<=pars.m){
      vector<unsigned int> pos(nlinks);
      combinations_recursive<wlinkSequenceVec>(trgLinks[n],wlseqVec, "ts", nlinks,pos,0,0,trgLinksBool[n],trgInterLinks[n].size());
      ++nlinks;
    }      
    //cout<<"trgLinks:"<<trgLinks[n].display(sp);
    //cout<<"interLinks:"<<trgInterLinks[n].display(sp);
    //cout<<"combs:\n"<<wlseqVec.printTalpDebug();

    // ADD LINKS FROM UNION\INTERSECTION IN TRG AND TRG SIDES
    if (wlseqVec.size()==1){
      for (wlinkSequence::const_iterator seqit=wlseqVec.begin()->begin(); seqit!=wlseqVec.begin()->end();++seqit){
	unsigned int stateId=0;
	vector<pair<wlink,bool> > lnkvec(1,make_pair(*seqit,true));
 	state* currState = new state(stateId,trefined, lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
	stateGarbage.push_back(trefined);
	trefined=currState;
      }
    }else if (wlseqVec.size()>1){
      //put each combination in a stack to see which is best (starting from intersection)
      stateStack ststack;
      for (wlinkSequenceVec::const_iterator vit=wlseqVec.begin();vit!=wlseqVec.end();++vit){
	state* father=trefined;
	for (wlinkSequence::const_iterator seqit=vit->begin(); seqit!=vit->end();++seqit){
	  unsigned int stateId=0;
	  vector<pair<wlink,bool> > lnkvec(1,make_pair(*seqit,true));
	  state* currState = new state(stateId,father,lnkvec, possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
	  father=currState;
	  if (seqit<vit->end()-1){
	    stateGarbage.push_back(father);
	  }
	}
	ststack.cleverInsert(father);
	//cout<<"STACK :\n"<<ststack .print(possLinks)<<endl;
      }
      ++(*ststack.begin())->nPtr;
      stateGarbage.push_back(trefined);
      trefined=*(ststack.begin());
    }
  }
  //cout<<"trefined:\n"<<trefined->print(possLinks)<<endl;  
  // TAKE INTERSECTION
  vector<bool> sstcov=srefined->stcov;
  for (int li=0;li<sstcov.size();++li){
    if (sstcov[li] && !trefined->stcov[li]){
      unsigned int stateId=0;
      vector<pair<wlink,bool> > lnkvec(1,make_pair(possLinks[li],false));
      state* currState = new state(stateId,srefined, lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcNull);
      stateGarbage.push_back(srefined);
      srefined=currState;
    }
  }
  //cout<<"INTER:\n"<<inter->print(possLinks)<<endl;  
  //cout<<"REFINED:\n"<<srefined->print(possLinks)<<endl;  
  // delete everything created
  delete inter,trefined;
  srefined->id=0;
  return srefined;
}

  // BUILD SRC AND TRG POSSIBLE COMBINATIONS HASHES
  /*
  wlinkCombinations combs;
  hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq> srcComb,trgComb;
  // source possible combinations
  for (int n=0; n<srcLinks.size();++n){
    set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
    if (srcLinks[n].size()>1){
      // srcLinks are already sorted according to ID order
      unsigned int nlinks=2;
      while (nlinks<=srcLinks[n].size() && nlinks<=pars.m){
	vector<unsigned int> pos(nlinks);
	combs.combinations_recursive(srcLinks[n],*wlseqSet, "st", nlinks,pos,0,0);
	for (set<weightedLinkSequence,ltweightedlinkseq>::const_iterator sit=wlseqSet->begin();sit!=wlseqSet->end();++sit){
	  hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=srcComb.find(make_pair(sit->begin()->sbeg(),*sit));
	  if (hashit==srcComb.end()){
	    srcComb.insert(make_pair(sit->begin()->sbeg(),*sit));
	  }
	}
	++nlinks;
      }
    }else if (srcLinks[n].size()==1){
      hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=srcComb.find(make_pair(srcLinks[n].begin()->sbeg(),srcLinks[n]));
      if (hashit==srcComb.end()){
	srcComb.insert(make_pair(srcLinks[n].begin()->sbeg(),srcLinks[n]));
      }
    }
  }
  // target possible combinations
  for (int n=0; n<trgLinks.size();++n){
    set<weightedLinkSequence,ltweightedlinkseq>* wlseqSet=new set<weightedLinkSequence,ltweightedlinkseq>;
    if (trgLinks[n].size()>1){
      // trgLinks are already sorted according to ID order
      unsigned int nlinks=2;
      while (nlinks<=trgLinks[n].size() && nlinks<=pars.m){
	vector<unsigned int> pos(nlinks);
	combs.combinations_recursive(trgLinks[n],*wlseqSet, "ts", nlinks,pos,0,0);
	//combinations_recursiv(trgLinks[n],*wlseqSet, "ts", nlinks,pos,0,0);
	for (set<weightedLinkSequence,ltweightedlinkseq>::const_iterator sit=wlseqSet->begin();sit!=wlseqSet->end();++sit){
	  hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=trgComb.find(make_pair(sit->begin()->tbeg(),*sit));
	  if (hashit==trgComb.end()){
	    trgComb.insert(make_pair(sit->begin()->tbeg(),*sit));
	  }
	}
	++nlinks;
      }
    }else if (trgLinks[n].size()==1){
      hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=trgComb.find(make_pair(trgLinks[n].begin()->tbeg(),trgLinks[n]));
      if (hashit==trgComb.end()){
	trgComb.insert(make_pair(trgLinks[n].begin()->tbeg(),trgLinks[n]));
      }
    }
  }
  cout<<"Printing SRC Hash:"<<endl;
  for (hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=srcComb.begin();hashit!=srcComb.end();++hashit){
    cout<<"int:"<<hashit->first;
    for (wlinkSequence::const_iterator seqit=hashit->second.begin();seqit!=hashit->second.end();++seqit){
      cout<<"\t"<<seqit->printTalpDebug()<<endl;
    }
  }
  cout<<"Printing TRG Hash:"<<endl;
  for (hash_set<pair<int,wlinkSequence>,hashfintlinkseq,hasheqintlinkseq>::const_iterator hashit=trgComb.begin();hashit!=trgComb.end();++hashit){
    cout<<"int:"<<hashit->first;
    for (wlinkSequence::const_iterator seqit=hashit->second.begin();seqit!=hashit->second.end();++seqit){
      cout<<"\t"<<seqit->printTalpDebug()<<endl;
    }
  }
  */

//state* symmetrisation::grow ( state *s1, state *s2, const wlinkSequence & possLinks, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp,  param & pars, int level, bool diagonal=false,bool final=false,bool bothuncovered=false){
string symmetrisation::grow ( state *s1, state *s2, const wlinkCoverage & posiOrderLinkCoverage, const sentPair & sp, bool diagonal,bool final,bool bothuncovered, int level){

  ostringstream sout;
  vector <pair <int,int> > neighbors; //neighbors
  //  int* fa; //counters of covered foreign positions
  //  int* ea; //counters of covered english positions
  //  int** A; //alignment matrix with information symmetric/direct/inverse alignments
  int m=sp.numSrcToks(level);
  int n=sp.numTrgToks(level);
  vector<int> fa(m+1,0); //counters of covered foreign positions
  vector<int> ea(n+1,0); //counters of covered foreign positions
  //  vector<vector<stateStack> > stacks(toExpandList.size()+1,vector<stateStack>(nmultstacks));
  vector<vector<int> > A(n+1,vector<int>(m+1));
  //  int* ea; //counters of covered english positions
  //  int** A; //alignment matrix with information symmetric/direct/inverse alignments
 
  neighbors.push_back(make_pair(-1,-0));
  neighbors.push_back(make_pair(0,-1));
  neighbors.push_back(make_pair(1,0));
  neighbors.push_back(make_pair(0,1));
  
  if (diagonal){
    neighbors.push_back(make_pair(-1,-1));
    neighbors.push_back(make_pair(-1,1));
    neighbors.push_back(make_pair(1,-1));
    neighbors.push_back(make_pair(1,1));    
  }
  
  int i,j,o;

  //covered foreign and english positions    
  //  memset(fa,0,(m+1)*sizeof(int));
  //memset(ea,0,(n+1)*sizeof(int));
  //matrix to quickly check if one point is in the symmetric
  //alignment (value=2), direct alignment (=1) and inverse alignment (=-1) 
  // for (i=1;i<=n;i++) memset(A[i],0,(m+1)*sizeof(int));

  set <pair <int,int> > currentpoints; //symmetric alignment
  set <pair <int,int> > unionalignment; //union alignment   
  pair <int,int> point; //variable to store points
  set<pair <int,int> >::const_iterator k; //iterator over sets
  
  //fill in the links
  for (int l=0; l<s1->stcov.size();++l){
    if (s1->stcov.at(l) || s2->stcov.at(l)){    // union
      j=posiOrderLinkCoverage.src(l);
      i=posiOrderLinkCoverage.trg(l);
      unionalignment.insert(make_pair(i,j));
      if (s1->stcov.at(l) && s2->stcov.at(l)){   // intersection
	fa[j]=1;ea[i]=1;
	A[i][j]=2;   
	currentpoints.insert(make_pair(i,j));
      }else if (s1->stcov.at(l)){
	A[i][j]=-1;
      }else{ //s2->stcov.at(l)
	A[i][j]=1;
      }
    } // if (s1->stcov.at(l)
  } // for (int l=0; l<s1->lcov.size();++l){

  int added=1; 

  while (added){
    added=0;
    ///scan the current alignment
    for (k=currentpoints.begin();k!=currentpoints.end();k++){
      //      cout << "{"<< (k->second) << "-" << (k->first) << "}\n";
      for (o=0;o<neighbors.size();o++){
	//cout << "go over check all neighbors\n";
	point.first=k->first+neighbors[o].first;      
	point.second=k->second+neighbors[o].second; 
	//	cout << point.second << " " << point.first << "\n";
	//check if neighbor is inside 'matrix'
	if (point.first>0 && point.first <=n && point.second>0 && point.second<=m){
	  //check if neighbor is in the unionalignment alignment
	  linkIndex l;
	  if (posiOrderLinkCoverage.getIDifExists(point.second,point.first,l)){
	    if (s1->stcov.at(l) || s2->stcov.at(l)){
	      //cout << "In unionalignment\n";cout.flush();
	      //check if it connects at least one uncovered word
	      if (!(ea[point.first] && fa[point.second])){
		//insert point in currentpoints!
		currentpoints.insert(point);
		A[point.first][point.second]=2;
		ea[point.first]=1; fa[point.second]=1;
		added=1;
		//cout << "added grow: " << point.second << "-" << point.first << "\n";cout.flush();
	      }
	    }
	  }
	}
      }
    }
  } // while (added)

  if (final){
    for (k=unionalignment.begin();k!=unionalignment.end();k++){
      if (A[k->first][k->second]==1){
	point.first=k->first;point.second=k->second;
	//one of the two words is not covered yet
	//cout << "{" << point.second-1 << "-" << point.first-1 << "} ";
	if ((bothuncovered &&  !ea[point.first] && !fa[point.second]) || (!bothuncovered && !(ea[point.first] && fa[point.second]))){
	  //add it!
	  currentpoints.insert(point);
	  A[point.first][point.second]=2;
	  //keep track of new covered positions                
	  ea[point.first]=1;fa[point.second]=1;
	  //added=1;
	  //cout << "added final: " << point.second << "-" << point.first << "\n";
	}
      }
    }
    for (k=unionalignment.begin();k!=unionalignment.end();k++){
      if (A[k->first][k->second]==-1){            
	point.first=k->first;point.second=k->second;
	//one of the two words is not covered yet
	//cout << "{" << point.second-1 << "-" << point.first-1 << "} ";
	if ((bothuncovered &&  !ea[point.first] && !fa[point.second]) || (!bothuncovered && !(ea[point.first] && fa[point.second]))){
	  //add it!
	  currentpoints.insert(point);
	  A[point.first][point.second]=2;
	  //keep track of new covered positions                
	  ea[point.first]=1;fa[point.second]=1;
	  //added=1;
	  //cout << "added final: " << point.second << "-" << point.first << "\n";
	}
      }
    }      
  }
             
  for (k=currentpoints.begin();k!=currentpoints.end();k++){
    //sout << k->second-1 << "-" << k->first-1 << " ";
    sout << k->second << "-" << k->first << " ";
  }
  return sout.str();
}

string stateStack::print (const wlinkSequence & posslnks, const param & pars ) const {
  ostringstream oss("");
  for (stateStack::const_iterator st=this->begin(); st != this->end(); ++st){
    oss<<(**st).print(posslnks,pars)<<endl;	
  }
  return oss.str();
}

string stateStack::nbest (int numbest, const wlinkCoverage & posiOrderLinkCoverage, const phraseLinkTable & existingLinks, int level ) const {
  ostringstream oss("");
  stateStack::const_iterator st=this->begin();
  int i=0;
  while (i<numbest && st!=this->end()){
    for (int lid=(**st).first;lid<=(**st).last;++lid){
      if (lid>=0){
	if ((**st).stcov[lid]){
	  if (lid>(**st).first){oss<<" ";}
	  oss<<posiOrderLinkCoverage.src(lid)<<"-"<<posiOrderLinkCoverage.trg(lid);
	}
      }
    }
    oss<<endl;
    ++st;
    ++i;
  }
  return oss.str();
}

// convert links between phrases into links between words
string stateStack::onebest (const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars ) const {
  ostringstream oss("");
  stateStack::const_iterator st=this->begin();
  oss<<(**st).printAlignment (possLinks, srcLinksVec, trgLinksVec, direction, pars )<<endl;
  return oss.str();
}
string stateStack::nbest (const wlinkSequence & possLinks,  const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, string direction, const param & pars, int numbest, int numSentPair ) const {
  ostringstream oss("");
  stateStack::const_iterator st=this->begin();
  int i=0;
  while (i<numbest && st!=this->end()){
    oss<<(**st).printForNbest (possLinks, srcLinksVec, trgLinksVec, direction, pars,numSentPair )<<endl;
    ++i;
    ++st;
  }
  return oss.str();
}

string stateList::print (const wlinkSequence & posOrderPosslnks, const param & pars ) const {
    ostringstream oss("");
    for (stateList::const_iterator st=this->begin(); st != this->end(); ++st){
      oss<<(**st).print(posOrderPosslnks,pars)<<endl;	
    }
    return oss.str();
}

void nbestList::cleverInsert(state *_st, linkIndex nposlinks ){
  vector<int> lvec;
  if (_st->lcov.size()==0 && nposlinks>0){
    lvec.assign(nposlinks,0);
    _st->retrieveLinkCoverage(lvec);
    _st->setlcov(lvec);
  }
  hash_map<vector<int>,state*, hashfvecint,hasheqvecint>::iterator it=nbjash.find(_st->lcov);
  if (it==nbjash.end()){
    _st->father=NULL;
    nbstack.cleverInsert(_st);
    nbjash.insert(make_pair(_st->lcov,_st));
  }else{
    if (it->second != _st){
      // we always keep the most recent address for same alignment
      //cout<<"erasing"<<it->second<<endl;
      stateStack::iterator stit=nbstack.find(it->second);
      nbstack.erase(stit);
      --it->second->nPtr;
      if (it->second->nPtr==0){
	delete it->second;
      }
      _st->father=NULL;
      //cout<<"inserting"<<_st<<endl;
      nbstack.cleverInsert(--stit,_st);
      it->second=_st;
    }
  }
}
void nbestList::cleverInsert(const nbestList & nb, linkIndex nposlink){
  for (stateStack::const_iterator nbit=nb.nbstack.begin(); nbit!=nb.nbstack.end();++nbit){
    nbstack.cleverInsert(*nbit);
    //    if ((*nbit)->lcov.size()==0){
    //}
  }
}
void nbestList::clear(linkIndex nposlink){
  for (stateStack::iterator nbit=nbstack.begin(); nbit!=nbstack.end();++nbit){
    if ((*nbit)->lcov.size()==0){
      vector<int> lvec;
      lvec.assign(nposlink,0);
      (*nbit)->retrieveLinkCoverage(lvec);
    }
    hash_map<vector<int>,state*, hashfvecint,hasheqvecint>::iterator it=nbjash.find((*nbit)->lcov);
    nbjash.erase(it);
    nbstack.erase(nbit);
  }
}

unsigned long long int stateCoverage::getWordCoverageID(const vector<int> & scov, const vector<int> & tcov){
  pair<vector<int>,vector<int> > p(scov,tcov);
  hash_map<pair<vector<int>,vector<int> >, unsigned long long int, hashfpairvecintasbool,hasheqpairvecintasbool>::iterator hit=wordCoverageMap.find(p);
  if (hit==wordCoverageMap.end()){
    //add an entry
    unsigned long long int ind(wordCoverageVoc.size());
    wordCoverageVoc.push_back(p);
    wordCoverageMap.insert(make_pair(p,ind));
    return ind;
  }else{
    return hit->second;
  }
}
  
string stateCoverage::printWordCoverage(unsigned long long int id) const {
  //cerr<<"wordCoverageVoc.size:"<<wordCoverageVoc.size()<<" id:"<<id<<endl;
  pair<vector<int>,vector<int> > p=wordCoverageVoc.at(id);
  ostringstream oss("");
  oss<<"[";
  for (vector<int>::const_iterator it=p.first.begin(); it != p.first.end(); ++it){
    if (*it>0){oss<<"1";}
    else {oss<<"0";}
  }
  oss<<"]";
  oss<<"[";
  for (vector<int>::const_iterator it=p.second.begin(); it != p.second.end(); ++it){
    if (*it>0){oss<<"1";}
    else {oss<<"0";}
  }
  oss<<"]";

  return oss.str();
}
unsigned long long int stateCoverage::getMonolWordCoverageID(const vector<int> & cov){
  hash_map<vector<int>, unsigned long long int, hashfvecintasbool,hasheqvecintasbool>::iterator hit=monolWordCoverageMap.find(cov);
  if (hit==monolWordCoverageMap.end()){
    //add an entry
    unsigned long long int ind(monolWordCoverageVoc.size());
    monolWordCoverageVoc.push_back(cov);
    monolWordCoverageMap.insert(make_pair(cov,ind));
    return ind;
  }else{
    return hit->second;
  }
}
  
string stateCoverage::printMonolWordCoverage(unsigned long long int id) const {
  //cerr<<"monolWordCoverageVoc.size:"<<monolWordCoverageVoc.size()<<" id:"<<id<<endl;
  vector<int> cov=monolWordCoverageVoc.at(id);
  ostringstream oss("");
  oss<<"[";
  for (vector<int>::const_iterator it=cov.begin(); it != cov.end(); ++it){
    if (*it>0){oss<<"1";}
    else {oss<<"0";}
  }
  oss<<"]";
  return oss.str();
}

unsigned long long int multipleStacks::subStackIndex(const param & pars, bool sourceWordParse, state* _state, stateCoverage & stateCov) const {
  if (pars.multipleStacks == "target-number"){
    if (sourceWordParse){return _state->ntcov;}
    else {return _state->nscov;}
  }else if (pars.multipleStacks == "number"){
    return _state->nscov+_state->ntcov;
  }else if (pars.multipleStacks == "coverage"){
    return stateCov.getWordCoverageID(_state->scov,_state->tcov);
  }else if (pars.multipleStacks == "target-coverage"){
    if (sourceWordParse){return stateCov.getMonolWordCoverageID(_state->tcov);}
    else{return stateCov.getMonolWordCoverageID(_state->scov);}
  }else if (pars.multipleStacks == "source-coverage"){
    if (sourceWordParse){return stateCov.getMonolWordCoverageID(_state->scov);}
    else{return stateCov.getMonolWordCoverageID(_state->tcov);}
  }else{ // substackindex=="none"
    return 0;
  }
}
