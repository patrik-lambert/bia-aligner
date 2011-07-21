/*
 Author:  Patrik Lambert (lambert@gps.tsc.upc.edu)
 Description: BIlingual word Aligner

-----------------------------------------------------------------------

  Copyright 2006 by TALP Research Center
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <time.h>
#include <sys/times.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <set>
#include "params.h"
#include "defs.h"
#include "sentPair.h"
#include "scoreTable.h"
#include "wlink.h"
#include "state.h"

#include <stdlib.h>
#include <string.h>

#ifdef _SERVER
#include "ServerSocket.h"
#include "SocketException.h"
#endif

/**************************************************************************/
/**** GLOBAL DECLARATIONS *************************************************/
/**************************************************************************/
param pars;
vocTable svoc, tvoc; // oct 2010: vector<vocTable*> was replaced by vocTable (this assumes that all levels share the same voc)
struct tms *time_buffer = new tms;
float clocks_per_sec = sysconf (_SC_CLK_TCK);
ofstream foutput,foutput_st,foutput_ts,fnbest;
cost ibm1max (const wlink & phlnk, int level, wdAssoQScoreTable & wdas, phraseLinkTable & existingLinks, const sentPair & sp, const param & pars);
string decode (wlinkSequence & possLinks, wlinkCoverage & posiOrderLinkCoverage, const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec,phraseLinkTable & existingLinks, param & pars, nbestList & _alignments,string direction, const state & initAl, const sentPair & sp, int level,vector<pair<int,int> > & srcChFiltLims, vector<pair<int,int> > & trgChFiltLims);

int nBest=1;
double state_creation_time=0;

