#include "sentPair.h"

/* sentPair class
********************/
int sentPair::loadWordSentence (confusionNet<iword> & tokens, string line, vocTable & voc){ //sentence input (inputType=0)
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
int sentPair::loadWordSentence (confusionNet<iword> & tokens, const vector<string> & lines, vocTable & voc){
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
    while (iss >> tbuf && iss >> pbuf){
      bufInd=voc.insert(tbuf);
      iword w;
      w.addPos(nCol);
      w.addForm(bufInd);
      w.addStform(tbuf);
      //      cout<<"word:"<<w.print()<<endl;
      tokens.push_back_at_column(w,pbuf,nCol);
      //      cout<<"tokens.print:"<<tokens.print()<<endl;
    }
    ++nCol;
  }  
  return nCol;
}

string sentPair::display () const {
  ostringstream oss("");
  int count=0;
  for (confusionNet<iword>::const_iterator w=(this->s).begin();w!=(this->s).end();++w){
    oss<<count<<" [";
    ++count;
    for (iwordCnColumn::const_iterator c=w->begin();c!=w->end();++c){
      if (c!=w->begin()){oss<<" ||| ";}
      oss<<c->first.getPos()<<":"<<c->first.getForm();
      oss<<"("<<c->first.getStform()<<")";
      if (c->first.getNullCst()>0.000001) oss<<"-NULL"<<c->first.getNullCst();
    }
    oss<<"]  ";
    oss<<"{"; for (unsigned int i=0; i<w->getFertSize();++i){oss<<w->getFertCst(i);if (i<w->getFertSize()-1) oss<<" ";}; oss<<"}";
  }
  oss<<endl<<endl;
  count=0;
  for (confusionNet<iword>::const_iterator w=(this->t).begin();w!=(this->t).end();++w){
    oss<<count<<" [";
    ++count;
    for (iwordCnColumn::const_iterator c=w->begin();c!=w->end();++c){
      if (c!=w->begin()){oss<<" ||| ";}
      oss<<c->first.getPos()<<":"<<c->first.getForm();
      oss<<"("<<c->first.getStform()<<")";
      if (c->first.getNullCst()>0.000001) oss<<"-NULL"<<c->first.getNullCst();
      oss<<"{"; for (unsigned int i=0; i<c->first.getFertSize();++i){oss<<c->first.getFertCst(i);if (i<c->first.getFertSize()-1) oss<<" ";}; oss<<"}";
    }
    oss<<"]  ";
  }
  oss<<endl;
  return oss.str();
}


/* bilCorpus class
********************/

bilCorpus::bilCorpus (const string & fs, const string & ft, string range, int sInputType,int tInputType) {
  // open input source and target (word forms) files:
  fsFile=new ifstream(fs.c_str());
  if (! fsFile->good()){
    cerr << "ERROR while opening file:" << fs << endl;
    exit(EXIT_FAILURE);
  }
  ftFile=new ifstream(ft.c_str());
  if (! ftFile->good()){
    cerr << "ERROR while opening file:" << ft << endl;
    exit(EXIT_FAILURE);
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
    if (! fsFile->good()){
      cerr << "ERROR: file" << fs << " not good " << endl; exit(EXIT_FAILURE);
    }else{
      if (sInputType==0){
	if (!getline(*fsFile,line)){
	  cerr << "ERROR while going to first line of range in file:" << fs << endl;
	  exit(EXIT_FAILURE);
	}
      }else{
	string buf;
	int stop=-1;
	int filepos=ftFile->tellg();
	while (getline(*fsFile,line) && stop <=0){
	  istringstream iss(line);
	  iss >> buf;
	  if (buf == "name"){++stop;}
	  else {filepos=fsFile->tellg();}
	}
	fsFile->seekg(filepos);
      }
    }

    if (! ftFile->good()){
      cerr << "ERROR: file" << ft << " not good " << endl; exit(EXIT_FAILURE);
    }else{
      if (tInputType==0){
	if (!getline(*ftFile,line)){
	  cerr << "ERROR while going to first line of range in file:" << ft << endl;
	  exit(EXIT_FAILURE);
	}
      }else{
	string buf;
	int stop=-1;
	int filepos=ftFile->tellg();
	while (getline(*ftFile,line) && stop <=0){
	  istringstream iss(line);
	  iss >> buf;
	  if (buf == "name"){++stop;}
	  else {filepos=ftFile->tellg();}
	}
	ftFile->seekg(filepos);
      }
    }
  }
}

bool bilCorpus::getNextSentPair (sentPair & sp, vocTable & svoc, vocTable & tvoc, int sInputType,int tInputType){
  sp.clear();
  ++sentPairNum;
  if (!untilEnd && sentPairNum>lastSentPairNum){
    return false;
  }
  string line;
  if (! fsFile->good()){
    return false;
  }else{
    // read source sentence and split it
    if (sInputType==0){
      if (getline(*fsFile,line)) sp.loadSrcWords(line,svoc);
      else  return false;
    }else if (sInputType==1){
      vector<string> lines;
      string buf;
      int stop=-1;
      int filepos=ftFile->tellg();
      while (getline(*fsFile,line) && stop <=0){
	istringstream iss(line);
	iss >> buf;
	if (buf == "align"){lines.push_back(line);}
	if (buf == "name"){++stop;}
	else {filepos=fsFile->tellg();}
      }
      fsFile->seekg(filepos);
      sp.loadSrcWords(lines,svoc);
    }
  }

  if (! ftFile->good()){
    return false;
  }else{
    // read target sentence and split it
    if (tInputType==0){
      if (getline(*ftFile,line)) sp.loadTrgWords(line,tvoc);
      else  return false;
    }else if (tInputType==1){
      vector<string> lines;
      string buf;
      int stop=-1;
      int filepos=ftFile->tellg();
      while (getline(*ftFile,line) && stop <=0){
	istringstream iss(line);
	iss >> buf;
	if (buf == "align"){lines.push_back(line);}
	if (buf == "name"){++stop;}
	else {filepos=ftFile->tellg();}
      }
      ftFile->seekg(filepos);
      sp.loadTrgWords(lines,tvoc);
    }
  }
  return true;
}

