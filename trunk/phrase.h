#include<vector>
#include<sstream>
#include<iostream>
#include "math.h"
#include "iword.h"
#include "alUnit.h"
#include "sentPair.h"

#if __GNUC__>2
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#else
#include <hash_map>
#endif

using namespace std;

#ifndef _PHRASEDETECTION_H
#define _PHRASEDETECTION_H

class phraseDetection
{
  typedef hash_map<alUnit,unsigned long int,hashfalunit,hasheqalunit> alUnitVocMap;
  alUnitVocMap phVoc;
  alUnitVocMap subPhVoc;
  void expand(formType word, int nWordDone, const sentence & sent, uniqAlUnitCollection & foundPhrases);
  void expand(uniqAlUnitColumn & curPhCol, int nWordDone, const confusionNet<iword> & cn, uniqAlUnitColumnCollection & foundPhrases);
 public:
  phraseDetection(){};
  //  typedef hash_map<alUnit,unsigned int,hashfalunit,hasheqalunit> mapType;
  void searchSentence(const sentence & sent, uniqAlUnitCollection & foundPhrases);
  void searchSentence(const confusionNet<iword> & cn, uniqAlUnitColumnCollection & foundPhrases);
  void loadPhrase(alUnit & _alUnit);
  string printVoc(const vocTable & voc) const;
  string printSubVoc(const vocTable & voc) const;
  void pruneVoc(int minOccur);
};

#endif
