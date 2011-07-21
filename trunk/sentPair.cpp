#include "sentPair.h"

/* sentPair class
********************/
int sentPair::loadWordSentence (confusionNet<iword> & tokens, uniqAlUnitColumnCollection & units, string line, vocTable & voc){ //sentence input (inputType=0)
  int count=0;
  string buf;
  wordIndex bufInd;
  istringstream iss(line);
  while (iss >> buf){
    bufInd=voc.insert(buf);
    iword w;
    w.addPos(count);
    w.addForm(bufInd);
    w.addStform(buf);
    iwordCnColumn col;
    tokens.push_back(col);
    tokens.push_back_at_column(w,1.0,count);
    uniqAlUnitColumn uniqAlUnitCol(count);
    units.push_back(uniqAlUnitCol);
    units.push_back_at_column(alUnit(bufInd),1.0,count);
    ++count;
  }
  return count;
}

/*
name s
numaligns N
posterior P
align a w1 p1 w2 p2 ...
reference a w
hyps a w h1 h2 ...
info a w start dur ascore gscore phones phonedurs
*/
int sentPair::loadWordSentence (confusionNet<iword> & tokens, uniqAlUnitColumnCollection & units, const vector<string> & lines, vocTable & voc){
  int nCol=0;
  for (vector<string>::const_iterator it=lines.begin();it!=lines.end();++it){
    istringstream iss(*it);
    string buf;
    iss>>buf;iss>>buf; //skip "align position"
    string tbuf;
    wordIndex bufInd;
    float pbuf;
    iwordCnColumn col;
    tokens.push_back(col);
    uniqAlUnitColumn uniqAlUnitCol(nCol);
    units.push_back(uniqAlUnitCol);
    while (iss >> tbuf && iss >> pbuf){
      bufInd=voc.insert(tbuf);
      iword w;
      w.addPos(nCol);
      w.addForm(bufInd);
      w.addStform(tbuf);
      //cout<<"ncol "<<nCol<<" word:"<<w.print()<<endl;
      tokens.push_back_at_column(w,pbuf,nCol);
      //cout<<"tokens.print:"<<tokens.print()<<endl;
      units.push_back_at_column(alUnit(bufInd),pbuf,nCol);
      //cout<<"units print ncol:"<<units.print(voc);
    }
    ++nCol;
  }  
  return nCol;
}

int sentPair::loadPhraseSentence (sentence & tokens, string line, phraseToWordMap & phToWdPosi, int level){
  string buf;
  istringstream iss(line);
  int count=0;
  while (iss >> buf){
    string::size_type s=buf.find("_",0);
    string curStForm=buf.substr(0,s);++s;
    vector<int> wdPosis;
    while (s<buf.size()){
      string::size_type begin=s;
      while (s <buf.size() && buf.substr(s,1)!="_"){++s;}
      string strPosi=buf.substr(begin,s-begin);
      int posi=atoi(strPosi.data())-1;
      tokens[posi].addPos(count);
      tokens[posi].addStform(curStForm); //??? a un sens que stform soit un vecteur ? ou seul le niveau 0 a une "string form" ?
      wdPosis.push_back(posi);
      if (s<buf.size()){++s;}
    }
    ++count;
    phToWdPosi[level].push_back(wdPosis);
  }
  return count;
}

int sentPair::loadPofs (confusionNet<iword> & tokens, string line, vocTable & voc){
  string buf;
  istringstream iss(line);
  int count=0;
  while (iss >> buf){
    wordIndex bufInd=voc.insert(buf);
    tokens.at(count).at(0).first.setPofs(bufInd);
    ++count;
  }
  return count;
}

void sentPair::loadUnlinkedModel(confusionNet<iword> & tokens, const unlinkedModelTable & umtab){
  for (confusionNet<iword>::iterator s=tokens.begin();s!=tokens.end();++s){
    cost cst;
    if (umtab.cst(s->at(0).first.getForm(0),cst)) s->at(0).first.setNullCst(cst);
    else s->at(0).first.setNullCst(4.0);
  }
}

