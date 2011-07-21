#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "defs.h"
#include "params.h"
#include "math.h"
#include "iword.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

#define FERTMODUL 500000

#ifndef HASHINTWDIND
#define HASHINTWDIND

class hashfintwdind : public unary_function<pair<int,wordIndex>, size_t>{
 public:
  size_t operator()(pair<int,wordIndex> const &p) const{
    wordIndex res=0;
    res += (size_t) p.first;
    res += (size_t) pow((double)p.second,double(2));
    return res%FERTMODUL;
  }
};
                                                                                    
class hasheqintwdind{
 public:
  bool operator()(pair<int,wordIndex> const &p1, pair<int,wordIndex> const &p2) const{
    return p1==p2;
  }
};

#endif

#ifndef UNLINKEDMODELTABLE
#define UNLINKEDMODELTABLE
class unlinkedModelTable {
  hash_map<wordIndex,cost> jash_um;
 public:
  unlinkedModelTable (string fname, vocTable & voc) {load(fname,voc);};
  unlinkedModelTable (){};
  void load (string fname, vocTable & voc);
  bool cst(wordIndex src, cost & _cst) const;
  bool empty(){return jash_um.empty();}
};
#endif

#ifndef FERTTABLE
#define FERTTABLE

class fertTable {
  hash_map<wordIndex,vector<cost> > jash_fert;
 public:
  fertTable (string fname, vocTable & voc){load(fname,voc);};
  fertTable (){};
  void load (string fname, vocTable & voc);
  bool cstVec(wordIndex src, vector<cost> & vec) const;
  bool empty(){return jash_fert.empty();}
};

class classFertTable {
  hash_map<pair<int,wordIndex>, vector<cost>, hashfintwdind, hasheqintwdind> jash_fert;
 public:
  classFertTable(string fname, vocTable & voc, vocTable & clVoc){load(fname,voc, clVoc);};
  classFertTable(){};
  void load (string fname, vocTable & voc, vocTable & clVoc);
  bool cstVec(int cl, wordIndex src, vector<cost> & vec) const;
  bool empty(){return jash_fert.empty();}
};

#endif
