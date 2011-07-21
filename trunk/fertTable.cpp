#include "fertTable.h"

/* unlinkedModelTable class
***************************/
void unlinkedModelTable::load (string fname, vocTable & voc){
  ifstream ffile(fname.c_str());
  // open input file
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
    //line format: wordIndex cst 
    //cout << line << endl;
    istringstream iss(line);
    string wd;
    cost cst;    
    iss >> wd >> cst;
    wordIndex ind=voc.insert(wd);
    jash_um[ind]= cst;
  }
}
bool unlinkedModelTable::cst(wordIndex src, cost & _cst) const {
  hash_map<wordIndex,cost>::const_iterator cit=jash_um.find(src);
  if ( cit != jash_um.end() ) {
    _cst=cit->second;
    return true;
  }else{
    return false;
  }
}

/* fertTable class
********************/
void fertTable::load (string fname, vocTable & voc){
  ifstream ffile(fname.c_str());
  // open input file
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
    //line format: wordIndex cst1 cst2 ... cstN
    //cout << line << endl;
    istringstream iss(line);
    string wd;
    iss >> wd ;
    wordIndex ind=voc.insert(wd);
    vector<cost> costs;    
    float buf;
    while (iss >> buf){
      costs.push_back(buf);
    }
    jash_fert[ind]= costs;
  }
}

bool fertTable::cstVec(wordIndex src, vector<cost> & vec) const {
  hash_map<wordIndex,vector<cost> >::const_iterator cit=jash_fert.find(src);
  if ( cit != jash_fert.end() ) {
    for (vector<cost>::const_iterator qv=(cit->second).begin();qv!=(cit->second).end();++qv){
      vec.push_back(*qv);
    }
    return true;
  }else{
    return false;
  }
}
/* classFertTable class
********************/
void classFertTable::load (string fname, vocTable & voc, vocTable & clVoc){
  ifstream ffile(fname.c_str());
  // open input file
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
    //line format: int wordIndex cst1 cst2 ... cstN
    //cout << line << endl;
    istringstream iss(line);
    string wd,cl;
    iss >> cl >> wd ;
    wordIndex wdInd=voc.insert(wd);
    wordIndex clInd=clVoc.insert(cl);
    vector<cost> costs;    
    float buf;
    while (iss >> buf){
      costs.push_back(buf);
    }
    jash_fert[make_pair(clInd,wdInd)]= costs;
  }
}

bool classFertTable::cstVec(int cl, wordIndex src, vector<cost> & vec) const {
  hash_map<pair<int,wordIndex>,vector<cost>, hashfintwdind, hasheqintwdind>::const_iterator cit=jash_fert.find(make_pair(cl,src));
  if ( cit != jash_fert.end() ) {
    for (vector<cost>::const_iterator qv=(cit->second).begin();qv!=(cit->second).end();++qv){
      vec.push_back(*qv);
    }
    return true;
  }else{
    return false;
  }
}
