#include <vector>
#include <iostream>
#include <sstream>
#include "math.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;

#ifndef _LINKCLUSTER_H
#define _LINKCLUSTER_H

class linkCluster
{
  vector<int> sPosis,tPosis;
  bool isPhrase(const vector<int> & posis) const;
 public:
  linkCluster(int _src, int _trg);
  linkCluster(vector<int> sVec, vector<int> tVec);
  void addSposi(int posi);
  void addTposi(int posi);
  void insertCluster(const linkCluster & lc);
  string print() const;
  bool srcIsPhrase() const {
    return isPhrase(sPosis);
  }
  bool trgIsPhrase() const {
    return isPhrase(tPosis);
  }
  void sPhrase(vector<int> & _seq){
    _seq=sPosis;
  }
  void tPhrase(vector<int> & _seq){
    _seq=tPosis;
  }
  void doSort();
  void sortAndSplitIntoContiguous(vector<linkCluster> & splitvec);
};

#endif

#ifndef _LINKCLUSTERDIV_H
#define _LINKCLUSTERDIV_H
class linkClusterDiv: public vector<linkCluster>
{
  hash_map<int,int> scomp,tcomp;
  int nClust;
 public:
  linkClusterDiv(){nClust=0;};
  void addLink(int sposi, int tposi);
  void sortAndSplitIntoContiguous();
  string print() const;
  //  void storePhrases(phraseDetection & sphDetect, phraseDetection & tphDetect, vector<jword> & sent1, vector<jword> & sent2);
  int size() const {return nClust;}
};

#endif