void sentPair::loadFert(confusionNet<iword> & tokens, const fertTable & ftab, string mode){
  for (confusionNet<iword>::iterator c=tokens.begin();c!=tokens.end();++c){
    //cout<<"column:"<<c->print()<<endl;
    for (iwordCnColumn::iterator p=c->begin();p!=c->end();++p){
      vector<cost> vec;
      if (ftab.cstVec(p->first.getForm(0),vec)){
//       for (vector<cost>::const_iterator v=vec.begin();v!=vec.end();++v){
// 	    cout<<*v<<" ";
//       }
//       cout<<endl;
      }else{
	// this word is not in the fertility table thus it was not seen in training. Thus it is not either in the phrase-association or mss tables
	// so it cannot be linked: we give it "0 1 2 3 4" fertility cost vector
	for (int i=0; i<=4; ++i){vec.push_back(i);} 
      }
      p->first.setFertCostsVec(vec);
    }
    c->merge(mode);
  }
}

void sentPair::loadClFert(confusionNet<iword> & tokens, const classFertTable & ftab){
  for (int i=0;i<tokens.size();++i){
    vector<cost> vec;
    if (i==0){
      ftab.cstVec(1,tokens.at(i).at(0).first.getForm(0),vec);
    }else{
      ftab.cstVec(tokens.at(i-1).at(0).first.getPofs(),tokens.at(i).at(0).first.getForm(0),vec);
    }
    tokens.at(i).at(0).first.setFertCostsVec(vec);
  }
}

string sentPair::display () const {
  ostringstream oss("");
  int count=0;
  for (confusionNet<iword>::const_iterator w=(this->s).begin();w!=(this->s).end();++w){
    oss<<count<<" [";
    ++count;
    for (iwordCnColumn::const_iterator c=w->begin();c!=w->end();++c){
      if (c!=w->begin()){oss<<" ||| ";}
      for (int lev=0; lev<nlevels;++lev){
	if (lev==0) {
	  oss<<c->first.getPos()<<":"<<c->first.getStform(lev);
	  if (thereAreTags && lev==0) oss<<"-"<<c->first.getPofs();
	  if (c->first.getNullCst()>0.000001 && c->first.getNullCst()<10.0) oss<<"-NULL"<<c->first.getNullCst();
	}else {oss<<c->first.getPos(lev)<<":"<<c->first.getStform (lev);}
	oss<<"{"; for (int i=0; i<w->getFertSize();++i){oss<<w->getFertCst(i);if (i<w->getFertSize()-1) oss<<" ";}; oss<<"}";
	if (lev<nlevels-1){oss<<"|";}
      }
    }
    oss<<"] ";
  }
  oss<<endl<<endl;
  count=0;
  for (confusionNet<iword>::const_iterator w=(this->t).begin();w!=(this->t).end();++w){
    oss<<count<<" [";
    ++count;
    for (iwordCnColumn::const_iterator c=w->begin();c!=w->end();++c){
      if (c!=w->begin()){oss<<" ||| ";}
      for (int lev=0; lev<nlevels;++lev){
	if (lev==0){
	  oss<<c->first.getPos()<<":"<<c->first.getStform(lev);
	  if (thereAreTags && lev==0) oss<<"-"<<c->first.getPofs();
	  if (c->first.getNullCst()>0.000001 && c->first.getNullCst()<10.0) oss<<"-NULL"<<c->first.getNullCst();
	}else{oss<<c->first.getPos(lev)<<":"<<c->first.getStform(lev);}
	oss<<"{"; for (int i=0; i<c->first.getFertSize();++i){oss<<c->first.getFertCst(i);if (i<c->first.getFertSize()-1) oss<<" ";}; oss<<"}";
	if (lev<nlevels-1){oss<<"|";}
      }
    }
    oss<<"] ";
  }
  oss<<endl;
  for (int lev=1; lev<nlevels;++lev){
    for (int phPosi=0;phPosi<sPhToWdPosi[lev].size();++phPosi){
      oss<<phPosi<<"->";
      for (vector<int>::const_iterator v=sPhToWdPosi[lev][phPosi].begin();v!=sPhToWdPosi[lev][phPosi].end();++v){
	oss<<*v;
	if (v!=sPhToWdPosi[lev][phPosi].end()){oss<<",";}
      }
      oss<<" ";
    }
    oss<<endl;
    for (int phPosi=0;phPosi<tPhToWdPosi[lev].size();++phPosi){
      oss<<phPosi<<"->";
      for (vector<int>::const_iterator v=tPhToWdPosi[lev][phPosi].begin();v!=tPhToWdPosi[lev][phPosi].end();++v){
	oss<<*v;
	if (v!=tPhToWdPosi[lev][phPosi].end()){oss<<",";}
      }
      oss<<" ";
    }
    oss<<endl;
  }
  return oss.str();
}

