#include<vector>
#include<sstream>
#include<iostream>
#include<utility>
#include "math.h"
#include "defs.h"

using namespace std;

#ifndef CNCOLUMN
#define CNCOLUMN

template <class containedType> class cnColumn{
  vector<pair<containedType,float> > column;
 public:
  cnColumn(){};
  typedef typename vector<pair<containedType,float> >::const_iterator const_iterator;
  typedef typename vector<pair<containedType,float> >::iterator iterator;
  typedef typename vector<pair<containedType,float> >::size_type size_type;
  const_iterator begin() const { return column.begin(); }
  const_iterator end() const { return column.end(); }
  iterator begin() { return column.begin(); }
  iterator end() { return column.end(); }
  
  void push_back (const containedType & u, float fl){
    pair<containedType,float> p=make_pair(u,fl);
    column.push_back(p);
  }
  size_type size() const {return column.size();}
  pair<containedType,float> at(size_type n) const {return *(column.begin()+n);}
  pair<containedType,float> & at(size_type n) {return *(column.begin()+n);}
  string print () const {
    ostringstream oss("");
    for (typename vector<pair<containedType,float> >::const_iterator it=column.begin(); it!=column.end(); ++it){
      if (it != column.begin()){oss<<" ||| ";}
      oss<<it->first.print()<<"_"<<it->second;
    }
    return oss.str();
  }
  float probSum () const {
    float sum=0;
    for (typename vector<pair<containedType,float> >::const_iterator it=column.begin(); it!=column.end(); ++it){
      sum+=it->second;
    }
    return sum;
  }
  void merge (containedType & outContainedType, string mode="") const;
};

#endif

#ifndef CONFUSIONNET
#define CONFUSIONNET

template <class containedType> class confusionNet {
  vector<cnColumn<containedType> > cn;
 public:
  confusionNet(){};
  typedef typename vector<cnColumn<containedType> >::const_iterator const_iterator;
  typedef typename vector<cnColumn<containedType> >::iterator iterator;
  typedef typename vector<cnColumn<containedType> >::size_type size_type;
  const_iterator begin() const { return cn.begin(); }
  const_iterator end() const { return cn.end(); }
  iterator begin() { return cn.begin(); }
  iterator end() { return cn.end(); }
  void push_back (const cnColumn<containedType> & c){cn.push_back(c);}
  void push_back_at_column (const containedType & c, float fl, size_type n){cn.at(n).push_back(c,fl);}
  size_type size() const {return cn.size();}
  cnColumn<containedType> & at (size_type n) const {return *(cn.begin()+n);}
  cnColumn<containedType> & at (size_type n) {return *(cn.begin()+n);}
  cnColumn<containedType> operator[] (size_type n) {return cn[n];}
  void clear() {
    cn.clear();
  }
  string print () const {
    ostringstream oss("");
    int count=0;
    for (typename vector<cnColumn<containedType> >::const_iterator it=cn.begin(); it!=cn.end(); ++it){
	oss<<count<<" ["<<it->print()<<"] ";
	++count;
    }
    return oss.str();
  }
};

#endif


