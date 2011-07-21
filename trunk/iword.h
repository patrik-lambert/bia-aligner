#include<string>
#include<vector>
#include<sstream>
//#include<set>
#include "defs.h"
#include "confusionNet.h"

using namespace std;

#ifndef HASHSTRING
#define HASHSTRING

class hashfstring : public unary_function<string, size_t>{
 public:
  size_t operator()(string const &s) const{
    unsigned int res=0;
    for (unsigned int i=0;i<s.size();i++){
      res += (size_t) (unsigned int) pow((double)s.at(i), double((i%6)+1));
    }
    return res%WMODUL;
  }
};
class hasheqstring{
 public:
  bool operator()(string const &s1, string const &s2) const{
    return s1.compare(s2)==0;
  }
};
#endif

#ifndef HASHVECWDIND
#define HASHVECWDIND

class hashfvecwdind : public unary_function<vector<wordIndex>, size_t>{
 public:
  size_t operator()(vector<wordIndex> const &p) const{
    unsigned long long int res=0;
    int i=0;
    for (vector<wordIndex>::const_iterator it=p.begin();it!=p.end();++it){
      if (*it>=0){
	if (i%5==0) res += (size_t) *it*1277;
	else if (i%5==1) res += (size_t) *it*12781;
	else if (i%5==2) res += (size_t) *it*127031;
	else if (i%5==3) res += (size_t) *it*1270319;
	else res += (size_t) *it*127;
      }
      ++i;
    }
    return res%PHMODUL;
  }
};
                                                                                    
class hasheqvecwdind{
 public:
  bool operator()(vector<wordIndex> const &p1, vector<wordIndex> const &p2) const{
    return p1==p2;
  }
};

#endif

#ifndef _IWORD_H
#define _IWORD_H


class iword
{
  vector<int> pos;
  vector<wordIndex> form;
  vector<string> stform;
  int pofs;
  cost nullCst;
  vector<cost> vfert;
  //	stem;
  //	chunk;
 public:
  //iword(int pos=0, wordIndex s = 0,int pofs=-1);
  iword(){nullCst=10.0;};
  void setPos(int pos,int level);
  void addPos(int pos);
  int getPos(int level=0) const;
  void setForm(wordIndex s, int level);
  void addForm(wordIndex s);
  wordIndex getForm(int level=0) const;
  void setStform(string stform, int level);
  void addStform(string stform);
  string getStform(int level=0) const;
  void setPofs(int pofs);
  int getPofs() const;
  void setNullCst(cost _cst);
  cost getNullCst() const;
  void setFertCostsVec(const vector<cost> & vec);
  cost getFertCst(int fert) const;
  int getFertSize() const;
  string print() const;
};

#endif

#ifndef VOCTABLE
#define VOCTABLE

#include <iostream>
#include <fstream>

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif
// TABLE OF CORRESPONDENCE INDEX->WORD
//***************************************************************
class vocTable {
  vector<string> voc;
  hash_map<string,wordIndex,hashfstring,hasheqstring> jash_forms;
 public:
  //  vocTable (string fname){load(fname);};
  //void load (string fname);
  vocTable (){};
  bool empty () const {
    return voc.empty();
  }
  wordIndex insert ( string form ) {
    hash_map<string,wordIndex,hashfstring,hasheqstring>::const_iterator vit=jash_forms.find(form);
    if (vit != jash_forms.end()) return vit->second;
    else {
      wordIndex ind(voc.size());
      voc.push_back(form);
      jash_forms.insert(make_pair(form,ind));
      return ind;
    }
  }
  wordIndex index ( string form ) const {
    hash_map<string,wordIndex,hashfstring,hasheqstring>::const_iterator vit=jash_forms.find(form);
    if (vit != jash_forms.end()) return vit->second;
    else return -1;
  }
  string form ( wordIndex ind) const {
    if (ind < voc.size()) return voc.at(ind);
    else return "ERROR: word index outside range";
  }
/* class vocTable { */
/*   hash_map<wordIndex,string> jash_voc; */
/*  public: */
/*   vocTable (string fname){load(fname);}; */
/*   vocTable (){}; */
/*   void load (string fname); */
/*   bool empty () const { */
/*     return jash_voc.empty(); */
/*   } */
/*   string form ( wordIndex ind) const { */
/*     hash_map<wordIndex,string>::const_iterator vit=jash_voc.find(ind); */
/*     if (vit != jash_voc.end()) return vit->second; */
/*     else return ""; */

/*   } */
};
#endif

#ifndef _IWORDCOLUMN_H
#define _IWORDCOLUMN_H
class iwordCnColumn: public cnColumn<iword>
{
  cost nullCst;
  vector<cost> vfert;
 public:
  void  merge (string mode);
  cost getNullCst() const;
  cost getFertCst(int fert) const;
  int getFertSize() const;
};
#endif

#ifndef _CONFUSIONNETIWORD_H
#define _CONFUSIONNETIWORD_H

template <> class confusionNet<iword> {
  vector<iwordCnColumn> cn;
 public:
  confusionNet(){};
  typedef vector<iwordCnColumn>::const_iterator const_iterator;
  typedef vector<iwordCnColumn>::iterator iterator;
  typedef vector<iwordCnColumn>::size_type size_type;
  const_iterator begin() const { return cn.begin(); }
  const_iterator end() const { return cn.end(); }
  iterator begin() { return cn.begin(); }
  iterator end() { return cn.end(); }
  void push_back (const iwordCnColumn & c){cn.push_back(c);}
  void push_back_at_column (const iword & c, float fl, size_type n){cn.at(n).push_back(c,fl);}
  size_type size() const {return cn.size();}
  iwordCnColumn at (size_type n) const {return *(cn.begin()+n);}
  iwordCnColumn & at (size_type n) {return *(cn.begin()+n);}
  iwordCnColumn operator[] (size_type n) {return cn[n];}
  void clear() {
    cn.clear();
  }
  string print () const {
    ostringstream oss("");
    int count=0;
    for (vector<iwordCnColumn>::const_iterator it=cn.begin(); it!=cn.end(); ++it){
	oss<<count<<" ["<<it->print()<<"] ";
	++count;
    }
    return oss.str();
  }
};

#endif