string sentPair::printAlUnits(const vocTable & svoc, const vocTable & tvoc) const{
    ostringstream oss("");
    oss<<"source AlUnits:\n"<<sAlUnits.print(svoc)<<"\ntarget AlUnits:\n"<<tAlUnits.print(tvoc)<<endl;
    return oss.str();
}
void sentPair::addSrcFoundPhrases(const uniqAlUnitColumnCollection & coll){
  for (uniqAlUnitColumnCollection::const_iterator it=coll.begin();it != coll.end();++it){
    sAlUnits.push_back(*it);
  }
}
void sentPair::addTrgFoundPhrases(const uniqAlUnitColumnCollection & coll){
  for (uniqAlUnitColumnCollection::const_iterator it=coll.begin();it != coll.end();++it){
    tAlUnits.push_back(*it);
  }
}

/* bilCorpus class
********************/

bilCorpus::bilCorpus (const vector<string> & fs, const vector<string> & ft, string ftgs, string ftgt, string range, int sInputType,int tInputType): tgsFile(ftgs.c_str()),tgtFile(ftgt.c_str()) {
  // open input source and target (word forms) files:
  for (vector<string>::const_iterator fname=fs.begin(); fname!=fs.end();++fname){
    ifstream* fsFileLevel=new ifstream((*fname).c_str());
    if (! fsFileLevel->good()){
      cerr << "ERROR while opening file:" << *fname << endl;
      exit(EXIT_FAILURE);
    }
    fsFile.push_back( fsFileLevel );
  }
  for (vector<string>::const_iterator fname=ft.begin(); fname!=ft.end();++fname){
    ifstream* ftFileLevel=new ifstream((*fname).c_str());
    if (! ftFileLevel->good()){
      cerr << "ERROR while opening file:" << *fname << endl;
      exit(EXIT_FAILURE);
    }
    ftFile.push_back( ftFileLevel );
  }
  if (ftgs.compare("-")!=0 && ftgt.compare("-")!=0){
    // open input source and target Part-of-Speech files:
    if (! tgsFile){
      cerr << "ERROR while opening file:" << ftgs << endl;
      exit(EXIT_FAILURE);
    }
    if (! tgtFile){
      cerr << "ERROR while opening file:" << ftgt << endl;
      exit(EXIT_FAILURE);
    }
  }
  // RETRIEVE RANGE
  string rangeBegin,rangeEnd;
  string::size_type loc = range.find( "-", 0 );
  rangeBegin=range.substr(0,loc);
  rangeEnd=range.substr(loc+1,range.size()-loc);
  //cout<<"range:"<<rangeBegin<<"-"<<rangeEnd<<endl;
  if (rangeBegin.size()>0){firstSentPairNum=atoi(rangeBegin.c_str());}
  else{firstSentPairNum=1;}
  if (rangeEnd.size()>0){
    lastSentPairNum=atoi(rangeEnd.c_str());
    untilEnd=false;
  }else{
    untilEnd=true;
  }
  //cout<<"range int:"<<firstSentPairNum<<"-"<<lastSentPairNum<<endl;
  
  // GO TO FIRST SENTENCE PAIR
  sentPairNum=0;
  while (sentPairNum<firstSentPairNum-1){
    ++sentPairNum;
    string line;
    for (vector<ifstream*>::size_type nf=0; nf<fsFile.size();++nf){
      if (! fsFile[nf]->good()){
	cerr << "ERROR: file" << fs[nf] << " not good " << endl; exit(EXIT_FAILURE);
      }else{
	if (sInputType==0){
	  if (!getline(*fsFile[nf],line)){
	    cerr << "ERROR while going to first line of range in file:" << fs[nf] << endl;
	    exit(EXIT_FAILURE);
	  }
	}else{
	  string buf;
	  int stop=-1;
	  int filepos;
	  while (getline(*fsFile[nf],line) && stop <=0){
	    istringstream iss(line);
	    iss >> buf;
	    if (buf == "name"){++stop;}
	    else {filepos=fsFile[nf]->tellg();}
	  }
	  fsFile[nf]->seekg(filepos);
	}
      }
    }
    for (vector<ifstream*>::size_type nf=0; nf<ftFile.size();++nf){
      if (! ftFile[nf]->good()){
	cerr << "ERROR: file" << ft[nf] << " not good " << endl; exit(EXIT_FAILURE);
      }else{
	if (tInputType==0){
	  if (!getline(*ftFile[nf],line)){
	    cerr << "ERROR while going to first line of range in file:" << ft[nf] << endl;
	    exit(EXIT_FAILURE);
	  }
	}else{
	  string buf;
	  int stop=-1;
	  int filepos;
	  while (getline(*ftFile[nf],line) && stop <=0){
	    istringstream iss(line);
	    iss >> buf;
	    if (buf == "name"){++stop;}
	    else {filepos=ftFile[nf]->tellg();}
	  }
	  ftFile[nf]->seekg(filepos);
	}
      }
    }
    if (tgsFile && tgtFile){
      if (!getline(tgsFile,line)){
	cerr << "ERROR while going to first line of range in file:" << ftgs << endl;
	exit(EXIT_FAILURE);
      }
      if (!getline(tgtFile,line)){
	cerr << "ERROR while going to first line of range in file:" << ftgt << endl;
	exit(EXIT_FAILURE);
      }
    }
  }
}

