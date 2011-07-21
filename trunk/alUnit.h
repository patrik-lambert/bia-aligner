#include<vector>
#include<sstream>
#include<iostream>
#include "math.h"
#include "defs.h"
#include "confusionNet.h"
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
  void getUnit (alUnit & u) const {
    for (const_iterator it=unit.begin();it!=unit.end();++it){u.push_back(*it);}
  }
  string print (const vocTable & voc) const;
  string printDelim () const;
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
  bool operator()(pair<alUnit,alUnit>const &p1, pair<alUnit,alUnit> const &p2) const{
    return p1==p2;
  }
};
#endif

#ifndef UNIQALUNITCOLUMN
#define UNIQALUNITCOLUMN
/* uniqAlUnitColumn of size 1: 
formType1 p1
formType2 p2
...
of size 2 (al units have size 2):
formType11 formType12 p1 (p1 is a mixture between p11 and p12)
formType21 formType22 p2
...
*/
class uniqAlUnitColumn: public cnColumn<alUnit> {
  int _beg; //beginnng position
 public:
  uniqAlUnitColumn(int posi) : _beg(posi) {};
  uniqAlUnitColumn(const cnColumn<alUnit> & col, int posi): _beg(posi) { 
    for (cnColumn<alUnit>::const_iterator it=col.begin();it!=col.end();++it){
      this->push_back(it->first,it->second); 
    }
  }
  int firstPosi() const {return _beg;}
  int lastPosi() const {return (_beg+this->begin()->first.size()-1);}
  void setFirstPosi(int posi) {_beg=posi;}
  string print(const vocTable & voc) const;
};
#endif

#ifndef UNIQALUNITCOLUMNCOLLECTION
#define UNIQALUNITCOLUMNCOLLECTION
class uniqAlUnitColumnCollection {
  vector<uniqAlUnitColumn> collection;
 public:
  uniqAlUnitColumnCollection(){};
  typedef vector<uniqAlUnitColumn>::const_iterator const_iterator;
  typedef vector<uniqAlUnitColumn>::iterator iterator;
  typedef vector<uniqAlUnitColumn>::size_type size_type;
  const_iterator begin() const { return collection.begin(); }
  const_iterator end() const { return collection.end(); }
  iterator begin() { return collection.begin(); }
  iterator end() { return collection.end(); }
  void push_back(const uniqAlUnitColumn & c){collection.push_back(c);}
  void push_back_at_column(const alUnit & c, float prob, size_type n){collection.at(n).push_back(c, prob);}
  uniqAlUnitColumn at (size_type n) const {return collection.at(n);}
  uniqAlUnitColumn operator[] (size_type n) {return collection[n];}
  string print(const vocTable & voc) const;
  void clear(){collection.clear();}
};
#endif

#ifndef UNIQALUNIT
#define UNIQALUNIT

class uniqAlUnit: public alUnit {
  int _beg; //beginnng position
 public:
  uniqAlUnit(formType f, int posi);
  int firstPosi() const {return _beg;}
  int lastPosi() const {return (_beg+this->size()-1);}
  void setFirstPosi(int posi) {_beg=posi;}
  string print(const vocTable & voc) const;
};
#endif

#ifndef UNIQALUNITCOLLECTION
#define UNIQALUNITCOLLECTION

class uniqAlUnitCollection {
  vector<uniqAlUnit> unitColl;
 public:
  uniqAlUnitCollection(){};
  typedef vector<uniqAlUnit>::const_iterator const_iterator;
  typedef vector<uniqAlUnit>::size_type size_type;
  const_iterator begin() const { return unitColl.begin(); }
  const_iterator end() const { return unitColl.end(); }
  void push_back (const uniqAlUnit & u);
  //size_type size() const {return unitColl.size();}
  //formType at(size_type n) const {return unit.at(n);}
  string print (const vocTable & voc) const;
};
#endif

