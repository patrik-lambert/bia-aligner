#include<vector>
#include<sstream>
#include<iostream>
#include "math.h"
#include "defs.h"
#include "iword.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;

typedef wordIndex formType;

#ifndef ALUNIT
#define ALUNIT
namespace alUnitConst {
  const string unitDelim="|||";
};

class alUnit {
  vector<formType> unit;
 public:
  alUnit(vector<formType> & v);
  alUnit(formType f);
  alUnit(){};
  typedef vector<formType>::const_iterator const_iterator;
  typedef vector<formType>::size_type size_type;
  const_iterator begin() const { return unit.begin(); }
  const_iterator end() const { return unit.end(); }
  bool operator== (const alUnit & y) const; 
  void push_back (formType f);
  size_type size() const {return unit.size();}
  formType at(size_type n) const {return unit.at(n);}
  string print (const vocTable & voc) const;
  string printDelim () const;
  void reduce (alUnit & reducedUnit, const vocCorrespondence & corresp) const;
};
#endif


#define MODUL 100000000  

#ifndef HASHALUNIT
#define HASHALUNIT
class hashfalunit : public unary_function<alUnit, size_t>{
public:
  size_t operator()(alUnit const &v) const{
    unsigned long long int res=0;
    int i=0;
    for (alUnit::const_iterator it=v.begin();it!=v.end();++it){
      if (*it>=0){
	if (i%5==0) res += (size_t) *it*1277;
	else if (i%5==1) res += (size_t) *it*12781;
	else if (i%5==2) res += (size_t) *it*127031;
	else if (i%5==3) res += (size_t) *it*1270319;
	else res += (size_t) *it*127;
      }
      ++i;
    }
    return res%MODUL;
  }
};
class hasheqalunit{
public:
    bool operator()(alUnit const &v1, alUnit const &v2) const{
	return v1==v2;
    }
};
#endif

#ifndef HASHPAIRALUNIT
#define HASHPAIRALUNIT
class hashfpairalunit : public unary_function<pair<alUnit,alUnit>, size_t>{
public:
  size_t operator()(pair<alUnit,alUnit> const &p) const{
    unsigned long long int res=0;
    int i=0;
    for (alUnit::const_iterator it=p.first.begin();it!=p.first.end();++it){
      if (*it>=0){
	if (i%5==0) res += (size_t) *it*1277;
	else if (i%5==1) res += (size_t) *it*12781;
	else if (i%5==2) res += (size_t) *it*127031;
	else if (i%5==3) res += (size_t) *it*1270319;
	else res += (size_t) *it*127;
      }
      ++i;
    }
    for (alUnit::const_iterator it=p.second.begin();it!=p.second.end();++it){
      if (*it>=0){
	if (i%5==0) res += (size_t) *it*1277;
	else if (i%5==1) res += (size_t) *it*12781;
	else if (i%5==2) res += (size_t) *it*127031;
	else if (i%5==3) res += (size_t) *it*1270319;
	else res += (size_t) *it*127;
      }
      ++i;
    }
    return res%MODUL;
  }
};

class hasheqpairalunit{
 public:
  bool operator()(pair<alUnit,alUnit> const &p1, pair<alUnit,alUnit> const &p2) const{
    return p1==p2;
  }
};
#endif