bool bilCorpus::getNextSentPair (sentPair & sp, vocTable & svoc, vocTable & tvoc, int sInputType,int tInputType, vocTable & sTagVoc, vocTable & tTagVoc){
  sp.clear();
  ++sentPairNum;
  if (!untilEnd && sentPairNum>lastSentPairNum){
    return false;
  }
  string line;
  sp.nlevels=fsFile.size();
  sp.initPhToWdMaps();
  for (vector<ifstream*>::size_type nf=0; nf<fsFile.size();++nf){
    if (! fsFile[nf]->good()){
      return false;
    }else{
      // read source sentence and split it
      if (sInputType==0){
	if (nf==0 && getline(*fsFile[nf],line)) sp.loadSrcWords(line,svoc);
	else if (getline(*fsFile[nf],line)) sp.loadSrcPhrases(line,nf);
	else  return false;
      }else if (sInputType==1){
	if (nf==0){
	  vector<string> lines;
	  string buf;
	  int stop=-1;
	  int filepos;
	  while (getline(*fsFile[nf],line) && stop <=0){
	    buf=""; //if buf is not reset and "line" is empty, buf keeps its previous value
	    istringstream iss(line);
	    iss >> buf;
	    if (buf == "align"){lines.push_back(line);}
	    if (buf == "name"){++stop;}
	    else {filepos=fsFile[nf]->tellg();}
	  }
	  fsFile[nf]->seekg(filepos);
	  //	  for (vector<string>::const_iterator it=lines.begin();it!=lines.end();++it){cout<<"l:"<<*it<<endl;}
	  sp.loadSrcWords(lines,svoc);
	  //	  cout<<sp.src().print()<<endl;
	  //cout<<sp.srcAlUnits().print()<<endl;
	}else {sp.loadSrcPhrases(line,nf);}
      }
    }
  }
  for (vector<ifstream*>::size_type nf=0; nf<ftFile.size();++nf){
    if (! ftFile[nf]->good()){
      return false;
    }else{
      // read target sentence and split it
      if (tInputType==0){
	if (nf==0 && getline(*ftFile[nf],line)) sp.loadTrgWords(line,tvoc);
	else if (getline(*ftFile[nf],line)) sp.loadTrgPhrases(line,nf);
	else  return false;
      }else if (tInputType==1){
	if (nf==0){
	  vector<string> lines;
	  string buf;
	  int stop=-1;
	  int filepos;
	  while (getline(*ftFile[nf],line) && stop <=0){
	    buf=""; //if buf is not reset and "line" is empty, buf keeps its previous value
	    istringstream iss(line);
	    iss >> buf;
	    if (buf == "align"){lines.push_back(line);}
	    if (buf == "name"){++stop;}
	    else {filepos=ftFile[nf]->tellg();}
	  }
	  ftFile[nf]->seekg(filepos);
	  //	  for (vector<string>::const_iterator it=lines.begin();it!=lines.end();++it){cout<<"l:"<<*it<<endl;}
	  sp.loadTrgWords(lines,tvoc);
	}else {sp.loadTrgPhrases(line,nf);}
      }
    }
  }
  if (tgsFile && tgtFile){
    // read tag source sentence and split it
    if (getline(tgsFile,line)){
      sp.loadSrcTags(line,sTagVoc);
    }else {return false;}
    // read tag target sentence and split it
    if (getline(tgtFile,line)){
      sp.loadTrgTags(line,tTagVoc);
    }else{return false;}
  }
  return true;
}