int main(int argc, char **argv){

  // READING PARAMETERS
  pars.read_params(argc,argv);
  pars.print_server_params();
  if (!pars.servermode) pars.print_client_params();

  // READ MODELS
  cerr << "loading word association table...";
  bool showvoc(true);
  phraseDetection sPhSearch,tPhSearch;
  // load word association model and load phrase vocabulary
  wdAssoQScoreTable wdas(pars.wordAsTab,sPhSearch,tPhSearch,svoc,tvoc); //OK
  //cerr << "WDAS:\n"<<wdas.print() << endl;
  cerr << "done!\n";

  matchStemSynTable mssTab;
  if (pars.mssTab != "-"){
    cerr << "loading match-stem-syn association table...";
    //insert entries in mssTab (to later find match, stem or syn) 
    mssTab.load(pars.mssTab,svoc,tvoc);
    cerr << "done!\n";
  }

  vocTable sTagVoc,tTagVoc;
  wdAssoQScoreTable tgas;
  if (pars.tagAsTab.compare("-")!=0 && (pars.wtga>0 || pars.servermode)){
    cerr << "loading tag association table...";
    tgas.load(pars.tagAsTab,sTagVoc,tTagVoc);
    cerr << "done!\n";
  }
  unlinkedModelTable sunlinkedModel; //word based
  if (pars.sumTab!="-" && (pars.wum>0 || pars.wums>0 || pars.servermode)){
    cerr << "loading source unlinked Model table...";
    sunlinkedModel.load(pars.sumTab,svoc);
    cerr << "done!\n";
  }
  unlinkedModelTable tunlinkedModel;
  if (pars.tumTab!="-" && (pars.wum>0 || pars.wumt>0 || pars.servermode)){
    cerr << "loading target unlinked Model table...";
    tunlinkedModel.load(pars.tumTab,tvoc);
    cerr << "done!\n";
  }
  fertTable sfertModel; //word based
  if (pars.sfertTab!="-" && (pars.wf>0 || pars.wfs>0 || pars.servermode)){
    cerr << "loading source fertility table...";
    sfertModel.load(pars.sfertTab,svoc);
    cerr << "done!\n";
  }
  fertTable tfertModel;
  if (pars.tfertTab!="-" && (pars.wf>0 || pars.wft>0 || pars.servermode)){
    cerr << "loading target fertility table...";
    tfertModel.load(pars.tfertTab,tvoc);
    cerr << "done!\n";
  }
  classFertTable sClFertModel;
  if (pars.sClFertTab!="-" && (pars.wf>0 || pars.wfs>0 || pars.servermode)){
    cerr << "loading source class fertility table...";
    sClFertModel.load(pars.sClFertTab,svoc,sTagVoc);
    cerr << "done!\n";
  }
  classFertTable tClFertModel;
  if (pars.tClFertTab!="-" && (pars.wf>0 || pars.wft>0 || pars.servermode)){
    cerr << "loading class target fertility table...";
    tClFertModel.load(pars.tClFertTab,tvoc,tTagVoc);
    cerr << "done!\n";
  }

#ifdef _SERVER
  cerr << "LAUNCHING server at PORT=" << pars.PORT << endl;
  
  try{
    ServerSocket server ( pars.PORT );
    while ( true ){
      cerr << "Server " << pars.PORT << ": Waiting..." << endl;
      ServerSocket new_sock;
      server.accept ( new_sock );
      try{	
	while ( true ){
	  string request;
	  new_sock >> request;
	  cerr << "Server " << pars.PORT << ": request=" << request << endl;
	  if (request.find("die")==0){
	    cerr << "Server " << pars.PORT << ": bye bye!" << endl;
	    new_sock << "bye bye!";
	    exit(0);
	  }
	  istringstream inStr(request);
	  pars.reset();
	  while (inStr){
	    //is it ost itgs itgt b t wwda1 wwda2 wwda3 wwda4 wwda5 wwda6 wtga wf wfs wft wlb wpp wup wus wut wcn wcl whp whs wht wzp w1to1 w1toN wNto1 w1toNsum wNtoM range verbose

	    // -iwda -itga  

	    string word;
	    inStr >> word;
	    if (!inStr) continue;
	    if (word.compare("-itgs")==0) { inStr >> word; pars.itgsfile=word; continue; }
	    if (word.compare("-itgt")==0)  { inStr >> word; pars.itgtfile=word; continue; }
	    // here a parameter beginning with -is or -it can ONLY be -isn or -itn with n an integer or ""
	    if (word.substr(0,3)=="-is")  { 
	      int index; if (word.substr(3).compare("")==0) index=0;
	      else index=atoi(word.substr(3).c_str());
	      inStr >> word; pars.isfile[index]=word; continue; }
	    if (word.substr(0,3)=="-it")  { 
	      int index; if (word.substr(3).compare("")==0) index=0;
	      else index=atoi(word.substr(3).c_str());
	      inStr >> word; pars.itfile[index]=word; continue; }
	    if (word.compare("-o")==0) { inStr >> word; pars.ofile=word; continue; }
	    if (word.compare("-ost")==0) { inStr >> word; pars.ostfile=word; continue; }
	    if (word.compare("-ots")==0) { inStr >> word; pars.otsfile=word; continue; }
	    if (word.compare("-lev")==0)  { inStr >> word; pars.nLevels=atoi(word.data()); continue; }
	    if (word.compare("-sinputtype")==0)  { inStr >> word; pars.sInputType=atoi(word.data()); continue; }
	    if (word.compare("-tinputtype")==0)  { inStr >> word; pars.tInputType=atoi(word.data()); continue; }
	    if (word.substr(0,2)=="-b")  { 
	      int index; if (word.substr(2).compare("")==0) index=0;
	      else index=atoi(word.substr(2).c_str());
	      inStr >> word; pars.b[index]=atoi(word.data()); continue; }
	    if (word.compare("-t")==0)  { inStr >> word; pars.t=atof(word.data()); continue; }
	    if (word.compare("-nms")==0)  { inStr >> word; pars.nms=atoi(word.data()); continue; }
	    if (word.compare("-n")==0)  { inStr >> word; pars.n=atoi(word.data()); continue; }
	    if (word.compare("-d")==0)  { inStr >> word; pars.d=atoi(word.data()); continue; }
	    if (word.compare("-f")==0)  { inStr >> word; pars.f=atoi(word.data()); continue; }
	    if (word.compare("-hr")==0)  { inStr >> word; pars.hr=atoi(word.data()); continue; }
	    if (word.compare("-m")==0)  { inStr >> word; pars.m=atoi(word.data()); continue; }
	    if (word.compare("-c")==0)  { inStr >> word; pars.c=atoi(word.data()); continue; }
	    if (word.compare("-ct")==0)  { inStr >> word; pars.ct=atof(word.data()); continue; }
	    if (word.compare("-init")==0)  { inStr >> word; pars.init=word; continue; }
	    if (word.compare("-first")==0)  { inStr >> word; pars.first=word; continue; }
	    if (word.compare("-phlink-count")==0)  { inStr >> word; pars.phLinkWeight=word; continue; }
	    if (word.compare("-one-to-one")==0)  { inSrt >> word; if (word!="0"){pars.oneToOne=true;} continue; }
	    if (word.compare("-backtrack")==0)  { inSrt >> word; if (word!="0"){pars.backTrack=true;} continue; }
	    if (word.compare("-phas")==0)  { inStr >> word; pars.phas=word; continue; }
	    if (word.compare("-exp")==0)  { inStr >> word; pars.exp=word; continue; }
	    if (word.compare("-dir")==0)  { inStr >> word; pars.direction=word; continue; }
	    if (word.compare("-accu")==0)  { pars.accu=true; continue; }
	    if (word.compare("-doba")==0)  { inStr >> word; pars.doba=atoi(word.data()); continue; }
	    if (word.compare("-aoba")==0)  { inStr >> word; pars.avgeOnlyBestAsso=atoi(word.data()); continue; }
	    if (word.substr(0,6)=="-wwda1")  { 
	      int index; if (word.substr(6).compare("")==0) index=0;
	      else index=atoi(word.substr(6).c_str());
	      inStr >> word; pars.wwda1[index]=atof(word.data()); 
	      if (pars.wwda1[index] == 0) --pars.nmodels[index];continue; }
	    if (word.compare("-wwda2")==0)  { 
	      inStr >> word; pars.wwda2=atof(word.data()); 
	      if (pars.wwda2 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wwda3")==0)  { 
	      inStr >> word; pars.wwda3=atof(word.data()); 
	      if (pars.wwda3 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wwda4")==0)  { 
	      inStr >> word; pars.wwda4=atof(word.data()); 
	      if (pars.wwda4 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wwda5")==0)  { 
	      inStr >> word; pars.wwda5=atof(word.data()); 
	      if (pars.wwda5 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wwda6")==0)  { 
	      inStr >> word; pars.wwda6=atof(word.data()); 
	      if (pars.wwda6 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wtga")==0)  { 
	      inStr >> word; pars.wtga=atof(word.data()); 
	      if (pars.wtga != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wrk")==0)  { 
	      inStr >> word; pars.wrk=atof(word.data()); 
	      if (pars.wrk != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wrks")==0)  { 
	      inStr >> word; pars.wrks=atof(word.data()); 
	      if (pars.wrks != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wrkt")==0)  { 
	      inStr >> word; pars.wrkt=atof(word.data()); 
	      if (pars.wrkt != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wrkb")==0)  { 
	      inStr >> word; pars.wrkb=atof(word.data()); 
	      if (pars.wrkb != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wrkw")==0)  { 
	      inStr >> word; pars.wrkw=atof(word.data()); 
	      if (pars.wrkw != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wum")==0)  { 
	      inStr >> word; pars.wum=atof(word.data()); 
	      if (pars.wum != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wums")==0)  { 
	      inStr >> word; pars.wums=atof(word.data()); 
	      if (pars.wums != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wumt")==0)  { 
	      inStr >> word; pars.wumt=atof(word.data()); 
	      if (pars.wumt != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wf")==0)  { 
	      inStr >> word; pars.wf=atof(word.data()); 
	      if (pars.wf != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wfs")==0)  { 
	      inStr >> word; pars.wfs=atof(word.data()); 
	      if (pars.wfs != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wft")==0)  { 
	      inStr >> word; pars.wft=atof(word.data()); 
	      if (pars.wft != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wchfilt")==0)  { 
	      inStr >> word; pars.wchfilt=atof(word.data()); 
	      if (pars.wchfilt != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wchfilts")==0)  { 
	      inStr >> word; pars.wchfilts=atof(word.data()); 
	      if (pars.wchfilts != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wchfiltt")==0)  { 
	      inStr >> word; pars.wchfiltt=atof(word.data()); 
	      if (pars.wchfiltt != 0) ++pars.nmodels[0];continue; }
	    if (word.substr(0,4)=="-wlb")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wlb[index]=atof(word.data()); 
	      if (pars.wlb[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wpp")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wpp[index]=atof(word.data()); 
	      if (pars.wpp[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wup")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wup[index]=atof(word.data()); 
	      if (pars.wup[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wus")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wus[index]=atof(word.data()); 
	      if (pars.wus[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wut")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wut[index]=atof(word.data()); 
	      if (pars.wut[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wcn")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wcn[index]=atof(word.data()); 
	      if (pars.wcn[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wcl")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wcl[index]=atof(word.data()); 
	      if (pars.wcl[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-whp")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.whp[index]=atof(word.data()); 
	      if (pars.whp[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-whs")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.whs[index]=atof(word.data()); 
	      if (pars.whs[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wht")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wht[index]=atof(word.data()); 
	      if (pars.wht[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.substr(0,4)=="-wzp")  { 
	      int index; if (word.substr(4).compare("")==0) index=0;
	      else index=atoi(word.substr(4).c_str());
	      inStr >> word; pars.wzp[index]=atof(word.data()); 
	      if (pars.wzp[index] != 0) ++pars.nmodels[index];continue; }
	    if (word.compare("-w1to1")==0)  { 
	      inStr >> word; pars.w1to1=atof(word.data()); 
	      if (pars.w1to1 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-w1toN")==0)  { 
	      inStr >> word; pars.w1toN=atof(word.data()); 
	      if (pars.w1toN != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wNto1")==0)  { 
	      inStr >> word; pars.wNto1=atof(word.data()); 
	      if (pars.wNto1 != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-w1toNsum")==0)  { 
	      inStr >> word; pars.w1toNsum=atof(word.data()); 
	      if (pars.w1toNsum != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-wNtoM")==0)  { 
	      inStr >> word; pars.wNtoM=atof(word.data()); 
	      if (pars.wNtoM != 0) ++pars.nmodels[0];continue; }
	    if (word.compare("-outputtype")==0){ inStr >> word; pars.outputType=word; continue; }
	    if (word.compare("-n-best-list")==0){ inStr >> word; pars.nbestFile=word; inStr >> word; pars.nbestSize=atoi(word.data()); continue; }
	    if (word.compare("-range")==0){ inStr >> word; pars.range=word; continue; }
	    if (word.compare("-verbose")==0){ inStr >> word; pars.verbose=atoi(word.data()); continue; }
	  } // while inStr
  //****************************************
  // checking parameters
  //****************************************
	  vector<string> isfileTmp;
	  for (vector<string>::iterator v=pars.isfile.begin(); v!=pars.isfile.end();++v){
	    if ((*v).compare("-")!=0) isfileTmp.push_back(*v);
	  }
	  pars.isfile=isfileTmp;
	  vector<string> itfileTmp;
	  for (vector<string>::iterator v=pars.itfile.begin(); v!=pars.itfile.end();++v){
	    if ((*v).compare("-")!=0) itfileTmp.push_back(*v);
	  }
	  pars.itfile=itfileTmp;

	  //vector<string>::size_type nLevels=pars.isfile.size();
	  pars.wwda1.erase(pars.wwda1.begin()+pars.nLevels,pars.wwda1.end());
	  pars.wlb.erase(pars.wlb.begin()+pars.nLevels,pars.wlb.end());
	  pars.wpp.erase(pars.wpp.begin()+pars.nLevels,pars.wpp.end());
	  pars.wup.erase(pars.wup.begin()+pars.nLevels,pars.wup.end());
	  pars.wus.erase(pars.wus.begin()+pars.nLevels,pars.wus.end());
	  pars.wut.erase(pars.wut.begin()+pars.nLevels,pars.wut.end());
	  pars.wcn.erase(pars.wcn.begin()+pars.nLevels,pars.wcn.end());
	  pars.wcl.erase(pars.wcl.begin()+pars.nLevels,pars.wcl.end());
	  pars.whp.erase(pars.whp.begin()+pars.nLevels,pars.whp.end());
	  pars.whs.erase(pars.whs.begin()+pars.nLevels,pars.whs.end());
	  pars.wht.erase(pars.wht.begin()+pars.nLevels,pars.wht.end());
	  pars.wzp.erase(pars.wzp.begin()+pars.nLevels,pars.wzp.end());
	  pars.nmodels.erase(pars.nmodels.begin()+pars.nLevels,pars.nmodels.end());

	  if (pars.wwda1[0]==0 && pars.wwda2==0 && pars.wwda3==0 && pars.wwda4==0 && pars.wwda5==0 && pars.wwda6==0){
	    string err("All word association weights are 0 ");
	    pars.error_params(err);
	  }
	  if (pars.wup[0]==0 && pars.wlb[0]==0 && pars.wus[0]==0 && pars.wut[0]==0 && pars.wum==0 && pars.wums==0 && pars.wumt==0 &&pars.wf==0 && pars.wfs==0 && pars.wft==0){
	    string err("Link bonus, unlinked word penalty and fertility models can't all have 0 weight.");
	    pars.error_params(err);
	  }
	  for (int lev=1; lev<pars.nLevels;++lev){
	    if (pars.wup[lev]==0 && pars.wlb[lev]==0 && pars.wus[lev]==0 && pars.wut[lev]==0){
	      string err("Level $lev: link bonus and unlinked word penalty models can't both have 0 weight.");
	      pars.error_params(err);
	    }
	  }
	  pars.print_client_params();
#endif
	  //for (vector<int>::iterator it=pars.nmodels.begin();it!=pars.nmodels.end();++it){cout<<"nmodels:"<<*it<<endl;}
	  (void) times (time_buffer);
	  double tstart = time_buffer->tms_utime / clocks_per_sec;
	  double decoding_time=0;

	  // OPEN OUTPUT FILE(s)
	  if ( pars.ofile.compare("-")!=0 ){
	    foutput.open( (pars.ofile).data() );
	    if( !foutput.is_open() ) {
	      cerr << "BIA: Error opening output file=" << pars.ofile << endl;
	      exit(0);
	    }
	  }
	  if ( pars.ostfile.compare("-")!=0 ){
	    foutput_st.open( (pars.ostfile).data() );
	    if( !foutput_st.is_open() ) {
	      cerr << "BIA: Error opening output file=" << pars.ostfile << endl;
	      exit(0);
	    }
	  }
	  if ( pars.otsfile.compare("-")!=0 ){
	    foutput_ts.open( (pars.otsfile).data() );
	    if( !foutput_ts.is_open() ) {
	      cerr << "BIA: Error opening output file=" << pars.otsfile << endl;
	      exit(0);
	    }
	  }
	  // n-best-list file
	  if ( pars.nbestFile.compare("-")!=0 ){
	    fnbest.open( (pars.nbestFile).data() );
	    if( !fnbest.is_open() ) {
	      cerr << "BIA: Error opening n-best-list file=" << pars.nbestFile << endl;
	      exit(0);
	    }
	  }

	  // OPEN SOURCE & TARGET INPUT
	  // open corpus files and go to first sentence pair of range
	  bilCorpus corp(pars.isfile,pars.itfile,pars.itgsfile,pars.itgtfile,pars.range,pars.sInputType,pars.tInputType);
	  sentPair sp;
	  while (corp.getNextSentPair(sp,svoc,tvoc,pars.sInputType,pars.tInputType,sTagVoc,tTagVoc) ){
	    if (!sunlinkedModel.empty() && (pars.wum>0 || pars.wums>0)) sp.loadSrcUnlinkedModel(sunlinkedModel);
	    if (!tunlinkedModel.empty() && (pars.wum>0 || pars.wumt>0)) sp.loadTrgUnlinkedModel(tunlinkedModel);
	    if (!sfertModel.empty() && (pars.wf>0 || pars.wfs>0)) sp.loadSrcFert(sfertModel,pars.meshLinkCost);
	    if (!tfertModel.empty() && (pars.wf>0 || pars.wft>0)) sp.loadTrgFert(tfertModel,pars.meshLinkCost);
	    if (!sClFertModel.empty() && (pars.wf>0 || pars.wfs>0)) sp.loadSrcClassFert(sClFertModel);
	    if (!tClFertModel.empty() && (pars.wf>0 || pars.wft>0)) sp.loadTrgClassFert(tClFertModel);
	    if (pars.verbose>0){
	      ostringstream oss("");
	      oss << "sentence pair "<<corp.getSentPairNum()<<":\n"<<sp.display()<<"\n";
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();	 
	    }

	    // DETECT PHRASES IN SOURCE AND TARGET SENTENCES
	    uniqAlUnitColumnCollection sphfound,tphfound;
	    sPhSearch.searchSentence(sp.src(),sphfound);
	    //cout<<"SOURCE PHRASES FOUND\n"<<sphfound.print()<<endl;
	    sp.addSrcFoundPhrases(sphfound);
	    tPhSearch.searchSentence(sp.trg(),tphfound);
	    //cout<<"TARGET PHRASES FOUND\n"<<tphfound.print()<<endl;
	    sp.addTrgFoundPhrases(tphfound);
	    if (pars.verbose>0){
	      ostringstream oss("");
	      oss <<"ALUNITS\n"<<sp.printAlUnits(svoc,tvoc)<<endl;
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();
	    }

	    // PRUNE ASSO SCORE TABLE FOR THIS SENTENCE PAIR
	    wdAssoScoreTable spWdas(wdas,mssTab,sp,pars);
	    if (pars.verbose>0){
	      ostringstream oss("");
	      oss <<"ASSOCIATION SCORE TABLE (size:"<<spWdas.size()<<"):\n"<<spWdas.display(svoc,tvoc);
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();
	    }

	    // LIST OF POSSIBLE LINKS
	    // links should be something like 1,2-4 (vector<int> s,t)
	    
	    wordLinkSequenceVec srcLinksVec(pars.nLevels);
	    wordLinkSequenceVec trgLinksVec(pars.nLevels);
	    for (wordLinkSequenceVec::size_type lev=0;lev<pars.nLevels;++lev){
	      srcLinksVec.initLevel(lev,sp.numSrcToks(lev)); // will contain the set of possible links for each source position
	      trgLinksVec.initLevel(lev,sp.numTrgToks(lev)); // will contain the set of possible links for each target position
	    }
	    wlinkSequencePtrVec possLinks(pars.nLevels);
	    wlinkCoverage posiOrderLinkCoverage; // TODO for level>0
	    vector<linkIndex> linkId(pars.nLevels,0);
	    phraseLinkTable existingLinks;
	    for (uniqAlUnitColumnCollection::const_iterator sit=sp.srcAlUnits().begin(); sit != sp.srcAlUnits().end(); ++sit){
	      int sBegPosi=sit->firstPosi();
	      int sEndPosi=sit->lastPosi();
	      for (uniqAlUnitColumnCollection::const_iterator tit=sp.trgAlUnits().begin();tit != sp.trgAlUnits().end(); ++tit){
		int tBegPosi=tit->firstPosi();
		int tEndPosi=tit->lastPosi();
		//cerr<<sBegPosi<<","<<sEndPosi<<"-"<<tBegPosi<<","<<tEndPosi<<endl;
		//see if there is some link possible for these two columns
		float sProbSum=sit->probSum();
		float tProbSum=tit->probSum();
		float match=0; float stem=0; float syn=0;
		bool existsAssoScore=false;
		cnColumn<wlink> linkMesh;
		int lkMeshId=0;
		int sColPosi=0;
		// go through alUnits in column of alUnits beginning at some position and of a given size
		for (uniqAlUnitColumn::const_iterator scit=sit->begin(); scit != sit->end();++scit){
		  float sProb=scit->second;
		  int tColPosi=0;
		  for (uniqAlUnitColumn::const_iterator tcit=tit->begin(); tcit != tit->end();++tcit){
		    float tProb=tcit->second;
		    //if score exists for these words, we put them in the list
		    wdAssoScore lScore;
		    if (spWdas.assoScore(scit->first,tcit->first,lScore)){
		      existsAssoScore=true;
		      matchStemSyn mss;
		      bool match,stem,syn;
		      if (mssTab.getMatchStemSyn(scit->first,tcit->first,match,stem,syn)){
		      }else{
			match=0; stem=0; syn=0;
		      }
		      wlink lnk(sBegPosi,sEndPosi,tBegPosi,tEndPosi,lkMeshId,lScore.vcosts,pars,lScore.trgRankForSrc,lScore.srcRankForTrg,match,stem,syn);
		      if (pars.meshLinkCost == "interp"){match=0; stem=0; syn=0;}
		      ++lkMeshId;
		      linkMesh.push_back(lnk,sProb * tProb / (sProbSum * tProbSum) ); //normalised probs
		      //cout << "sProb" <<sProb<<" tProb"<<tProb<<"  word link:" << lnk.printVoc(sp,0) <<endl;
		    } // if spWdas.assoScore
		    
		    // TODO FOR CN INPUT
		    //vector<cost> pofsCosts;
		    //if (pars.wtga>0){
		    //  if (tgas.cstVec(*scit,*tcit,pofsCosts)){
		    //	lnk.setPofsCost(pofsCosts[0]);
		    //  }else{
		    //	lnk.setPofsCost(10.0);
		    //  }
		    //}
		    tColPosi++;
		  }
		  sColPosi++;
		}

		if (existsAssoScore){
		  //if (sBegPosi<sEndPosi||tBegPosi<tEndPosi){
		  //  cerr<<"Phrase link in Sentence Pair:"<<corp.getSentPairNum()<<endl;
		  //}
		  wlink lnk(sBegPosi,sEndPosi,tBegPosi,tEndPosi,linkId[0]);
		  // insert individual (src,trg) pairs in wlinkCoverage class
		  for (int sposi=sBegPosi;sposi<=sEndPosi;++sposi){
		    for (int tposi=tBegPosi;tposi<=tEndPosi;++tposi){
		      posiOrderLinkCoverage.addLink(sposi,tposi);
		    }
		  }
		  linkMesh.merge(lnk,pars.meshLinkCost); 
		  //cout << "merged link:" << lnk.printVoc(sp,0) <<endl;
		  possLinks[0]->put(lnk);
		  srcLinksVec[0][sEndPosi].second.insert(lnk);
		  trgLinksVec[0][tEndPosi].second.insert(lnk);
		  // insert word-to-word link cost in a hash
		  //if (sBegPosi==sEndPosi && tBegPosi==tEndPosi){
		  //  1to1LinkCost.insert(make_pair

		  // TODO for phrase-base links
		  if (pars.nLevels>0){
		    for (int lev=1; lev<pars.nLevels;++lev){
		      wlink phlnk(sp.srcat(sBegPosi).getPos(lev),sp.trgat(tBegPosi).getPos(lev),linkId[lev]);
		      //cout<<"wd link:"<<lnk.printDebug(sp)<<"\n\tphrase link:"<<phlnk.printDebug(sp)<<endl;
		      //cout<<"TABLE (before):"<<existingLinks.display(sp)<<endl;
		      if (existingLinks.put (phlnk,lev, lnk)){
			//cout<<"\texisting\n";
			++linkId[lev];
			possLinks[lev]->put(phlnk);
		      }
		      //cout<<"TABLE (after):"<<existingLinks.display(sp)<<endl;
		    }
		  }
		  // END: TODO for phrase-base links
		  ++linkId[0];
		  //fflush(stdout);
		} //if existsAssoScore
	      } //for tit
	    } //for sit

	    // create an index for the individual (src,trg) pairs covered in the possible links, and mappings (src,trg)->index and index->(src,trg)
	    posiOrderLinkCoverage.buildIndex();
	    if (pars.backTrack){
	      posiOrderLinkCoverage.addAssociatedPbLinks(*possLinks.at(0));
	    }
	    // IBMMAX COSTS FOR LEVEL>0 LINKS
	    // TO DO for phrase-based links
	    if (pars.phas.substr(0,2)=="im"){
	      ostringstream oss("");
	      for (int lev=1; lev<pars.nLevels; ++lev){
		for (wlinkSequence::iterator lit=possLinks[lev]->begin();lit!=possLinks[lev]->end();++lit){
		  // look if this link is in existing links hash
 		  hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator exlit=existingLinks.jash_phLink.find(make_pair(*lit,lev));
		  cost curCst;
		  if (exlit!=existingLinks.jash_phLink.end()){
		    if (exlit->second->isSet()){curCst=exlit->second->cst;}
		    else{
		      curCst=ibm1max(*lit,lev,wdas,existingLinks,sp,pars);
		      exlit->second->setCst(curCst);
		      //if (pars.verbose>2){oss<<lit->printTalpDebug()<<" ibm1max:"<<exlit->second->cst<<endl;}
		    }
		  }
		  lit->cst=curCst;lit->costs.push_back(curCst);lit->costs.push_back(curCst);lit->costs.push_back(curCst);
		}
	      }
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();	 
	    }
	    // END: TO DO for phrase-based links

	    // TARGET-SOURCE ORDERED POSSLINKS
	    //wlinkSequencePtrVec tsposiOrderPossLinks(pars.nLevels);
	    //for (int lev=0; lev<pars.nLevels;++lev){
	    //tsposiOrderPossLinks[lev]=possLinks[lev];
	    //stable_sort(tsposiOrderPossLinks[lev].begin(), tsposiOrderPossLinks[lev].end(),ltwlinkidts());
	    //}

	    // INTRODUCING LINKS IN  SRCLINKS, TRGLINKS FOR LEVEL>0
	    for (int lev=1; lev<pars.nLevels; ++lev){
	      for (wlinkSequence::const_iterator seqit=possLinks[lev]->begin();seqit!=possLinks[lev]->end();++seqit){
		srcLinksVec[lev][seqit->sbeg()].second.insert(*seqit);
		trgLinksVec[lev][seqit->tbeg()].second.insert(*seqit);
	      }
	    }

	    // DETERMINING WHICH WORDS MAY BE EXPANDED FIRST
	    for (int lev=0; lev<pars.nLevels; ++lev){
	      float srcAve=possLinks[lev]->size()*1.0/srcLinksVec[lev].size(); //average number of links per word position
	      float trgAve=possLinks[lev]->size()*1.0/trgLinksVec[lev].size(); //average number of links per word position
	      if (pars.verbose>0){
		ostringstream oss("");
		oss<<"level:"<<lev<<" srcAve:"<<srcAve<<" trgAve:"<<trgAve<<endl;
		if( foutput.is_open() ) foutput << oss.str();
		else cerr << oss.str();	 
	      }
	      for (int n=0; n<srcLinksVec[lev].size();++n){ //for each source word position
		if ( (pars.first == "avge" || (pars.first=="prev" && lev==0) || (pars.first=="tags" && lev==0) ) && srcLinksVec[lev][n].second.size()>srcAve){srcLinksVec[lev][n].first=1;}
		else if (pars.first == "tags" && lev>0){
		  string stform=sp.srcat(sp.srcWdPosis(lev,n).at(0)).getStform(lev);
		  if (stform=="NP" || stform=="VP" || stform=="ADJP"){srcLinksVec[lev][n].first=0;}
		  //if (stform=="NP"){srcLinksVec[lev][n].first=0;}
		  else {srcLinksVec[lev][n].first=1;}
		}else if (pars.first == "prev" && lev>0){
		  int first=1;
		  for (vector<int>::const_iterator it=sp.srcWdPosis(lev,n).begin();it!=sp.srcWdPosis(lev,n).end();++it){
		    if (srcLinksVec[lev-1][*it].first==0){first=0;break;}
		  }
		  srcLinksVec[lev][n].first=first;
		}
		else{srcLinksVec[lev][n].first=0;}
	      }
	      for (sentence::size_type n=0; n<trgLinksVec[lev].size();++n){
		if ( (pars.first == "avge" || (pars.first=="prev" && lev==0)|| (pars.first=="tags" && lev==0) ) && trgLinksVec[lev][sp.trgat(n).getPos(lev)].second.size()>trgAve){trgLinksVec[lev][sp.trgat(n).getPos(lev)].first=1;}
		else if (pars.first == "tags" && lev>0){
		  string stform=sp.trgat(sp.trgWdPosis(lev,n).at(0)).getStform(lev);
		  if (stform=="NP" || stform=="VP" || stform=="ADJP"){trgLinksVec[lev][n].first=0;}
		  else {trgLinksVec[lev][n].first=1;}
		}else if (pars.first == "prev" && lev>0){
		  int first=1;
		  for (vector<int>::const_iterator it=sp.trgWdPosis(lev,n).begin();it!=sp.trgWdPosis(lev,n).end();++it){
		    if (trgLinksVec[lev-1][*it].first==0){first=0;break;}
		  }
		  trgLinksVec[lev][n].first=first;
		  
		}
		else{trgLinksVec[lev][sp.trgat(n).getPos(lev)].first=0;}
	      }
	    }

	    if (pars.verbose>0){
	      ostringstream oss("");
	      oss<<"src links:\n"<<srcLinksVec.display(sp,showvoc)<<endl;
	      oss<<"trg links:\n"<<trgLinksVec.display(sp,showvoc)<<endl;
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();	 
	    }
	    if (pars.verbose>0){
	      ostringstream oss("");
	      oss<<"Possible Links:\n";
	      oss<<possLinks.display(sp,showvoc)<<endl;
	      if( foutput.is_open() ) foutput << oss.str();
	      else cerr << oss.str();	 
	    }

	    // DECODING
	    //***********************************************************************************
	    //Initial Alignment
	    // create empty alignment and add to alignments

	    int level=pars.nLevels-1;
	    stateList stateGarbage;
	    state* initAl=new state;
	    stateGarbage.push_back(initAl);
	    //cout<<"_initAl:"<<initAl<<endl;
	    initAl->setEmptyAl(posiOrderLinkCoverage.size(),possLinks[level]->size(),sp,level,pars);

	    vector<string> directions;
	    string::size_type beg=0;
	    string::size_type loc = pars.direction.find("-",beg);
 	    while (loc!=string::npos){
	      //cout<<": beg:"<<beg<<" loc:"<<loc<<" subst:"<<str.substr(beg,loc-beg)<<endl;
	      directions.push_back(pars.direction.substr(beg,loc-beg));
	      beg=loc+1;
	      loc=pars.direction.find("-",beg);
	    }
	    directions.push_back(pars.direction.substr(beg));

	    // no possible link
   	    if (possLinks[pars.nLevels-1]->size()==0){
  	      if( foutput.is_open() ) foutput << endl;
  	      else cerr << endl;	 
	      if ( fnbest.is_open() ) fnbest << initAl->printForNbest(*possLinks[level], srcLinksVec, trgLinksVec, directions[directions.size()-1],pars,corp.getSentPairNum()-1 )<<endl;
	      continue;
  	    }

	    ostringstream oss("");
	    nbestList alignments;
	    vector<pair<int,int> > trgChFiltLimits,srcChFiltLimits;
	    stateStack copiedStates;
	    int cntDirs=0;
	    for (vector<string>::const_iterator dirit=directions.begin();dirit!=directions.end();++dirit){
	      if (cntDirs<directions.size()-1 && pars.doba>0) pars.dOnlyBestAsso=pars.doba;
	      else pars.dOnlyBestAsso=0;
	      bool accu(false);
	      bool asrcParse(false),atrgParse(false),srcParse(false),trgParse(false);
	      if (*dirit == "as" || *dirit=="a" || *dirit=="asIat" || *dirit=="asRIat" || *dirit=="asUat"|| *dirit=="asRat" || *dirit=="asGDFAat"){asrcParse=true;}
	      if (*dirit == "at" || *dirit=="asIat"|| *dirit=="asRIat" ||*dirit=="asUat"|| *dirit=="asRat" || *dirit=="asGDFAat"){atrgParse=true;}
	      if (*dirit == "st" || *dirit =="stts"|| *dirit=="stIts" || *dirit=="stRIts"|| *dirit=="stUts"|| *dirit=="stRts"){srcParse=true;}
	      if (*dirit == "ts" || *dirit =="stts"|| *dirit=="stIts" || *dirit=="stRIts"|| *dirit=="stUts"|| *dirit=="stRts"){trgParse=true;}
	      nbestList asAlignments,atAlignments,stAlignments,tsAlignments;
	      if (*dirit=="stts" || pars.accu) accu=true;
	      if (asrcParse){
		if (pars.verbose>0){oss<<"Traversing search space following association score order\n";}

		oss<<decode(*possLinks[level], posiOrderLinkCoverage, srcLinksVec, trgLinksVec, existingLinks, pars, asAlignments,"as",*initAl,sp,level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"AS PARSE N-BEST LIST:"<<asAlignments.print(*possLinks[level],pars);}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(asAlignments,possLinks[level]->size());
	      }
	      (void) times (time_buffer);
	      double t1 = time_buffer->tms_utime / clocks_per_sec;
	      if (atrgParse){
		if (pars.verbose>0){oss<<"Traversing search space following association score order\n";}
		oss<<decode(*possLinks[level], posiOrderLinkCoverage, srcLinksVec, trgLinksVec, existingLinks, pars, atAlignments,"at",*initAl,sp,level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"AT PARSE N-BEST LIST:"<<atAlignments.print(*possLinks[level],pars);}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(atAlignments,possLinks[level]->size());
	      }
	      (void) times (time_buffer);
	      double t2 = time_buffer->tms_utime / clocks_per_sec;
	      decoding_time+=t2-t1;
	      if (srcParse){
		if (pars.verbose>0){oss<<"Traversing search space following source sentence and for each source word, association score order\n";}
		oss<<decode(*possLinks[level], posiOrderLinkCoverage, srcLinksVec, trgLinksVec, existingLinks, pars, stAlignments,"st",*initAl,sp,level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"ST PARSE N-BEST LIST:"<<stAlignments.print(*possLinks[level],pars);}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(stAlignments,possLinks[level]->size());
	      }
	      if (trgParse){
		if (pars.verbose>0){oss<<"Traversing search space following target sentence and for each target word, association score order\n";}
		oss<<decode(*possLinks[level], posiOrderLinkCoverage, srcLinksVec, trgLinksVec, existingLinks, pars, tsAlignments,"ts",*initAl,sp,level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"TS PARSE N-BEST LIST:"<<tsAlignments.print(*possLinks[level],pars);}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(tsAlignments,possLinks[level]->size());
	      }
	      //oss<< existingLinks.display(sp)<<endl;
	      
	      if (*dirit=="stIts"){
		symmetrisation symm;
		state* inter=symm.getIntersection(*stAlignments.nbstack.begin(),*tsAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking intersection of 'st' and 'ts' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(inter,possLinks[level]->size());
		if (pars.verbose>0){oss << "INTERSECTION:"<<inter->print(*possLinks[level],pars)<<endl;}
		initAl=inter;
		if ((pars.wchfilt>0 || pars.wchfilts>0) && cntDirs<directions.size()-1) {
		  inter->calculateSrcChFiltLimits(srcChFiltLimits,sp,*possLinks[level],level);
		}
		if ((pars.wchfilt>0 || pars.wchfiltt>0) && cntDirs<directions.size()-1) {
		  inter->calculateTrgChFiltLimits(trgChFiltLimits,sp,*possLinks[level],level);
		}
	      }else if (*dirit=="stRIts"){
		symmetrisation symm;
		state* inter=symm.getRefinedIntersection(*stAlignments.nbstack.begin(),*tsAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking refined intersection of 'st' and 'ts' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(inter,possLinks[level]->size());
		if (pars.verbose>0){oss << "REFINED INTERSECTION:"<<inter->print(*possLinks[level],pars)<<endl;}
		initAl=inter;
		if ((pars.wchfilt>0 || pars.wchfilts>0) && cntDirs<directions.size()-1) {
		  inter->calculateSrcChFiltLimits(srcChFiltLimits,sp,*possLinks[level],level);
		}
		if ((pars.wchfilt>0 || pars.wchfiltt>0) && cntDirs<directions.size()-1) {
		  inter->calculateTrgChFiltLimits(trgChFiltLimits,sp,*possLinks[level],level);
		}
	      }else if (*dirit=="asIat"){
		symmetrisation symm;
		state* inter=symm.getIntersection(*asAlignments.nbstack.begin(),*atAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking intersection of 'as' and 'at' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(inter,possLinks[level]->size());
		if (pars.verbose>0){oss << "INTERSECTION:"<<inter->print(*possLinks[level],pars)<<endl;}
		initAl=inter;
		if ((pars.wchfilt>0 || pars.wchfilts>0) && cntDirs<directions.size()-1) {
		  inter->calculateSrcChFiltLimits(srcChFiltLimits,sp,*possLinks[level],level);
		  if (pars.verbose>1){
		    int ccnt=0;
		    for (vector<pair<int,int> >::const_iterator vit=srcChFiltLimits.begin(); vit!=srcChFiltLimits.end();++vit){
		      oss<<"src chunk "<<ccnt<<":["<<vit->first<<","<<vit->second<<"]\n";
		      ++ccnt;
		    }
		  }
		}
	      }else if (*dirit=="asRIat"){
		symmetrisation symm;
		state* inter=symm.getRefinedIntersection(*asAlignments.nbstack.begin(),*atAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking refined intersection of 'as' and 'at' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(inter,possLinks[level]->size());
		if (pars.verbose>1){oss << "REFINED INTERSECTION:"<<inter->print(*possLinks[level],pars)<<endl;}
		initAl=inter;
		if ((pars.wchfilt>0 || pars.wchfilts>0) && cntDirs<directions.size()-1) {
		  inter->calculateSrcChFiltLimits(srcChFiltLimits,sp,*possLinks[level],level);
		  if (pars.verbose>1){
		    int ccnt=0;
		    for (vector<pair<int,int> >::const_iterator vit=srcChFiltLimits.begin(); vit!=srcChFiltLimits.end();++vit){
		      oss<<"src chunk "<<ccnt<<":["<<vit->first<<","<<vit->second<<"]\n";
		      ++ccnt;
		    }
		  }
		}
		if ((pars.wchfilt>0 || pars.wchfiltt>0) && cntDirs<directions.size()-1) {
		  inter->calculateTrgChFiltLimits(trgChFiltLimits,sp,*possLinks[level],level);
		  if (pars.verbose>1){
		    int ccnt=0;
		    for (vector<pair<int,int> >::const_iterator vit=trgChFiltLimits.begin(); vit!=trgChFiltLimits.end();++vit){
		      oss<<"trg chunk "<<ccnt<<":["<<vit->first<<","<<vit->second<<"]\n";
		      ++ccnt;
		    }
		  }
		}
	      }else if (*dirit=="stUts"){
		symmetrisation symm;
		state* uni=symm.getUnion(*stAlignments.nbstack.begin(),*tsAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking union of 'st' and 'ts' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(uni,possLinks[level]->size());
		if (pars.verbose>1){oss << "UNION:"<<uni->print(*possLinks[level],pars)<<endl;}
		initAl=uni;
	      }else if (*dirit=="asUat"){
		symmetrisation symm;
		state* uni=symm.getUnion(*asAlignments.nbstack.begin(),*atAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking union of 'as' and 'at' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(uni,possLinks[level]->size());
		if (pars.verbose>1){oss << "UNION:"<<uni->print(*possLinks[level],pars)<<endl;}
		initAl=uni;
	      }else if (*dirit=="stRts"){
		symmetrisation symm;
		state* refined=symm.getRefinedUnion(*stAlignments.nbstack.begin(),*tsAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking refined union of 'st' and 'ts' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(refined,possLinks[level]->size());
		if (pars.verbose>1){oss << "REFINED UNION:"<<refined->print(*possLinks[level],pars)<<endl;}
		initAl=refined;
	      }else if (*dirit=="asRat"){
		symmetrisation symm;
		state* refined=symm.getRefinedUnion(*asAlignments.nbstack.begin(),*atAlignments.nbstack.begin(), *possLinks[level],posiOrderLinkCoverage, sp, pars, level,srcChFiltLimits,trgChFiltLimits);
		if (pars.verbose>0){oss<<"Taking refined union of 'as' and 'at' alignments\n";}
		if (!alignments.empty() && accu==false) {
		  copiedStates.insert(alignments.nbstack.begin(),alignments.nbstack.end());
		  alignments.clear(possLinks[level]->size());
		}
		alignments.cleverInsert(refined,possLinks[level]->size());
		if (pars.verbose>1){oss << "REFINED UNION:"<<refined->print(*possLinks[level],pars)<<endl;}
		initAl=refined;
	      }else if (*dirit=="asGDFAat"){
		symmetrisation symm;
		bool diagonal=true;
		bool final=true;
		bool bothuncovered=true; // we add links in the final step only if both words are uncovered
		oss<< symm.grow(*asAlignments.nbstack.begin(),*atAlignments.nbstack.begin(), posiOrderLinkCoverage, sp,diagonal,final,bothuncovered,level);
		if( foutput.is_open() ) foutput << oss.str()<<endl;
		else cout << oss.str()<<endl;      
	      }else{
		initAl=*alignments.nbstack.begin();
	      }
	      if (initAl != NULL){
		initAl->hist.clear();initAl->nhist=0;
	      }
	      ++cntDirs;
	    } // for (vector<string> directions
	    if (pars.verbose>0){oss<<"N-BEST LIST:"<<alignments.print(*possLinks[level],pars);}
	    //	    oss << alignments.nbest(nBest,*possLinks[level],existingLinks,level);
	    //oss << alignments.nbest(nBest,posiOrderLinkCoverage,existingLinks,level);
	    if (pars.direction != "asGDFAat" && pars.direction != "asIat-asGDFAat"){
	      oss << alignments.onebest(*possLinks[level], srcLinksVec, trgLinksVec, directions[directions.size()-1],pars );
	      if( foutput.is_open() ) foutput << oss.str();
	      else cout << oss.str();      
	      if ( fnbest.is_open() ) fnbest << alignments.nbest(*possLinks[level], srcLinksVec, trgLinksVec, directions[directions.size()-1],pars,nBest,corp.getSentPairNum()-1 );
	    }
	  } // while not end of file
	  (void) times (time_buffer);
	  double tend = time_buffer->tms_utime / clocks_per_sec;
	  cerr<<"Processing time:"<<tend-tstart<<endl;
	  cerr<<"Decoding time:"<<decoding_time<<endl;
	  cerr<<"State creation time:"<<state_creation_time<<endl;
	  cerr<<"\tState cp stcov time:"<<pars.timeControl[0]<<" ("<<(pars.timeControl[0]/state_creation_time)<<")"<<endl;
	  cerr<<"\tState cp rest time:"<<pars.timeControl[1]<<" ("<<(pars.timeControl[1]/state_creation_time)<<")"<<endl;
	  cerr<<"\tState time 1:"<<pars.timeControl[2]<<" ("<<(pars.timeControl[2]/state_creation_time)<<")"<<endl;
	  cerr<<"\tState time 2 (go through stcov):"<<pars.timeControl[3]<<" ("<<(pars.timeControl[3]/state_creation_time)<<")"<<endl;
	  cerr<<"\tState time 3 (rest):"<<pars.timeControl[4]<<" ("<<(pars.timeControl[4]/state_creation_time)<<")"<<endl;
	  cerr<<"\tState time 4 (stcov assignment):"<<pars.timeControl[5]<<" ("<<(pars.timeControl[5]/state_creation_time)<<")"<<endl;
	  cerr<<"Number of created states:"<<pars.nNewStates<<endl; 
	  if (foutput.is_open()) foutput.close();
	  
#ifdef _SERVER
	  new_sock << "done!";

	}
      }
      catch ( SocketException& ) {}
    }
  }
  catch ( SocketException& e ){
    cerr <<  "Server " << pars.PORT << "Exception was caught:" << e.description() << "\nExiting.\n";
  }
#endif

  return 0;  
}

// endmain
/* ********************************************************************************************************** */

cost ibm1max (const wlink & phlnk, int level, wdAssoQScoreTable & wdas, phraseLinkTable & existingLinks, const sentPair & sp, const param & pars ) {
  // imb1 with maximum approximation SRC->TRG
  cost stResult=0;
  int cnt=0;
  if (pars.verbose>2){cerr<<"[ibm1max] SRC->TRG\n";}
  for (vector<int>::const_iterator sposit=sp.srcWdPosis(level,phlnk.sbeg()).begin(); sposit!=sp.srcWdPosis(level,phlnk.sbeg()).end(); ++sposit){
    cost minCst=10000;
    for (vector<int>::const_iterator tposit=sp.trgWdPosis(level,phlnk.tbeg()).begin();tposit!=sp.trgWdPosis(level,phlnk.tbeg()).end();++tposit){
      cost curCst;
      if (level==1){
	curCst=wdas.combCst(sp.srcat(*sposit).getForm(), sp.trgat(*tposit).getForm(), pars, 5);
	if (pars.verbose>2){cerr<<"[ibm1max]"<<*sposit<<" "<<sp.srcat(*sposit).getForm()<<","<<*tposit<<" "<<sp.trgat(*tposit).getForm()<<"->"<<curCst<<"    ";}
      }else{
	cerr<<"ERROR: IBM1MAX FOR LEVEL>1 HAS NOT BEEN TESTED\n";exit(0);
	// look if this link is in existing links hash
	hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator exlit=existingLinks.jash_phLink.find(make_pair(phlnk,level));
	if (exlit!=existingLinks.jash_phLink.end()){
	  if (exlit->second->isSet()){curCst=exlit->second->cst;}
	  else{
	    exlit->second->setCst(ibm1max(wlink(*sposit,*tposit),level,wdas,existingLinks,sp,pars));
	  }
	}
      }
      if (curCst<minCst) minCst=curCst;
    }
    stResult+=minCst;
    ++cnt;
  }
  stResult=stResult/(cnt*1.0);
  if (pars.verbose>2){cerr<<"\n[ibm1max] stresult:"<<stResult<<endl;}
  // imb1 with maximum approximation SRC->TRG
  cost tsResult=0;
  cnt=0;
  if (pars.verbose>2){cerr<<"[ibm1max] TRG->SRC\n";}
  for (vector<int>::const_iterator tposit=sp.trgWdPosis(level,phlnk.tbeg()).begin();tposit!=sp.trgWdPosis(level,phlnk.tbeg()).end();++tposit){
    cost minCst=10000;
    for (vector<int>::const_iterator sposit=sp.srcWdPosis(level,phlnk.sbeg()).begin(); sposit!=sp.srcWdPosis(level,phlnk.sbeg()).end(); ++sposit){
      cost curCst;
      if (level==1){
	curCst=wdas.combCst(sp.srcat(*sposit).getForm(), sp.trgat(*tposit).getForm(), pars, 5);
      }else{
	cerr<<"ERROR: IBM1MAX FOR LEVEL>1 HAS NOT BEEN TESTED\n";exit(0);
	// look if this link is in existing links hash
	hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator exlit=existingLinks.jash_phLink.find(make_pair(phlnk,level));
	if (exlit!=existingLinks.jash_phLink.end()){
	  curCst=exlit->second->cst;
	}
      }
      if (curCst<minCst) minCst=curCst;
    }
    tsResult+=minCst;
    ++cnt;
  }
  tsResult=tsResult/(cnt*1.0);
  if (pars.verbose>2){cerr<<"[ibm1max] tsresult:"<<tsResult<<endl;}
  if (pars.phas=="imbest"){
    if (stResult>tsResult) return tsResult;
    else return stResult;
  }else if (pars.phas=="imworst"){
    if (stResult>tsResult) return stResult;
    else return tsResult;
  }
  else return (stResult+tsResult)/2.0;
} 
  
string decode (wlinkSequence & possLinks, wlinkCoverage & posiOrderLinkCoverage, const wordLinkSequenceVec & srcLinksVec, const wordLinkSequenceVec & trgLinksVec, phraseLinkTable & existingLinks, param & pars, nbestList & nblist, string _direction, const state & _initAl, const sentPair & sp, int level,vector<pair<int,int> > & srcChFiltLimits,vector<pair<int,int> > & trgChFiltLimits) {
  ostringstream oss("");
  bool debug=true;
  unsigned int stateId=0;
  stateList stateGarbage;

  // DECODING AT INFERIOR LEVEL and getting cost of this level's links
  string tabs;
  if (pars.verbose>0){
    for (int i=pars.nLevels-1; i>0;--i){tabs+="\t";}
  }
  if (level>0){
    for (wlinkSequence::iterator posipossl=possLinks.begin(); posipossl!=possLinks.end();++posipossl){
      hash_map<pair<wlink,int>,weightedLinkSequence*,hashflnklev,hasheqlnklev>::iterator exlit=existingLinks.jash_phLink.find(make_pair(*posipossl,level));
      nbestList lnkNbList;
      if (pars.verbose>1){
	oss<<tabs<<"decoding for "<<posipossl->printTalpDebug()<<endl;
	if (debug){cerr<<oss.str()<<endl; oss.str("");}
      }
      //srcLinks and trgLinks for inferior level
      wordLinkSequenceVec srcLinksLevVec(level), trgLinksLevVec(level);
      srcLinksLevVec.initLevel(level-1,sp.numSrcToks(level-1));
      trgLinksLevVec.initLevel(level-1,sp.numTrgToks(level-1));
      for (wlinkSequence::const_iterator seqit=exlit->second->begin();seqit!=exlit->second->end();++seqit){
	srcLinksLevVec[level-1][seqit->sbeg()].first=srcLinksVec[level-1][seqit->sbeg()].first;
	srcLinksLevVec[level-1][seqit->sbeg()].second.insert(*seqit);
	trgLinksLevVec[level-1][seqit->tbeg()].first=trgLinksVec[level-1][seqit->tbeg()].first;
	trgLinksLevVec[level-1][seqit->tbeg()].second.insert(*seqit);
      }
      
      // Initial state
      state* initAlLev=new state;
      initAlLev->setEmptyAl(exlit->second->size(),possLinks.size(),sp,level-1,pars);
      // WARNING: CHFILTLIMITS ARE FOR LEVEL=0; ADAPT THIS FOR LEVEL>0
      oss<<decode(*(exlit->second),posiOrderLinkCoverage, srcLinksLevVec,trgLinksLevVec,existingLinks,pars,lnkNbList,_direction,*initAlLev,sp,level-1,srcChFiltLimits,trgChFiltLimits);
      //oss<<"nbest list:"<<lnkNbList.display(*(exlit->second))<<endl;
      stateGarbage.push_back(initAlLev);
      weightedLinkSequence backupSeq=*(exlit->second);
      exlit->second->clear();
      (*lnkNbList.nbstack.begin())->printLinkSequence(backupSeq,*(exlit->second));
      //oss<<"decode level>0("<<posipossl->printTalpDebug()<<"):\n"<<existingLinks.display(sp)<<endl;

      wlink lnk=*posipossl;
      if (pars.phas=="possl" || pars.phas=="bestl"){
	if (pars.phas=="possl"){
	  cost cst=0;
	  for (wlinkSequence::const_iterator phit=backupSeq.begin();phit!=backupSeq.end();++phit){cst+=phit->cst;}
	  posipossl->cst=cst/backupSeq.size();
	}else if (pars.phas=="bestl"){
	  posipossl->cst=(*lnkNbList.nbstack.begin())->costs[0]/(*lnkNbList.nbstack.begin())->nstcov;
	}
	posipossl->costs.push_back(posipossl->cst);posipossl->costs.push_back(posipossl->cst);posipossl->costs.push_back(posipossl->cst);
      }
    }
  }
  if (pars.verbose>1 && level>0){
    oss<<"EXISTING LINKS:\n"<<existingLinks.display(sp)<<endl;
    if (debug){cerr<<oss.str()<<endl; oss.str("");}
  }
  
  // BEST LINK POSITION HASH (FOR FUTURE DISTORTION COST ESTIMATION)
  futureCostEstimation fcEstim;
  if (pars.f>0){
    oss<<tabs<<fcEstim.loadBestLinks(possLinks, srcLinksVec, trgLinksVec, pars, sp, _direction, level);
    if (pars.verbose>1){
      oss<<tabs<<fcEstim.printBestLinks();
    }
  }
  // DETERMINING LINKS TO BE EXPANDED
  string search,direction;
  search=pars.exp;
  direction=_direction;
  linksToExpand toExpandList;
  oss << toExpandList.buildList(search, direction, possLinks, srcLinksVec[level],trgLinksVec[level],pars);
  if (pars.verbose>1){
    oss<<tabs<<"to-expand list (level "<<level<<"):\n"<<toExpandList.display(sp,level,1);
    if (debug){cerr<<oss.str()<<endl; oss.str("");}
  }
  
  state* initAl=new state(_initAl);
  if (initAl->lcov.size()==0){
    initAl->setlcov(possLinks.size());
  }
  //initAl->lcov=_initAl.lcov;
  state* oldInitAl=new state(*initAl);
  // making sure oldInitAl is different from initAl to enter loop below
  if (initAl->lcov.at(0)){oldInitAl->lcov.at(0)=0;}
  else{oldInitAl->lcov.at(0)=1;}
  
  // DECODING
  bool sourceWordParse=(search=="word" && (direction=="as" || direction=="a" || direction=="st"));
  int dIter=0;
  cost bestCost=initAl->cst;
  while (dIter<pars.d && initAl->lcov != oldInitAl->lcov){
    if (dIter>0) fcEstim.disable();
    if (pars.verbose>0){
      oss<<tabs<<"initial Alignment("<<initAl<<"):\n"<<initAl->print(possLinks,pars)<<endl;
      if (debug){cerr<<oss.str(); oss.str("");}
    }
    // Stacks initialization
    multipleStacks multStacks;
    int nmultstacks=1;
    stateCoverage stateCov;
    if (pars.multipleStacks == "target-number"){
      if (sourceWordParse){
	nmultstacks=sp.numTrgToks(level)+1; //0,1,2...N words covered
      }else{
	nmultstacks=sp.numSrcToks(level)+1;
      }
    }else if (pars.multipleStacks == "number"){
      nmultstacks=sp.numSrcToks(level)+sp.numTrgToks(level)+1;
    }else if (pars.multipleStacks == "coverage"){
      unsigned int numb=pars.b.at(dIter)*pars.nms;
      if (numb==0){numb=sp.numSrcToks(level)*sp.numTrgToks(level);}
      unsigned int maxLinks=toExpandList.maxLinkSeqSize();
      nmultstacks=numb*(maxLinks+1); // in the worst case all expanded states will have a different (source,target) coverage
    }else if (pars.multipleStacks == "target-coverage" || pars.multipleStacks == "source-coverage"){
      unsigned int numb=pars.b.at(dIter)*pars.nms;
      if (numb==0){numb=sp.numSrcToks(level)*sp.numTrgToks(level);}
      unsigned int maxLinks=toExpandList.maxLinkSeqSize();
      nmultstacks=numb*(maxLinks+1); // in the worst case all expanded states will have a different (source,target) coverage
    }
    vector<vector<stateStack> > stacks(toExpandList.size()+1,vector<stateStack>(nmultstacks));
    vector<bool> nBestMultipleStack(nmultstacks,true);

    oldInitAl=initAl;
    if (pars.multipleStacks == "target-number"){
      if (sourceWordParse){stacks[0][initAl->ntcov].cleverInsert(initAl);}
      else {stacks[0][initAl->nscov].cleverInsert(initAl);}
    }else if (pars.multipleStacks == "number"){
      stacks.at(0).at(initAl->nscov+initAl->ntcov).cleverInsert(initAl);
    }else if (pars.multipleStacks == "coverage"){
      unsigned long long int wcovid=stateCov.getWordCoverageID(initAl->scov,initAl->tcov); // at this point wcovid=0
      stacks.at(0).at(wcovid).cleverInsert(initAl);
    }else if (pars.multipleStacks == "target-coverage"){
      unsigned long long int wcovid;
      if (sourceWordParse){wcovid=stateCov.getMonolWordCoverageID(initAl->tcov);} // at this point wcovid=0
      else{wcovid=stateCov.getMonolWordCoverageID(initAl->scov);} // at this point wcovid=0
      stacks.at(0).at(wcovid).cleverInsert(initAl);
    }else if (pars.multipleStacks == "source-coverage"){
      unsigned long long int wcovid;
      if (sourceWordParse){wcovid=stateCov.getMonolWordCoverageID(initAl->scov);} // at this point wcovid=0
	else{wcovid=stateCov.getMonolWordCoverageID(initAl->tcov);} // at this point wcovid=0
      stacks.at(0).at(wcovid).cleverInsert(initAl);
    }else{
      stacks[0][0].cleverInsert(initAl);
    }
    int nstack=0;
    for (int expandIter=0; expandIter<2; ++expandIter){
      int onlyBestAsso(-1);
      if (dIter<pars.d-1 && pars.doba>0) onlyBestAsso=pars.doba-1;
      else if (pars.dOnlyBestAsso>0) onlyBestAsso=pars.dOnlyBestAsso-1;
      else if (expandIter>0 && pars.avgeOnlyBestAsso) onlyBestAsso=pars.avgeOnlyBestAsso-1; //only puts in stack sequences with all links having at least trgRankForSrc=0 or srcRankForSrc=0.
      //oss<<"onlyBestAsso:"<<onlyBestAsso<< " cond1:"<<(dIter<pars.d-1 && pars.doba>0)<<" dOnly:"<<pars.dOnlyBestAsso<<" cond3:"<<(expandIter>0 && pars.avgeOnlyBestAsso)<<endl;
      int hr=pars.hr;
      //if (expandIter==0){hr=0;}
      //else hr=pars.hr;
      for (linksToExpand::const_iterator lexp=toExpandList.begin(); lexp!=toExpandList.end();++lexp){ //in SWP mode, for each word in src
	if (lexp->first == expandIter){
	  //if (pars.verbose>0){oss<<tabs<<"nstacks:"<<stacks.size()<<endl;}
	  if (pars.verbose>1){
	    if (debug){cerr<<oss.str(); oss.str("");}
	    for (vector<stateStack>::size_type nsubstack=0; nsubstack< stacks[nstack].size(); ++nsubstack){
	      if (stacks[nstack][nsubstack].size()>0){
		oss<<tabs<<"\nSTACK "<<nstack<<": substack"<<nsubstack;
		if (pars.multipleStacks == "coverage"){
		  oss<<" "<<stateCov.printWordCoverage(nsubstack);
		  if (nBestMultipleStack.at(nsubstack)){oss<<" (to be expanded)";}
		  else{oss<<" (pruned)";}
		  oss<<endl;
		}else if (pars.multipleStacks == "source-coverage" || pars.multipleStacks == "target-coverage"){
		  oss<<" "<<stateCov.printMonolWordCoverage(nsubstack);
		  if (nBestMultipleStack.at(nsubstack)){oss<<" (to be expanded)";}
		  else{oss<<" (pruned)";}
		  oss<<endl;
		}
		else{oss<<endl;}
		if (debug){cerr<<oss.str(); oss.str("");}
		oss<<"\n"<<stacks[nstack][nsubstack].print(possLinks,pars);
	      }
	    }
	    if (debug){cerr<<oss.str(); oss.str("");}
	  }
	  //copy b-best list of previous stack to current stack
	  hash_map<list<vector<int> >, state*, hashflistvecints, hasheqlistvecints> jash_recombine;
	  hash_set<vector<int>,hashfvecint,hasheqvecint> createdStates;
	  cost prevStackBestCost=bestCost;

	  stateCov.clear();
	  int nsubstack=0;
	  //	  while ( nsubstack< stacks[nstack].size() && (pars.nms==0 || pars.nms>0 && nsubstack < pars.nms) ){
	  while ( nsubstack < stacks.at(nstack).size() ){
	    if (pars.nms>0 && ! nBestMultipleStack.at(nsubstack)){
	      ++nsubstack;
	      continue;
	    }
	    int i=0;
	    stateStack::const_iterator al=stacks[nstack][nsubstack].begin();
	    while (i<pars.b[dIter] && al != stacks[nstack][nsubstack].end() && (pars.t==0 || pars.t>0 && (*al)->cst<prevStackBestCost+pars.t) ){ // for each non-pruned state in previous stack 
	      if (pars.verbose>1){
		oss<<tabs<<"\nExpanding:\n"<<(*al)->print(possLinks,pars)<<endl<<endl;
		if (debug){cerr<<oss.str(); oss.str("");}
	      }
	      int iwei=0;
	      set<weightedLinkSequence,ltweightedlinkseq>::const_iterator weiLnkSeqit=lexp->second->begin();
	      cost weiBestCost=weiLnkSeqit->cst;

	      // If initial alignment not empty, see which links of next stack are in this state, and create a state without them
	      state* father;
	      vector<linkIndex> nextStackLinkIds;
	      if ( ! initAl->empty() ){
		vector<pair<wlink,bool> > rmvec; 
		if (!(*al)->empty()){
		  if ( (*al)->lcov.size()==0 ){(*al)->setlcov(possLinks.size());}
		  while ((pars.c==0 || iwei<pars.c) && (pars.ct==0 || weiLnkSeqit->cst < weiBestCost+pars.ct) && weiLnkSeqit != lexp->second->end()){ //for each possible linksequence in this set
		    for (weightedLinkSequence::const_iterator possl=weiLnkSeqit->begin(); possl!=weiLnkSeqit->end();++possl){ //for each link in a link combination
		      if ( (*al)->lcov.at(possl->id) ){
			rmvec.push_back(make_pair(*possl,false));
			nextStackLinkIds.push_back(possl->id);
		      }
		    }
		    ++iwei;++weiLnkSeqit;
		  }
		}
		if (rmvec.size()>0){
		  father= new state(stateId,*al,rmvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcEstim,direction);
		  if (pars.backTrack){createdStates.insert(father->lcov);}
		  if (pars.verbose>1){
		    oss<<tabs<<"Father:\n"<<father->print(possLinks,pars)<<endl;
		    if (debug){cerr<<oss.str(); oss.str("");}
		  }
		  // copy al to next stack
		  if (hr>0){ 
		    vector<int> vec;vec.push_back(-1);
		    (*al)->hist.push_back(vec);++((*al)->nhist);
		    if ((*al)->nhist>hr){(*al)->hist.pop_front();--((*al)->nhist);}
		    hash_map<list<vector<int> >, state*, hashflistvecints, hasheqlistvecints>::iterator combit=jash_recombine.find((*al)->hist);
		    if (combit==jash_recombine.end()){
		      jash_recombine.insert(make_pair((*al)->hist,*al));
		    }else{
		      if (combit->second->fcst > (*al)->fcst){
			jash_recombine[combit->first]=*al;
		      }
		    }
		  }else{
		    unsigned long long int subStackIndex=multStacks.subStackIndex( pars, sourceWordParse, *al, stateCov );
		    if (stacks.at(nstack+1).size()<=subStackIndex){cerr<<"ERROR: number of substacks underestimated at initialization\n";exit(EXIT_FAILURE);}
		    stacks[nstack+1][subStackIndex].cleverInsert(*al);
		  }
		}else{
		  father=*al;
		}
	      }else{
		father=*al;
	      }
	      
	      iwei=0;
	      weiLnkSeqit=lexp->second->begin();
	      while ((pars.c==0 || iwei<pars.c) && (pars.ct==0 || weiLnkSeqit->cst < weiBestCost+pars.ct) && weiLnkSeqit != lexp->second->end()){ //for each possible linksequence in this set, consider to expand state
		if (onlyBestAsso>-1){
		  bool currentOnlyBestAsso=true;
		  for (weightedLinkSequence::const_iterator possl=weiLnkSeqit->begin(); possl!=weiLnkSeqit->end();++possl){
		    if (possl->trgRankForSrc > onlyBestAsso && possl->srcRankForTrg > onlyBestAsso) currentOnlyBestAsso=false;
		  }
		  if (currentOnlyBestAsso==false) {++iwei;++weiLnkSeqit;continue;}
		}
		vector<pair<wlink,bool> > lnkvec;
		// when adding a link, check that this link is compatible with the links present in the state to be expanded
		set<linkIndex> conflictiveLinkIds;
		bool compatible=true;
		for (weightedLinkSequence::const_iterator possl=weiLnkSeqit->begin(); possl!=weiLnkSeqit->end();++possl){
		  if (!father->compatibleWithLink(*possl,posiOrderLinkCoverage,srcLinksVec.at(level),trgLinksVec.at(level),pars,conflictiveLinkIds)){
		    if (! pars.backTrack){
		      if (pars.verbose>1){
			oss<<tabs<<"-->link "<<possl->printTalpDebug()<<" not compatible with state:"<<father->id<<endl;
			if (debug){cerr<<oss.str(); oss.str("");}
		      }
		      compatible=false;break;
		    }
		  }
		}
		if (pars.backTrack){
		  for (set<linkIndex>::const_iterator id=conflictiveLinkIds.begin();id!=conflictiveLinkIds.end();++id){
		    lnkvec.push_back(make_pair(possLinks.at(*id),false));
		  }
		}else{
		  if (!compatible){
		    ++iwei;++weiLnkSeqit;continue;
		  }
		}
		// EXPAND STATE
		vector<linkIndex> linkIds;
		for (weightedLinkSequence::const_iterator possl=weiLnkSeqit->begin(); possl!=weiLnkSeqit->end();++possl){ //for each link in a link combination
		  lnkvec.push_back(make_pair(*possl,true));
		  linkIds.push_back(possl->id);
		}
		if (pars.verbose>1){
		  oss<<tabs<<"> new links:";
		  for (vector<pair<wlink,bool> >::const_iterator l=lnkvec.begin();l!=lnkvec.end();++l){oss<<" "<<l->first.printTalpDebug()<<" ("<<l->second<<")";}
		  oss<<endl;
		  if (debug){cerr<<oss.str(); oss.str("");}
		}

		(void) times (time_buffer);
		double t1 = time_buffer->tms_utime / clocks_per_sec;

		if (nextStackLinkIds.size()>0 && nextStackLinkIds==linkIds){
		  if (pars.verbose>1){
		    oss<<tabs<<"We already have this link: continue"<<endl;
		    if (debug){cerr<<oss.str(); oss.str("");}		  
		  }
		  ++iwei;++weiLnkSeqit;continue;
		} // we already have this state: don't create it again
		state* currState = new state(stateId,father,lnkvec,possLinks,posiOrderLinkCoverage,pars,sp,oss,level,srcChFiltLimits,trgChFiltLimits,fcEstim,direction);
		if (pars.backTrack){
		  pair<hash_set<vector<int>,hashfvecint,hasheqvecint>::iterator,bool> retPair=createdStates.insert(currState->lcov);
		  if (!retPair.second){
		    if (pars.verbose>1){oss<<tabs<<"Deleted duplicated state:"<<currState->print(possLinks,pars)<<endl;}
		    delete currState;--stateId;
		    ++iwei;++weiLnkSeqit;continue;
		  }
		}
		(void) times (time_buffer);
		double t2 = time_buffer->tms_utime / clocks_per_sec;
		state_creation_time+=t2-t1;
		if (pars.verbose>1){
		  oss<<tabs<<currState->print(possLinks,pars)<<endl;
		}
		// Threshold pruning
		if (pars.t>0 && currState->cst<bestCost) bestCost=currState->cst;
		if (pars.t>0 && currState->cst > prevStackBestCost+pars.t){
		  delete currState; //this state can't be father of anybody so we can delete it
		}else{
		  if (hr==0){
		    unsigned long long int subStackIndex=multStacks.subStackIndex( pars, sourceWordParse, currState, stateCov );
		    if (stacks.at(nstack+1).size()<=subStackIndex){cerr<<"ERROR: number of substacks underestimated at initialization\n";exit(EXIT_FAILURE);}
		    stacks[nstack+1][subStackIndex].cleverInsert(currState);
		  }else{
		    // insert currState in recombination hash
		    hash_map<list<vector<int> >, state*, hashflistvecints, hasheqlistvecints>::iterator combit=jash_recombine.find(currState->hist);
		    if (combit==jash_recombine.end()){
		      jash_recombine.insert(make_pair(currState->hist,currState));
		    }else{
		      if (combit->second->fcst > currState->fcst){
			stateGarbage.push_back(combit->second);
			jash_recombine[combit->first]=currState;
		      }else{
			delete currState;
		      }
		    }
		  }
		}
		++iwei;++weiLnkSeqit;
	      } //for each weighted link sequence in current set (lexp)
	      // copy father to next stack
	      if (hr>0){ 
		if (initAl->empty()){
		  vector<int> vec;vec.push_back(-1);
		  father->hist.push_back(vec);++(father->nhist);
		  if (father->nhist>hr){father->hist.pop_front();--(father->nhist);}
		}
		// insert currState's father in recombination hash
		hash_map<list<vector<int> >, state*, hashflistvecints, hasheqlistvecints>::iterator combit=jash_recombine.find(father->hist);
		if (combit==jash_recombine.end()){
		  jash_recombine.insert(make_pair(father->hist,*al));
		}else{
		  if (combit->second->fcst > father->fcst){
		    jash_recombine[combit->first]=*al;
		  }
		}
	      }else{
		unsigned long long int subStackIndex=multStacks.subStackIndex( pars, sourceWordParse, father, stateCov );
		if (stacks.at(nstack+1).size()<=subStackIndex){cerr<<"ERROR: number of substacks underestimated at initialization\n";exit(EXIT_FAILURE);}
		stacks[nstack+1][subStackIndex].cleverInsert(father);
	      }
	      ++al;++i;
	    } //while al in previous stack
	    if (hr>0){
	      for (hash_map<list<vector<int> >, state*, hashflistvecints, hasheqlistvecints>::iterator combit=jash_recombine.begin();combit!=jash_recombine.end();++combit){
		unsigned long long int subStackIndex=multStacks.subStackIndex( pars, sourceWordParse, combit->second, stateCov );
		if (stacks.at(nstack+1).size()<=subStackIndex){cerr<<"ERROR: number of substacks underestimated at initialization\n";exit(EXIT_FAILURE);}
		stacks.at(nstack+1).at(subStackIndex).cleverInsert(combit->second);
	      }
	    }
	    ++nsubstack;
	  } // while ( nsubstack< stacks[nstack].size() && (pars.nms==0 || pars.nms>0 && nsubstack < pars.nms) )
	  ++nstack;
	  // if nms>0, we sort the stack so that the nms first substacks are the best ones.
	  if (pars.nms>0){
	    if (pars.multipleStacks == "coverage"){
	      stacks.at(nstack).erase(stacks.at(nstack).begin()+stateCov.size(),stacks.at(nstack).end());
	    }else if (pars.multipleStacks == "source-coverage" || pars.multipleStacks == "target-coverage"){
	      stacks.at(nstack).erase(stacks.at(nstack).begin()+stateCov.monolSize(),stacks.at(nstack).end());
	    }
	    vector<pair<unsigned int, pair<cost,unsigned int> > > vectosort;
	    unsigned int ssindex=0;
	    for (vector<stateStack>::const_iterator it=stacks.at(nstack).begin();it!=stacks.at(nstack).end();++it){
	      if (! it->empty()){
		vectosort.push_back(make_pair(ssindex,make_pair((*it->begin())->cst,(*it->begin())->id)));
	      }
	      ++ssindex;
	    }
	    unsigned int nToSort=pars.nms-1;
	    // put the nms best substacks in the nms first positions (but not sorted)
	    nth_element(vectosort.begin(),vectosort.begin()+nToSort,vectosort.end(),ltpairuintpaircostuint());
	    if (pars.verbose>1){
	      oss<<"Vectosort after (size"<<vectosort.size()<<"):"<<endl;
	      for (vector<pair<unsigned int, pair<cost,unsigned int> > >::const_iterator it=vectosort.begin();it!=vectosort.end();++it){
		  oss<<it->first<<" "<<it->second.first<<" "<<it->second.second<<endl;
	      }
	      if (debug){cerr<<oss.str(); oss.str("");}
            }
	    // update boolean vector with index of unpruned substacks
            for (unsigned int i=0; i<stacks.at(nstack).size(); ++i){nBestMultipleStack.at(i)=false;}
	    for (unsigned int i=0; i<pars.nms && i<stacks.at(nstack).size();++i){
	      unsigned int ind=vectosort.at(i).first;
	      nBestMultipleStack.at(ind)=true;
	    }
	    if (pars.verbose>1){
	      oss<<"nBestMultipleStack:";
	      for (vector<bool>::const_iterator it=nBestMultipleStack.begin(); it!=nBestMultipleStack.end();++it){
		oss<<*it;
	      }
	      oss<<endl;
	      if (debug){cerr<<oss.str(); oss.str("");}
	    }
	  }
	} //if expandIter
      } //for linksToExpand (ie for each stack)
    } // for expandIter
    if (pars.verbose>1){
      oss<<endl;
      for (vector<stateStack>::size_type nsubstack=0; nsubstack< stacks[nstack].size(); ++nsubstack){
	if (stacks[nstack][nsubstack].size()>0){
	  oss<<tabs<<"STACK "<<nstack<<": substack"<<nsubstack;
	  if (pars.multipleStacks == "coverage"){oss<<" "<<stateCov.printWordCoverage(nsubstack);}
	  oss<<"\n"<<stacks[nstack][nsubstack].print(possLinks,pars)<<endl;
	  if (debug){cerr<<oss.str(); oss.str("");}
	}
      }
    }
    // merge all stacks in the same stack
    stateStack finalStack;
    for (vector<stateStack>::size_type nsubstack=0; nsubstack< stacks[nstack].size(); ++nsubstack){
      for (stateStack::const_iterator al=stacks[nstack][nsubstack].begin(); al!=stacks[nstack][nsubstack].end();++al){
	finalStack.cleverInsert(*al);
      }
    }
    if (finalStack.size()>0){
      stateStack::const_iterator al=finalStack.begin();
      initAl=*al;
      initAl->hist.clear();initAl->nhist=0;
      if (initAl->lcov.size()==0){
	initAl->setlcov(possLinks.size());
      }
      int i=0;
      // causes a segmentation fault in client-server for ex. if b=20 b1=5 (why?): while (i<pars.b[dIter] && al != stacks[nstack].end()){
      while (i<1 && al != finalStack.end()){
	nblist.cleverInsert(*al,possLinks.size());
	++i;++al;
      }
    }
    if (pars.verbose>1 && level == pars.nLevels){
      oss<<tabs<<direction<<" PARSE N-BEST LIST:"<<nblist.print(possLinks,pars);
      if (debug){cerr<<oss.str(); oss.str("");}
    }
    ++dIter;
  } //while diter
  return oss.str();
}
//ver pb de las nbest (añadimos estados idénticos)
