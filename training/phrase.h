#include<vector>
#include<sstream>
#include<iostream>
#include "math.h"
#include "alUnit.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;

typedef wordIndex clType;

#ifndef JWORD
#define JWORD
class jword {
  formType _form;
  clType _cl;
 public:
  unsigned int nocc;
  unsigned int fert;
  formType form() const {return _form;}
  clType cl() const {return _cl;}
  void setCl(clType cls) {_cl=cls;}
 jword(formType _frm, unsigned int _nocc, unsigned int _fert=0): _form(_frm), nocc(_nocc), fert(_fert) {};
};
#endif


#ifndef _PHRASEDETECTION_H
#define _PHRASEDETECTION_H

class phraseDetection
{
  typedef hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit> alUnitVocMap;
  alUnitVocMap phVoc;
  alUnitVocMap subPhVoc;
 public:
  phraseDetection(){};
  typedef hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> mapType;
  void searchSentence(const vector<jword> & sent, mapType & foundPhrases);
  void expand(formType word, int nWordDone, const vector<jword> & sent, mapType & foundPhrases);
  void loadPhrase(alUnit & _alUnit);
  void pruneVoc(unsigned int minOccur);
  void pruneVocAsReduced(const phraseDetection & redPhSearch, const vocCorrespondence & vocToRedVoc);
  bool isInVoc (const alUnit & _alUnit) const;
  bool isInSubVoc (const alUnit & _alUnit) const;
  string printVoc(const vocTable & voc) const; 
  string printSubVoc(const vocTable & voc) const; 
};

#endif
